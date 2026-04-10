#include "sim/modbus_client.h"
#include "sim/registry.h"

#include <cstdint>
#include <cstring>
#include <thread>
#include <chrono>
#include <vector>
#include <cstdio>
#include <atomic>

// ── Platform socket includes ──────────────────────────────────────────────────
#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <winsock2.h>
#  include <ws2tcpip.h>
#  ifdef _MSC_VER
#    pragma comment(lib, "ws2_32.lib")
#  endif
   using sock_t = SOCKET;
#  define SOCK_INVALID INVALID_SOCKET
#  define sock_close   closesocket
#else
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <unistd.h>
   using sock_t = int;
#  define SOCK_INVALID (-1)
#  define sock_close   close
#endif

namespace myplc::sim {

// ── Helpers ───────────────────────────────────────────────────────────────────
static bool recv_all(sock_t s, uint8_t* buf, int n) {
    int got = 0;
    while (got < n) {
#ifdef _WIN32
        int r = recv(s, reinterpret_cast<char*>(buf + got), n - got, 0);
#else
        int r = static_cast<int>(recv(s, buf + got, static_cast<size_t>(n - got), 0));
#endif
        if (r <= 0) return false;
        got += r;
    }
    return true;
}

static bool send_all(sock_t s, const uint8_t* buf, int n) {
#ifdef _WIN32
    return send(s, reinterpret_cast<const char*>(buf), n, 0) == n;
#else
    return static_cast<int>(send(s, buf, static_cast<size_t>(n), 0)) == n;
#endif
}

// ── Low-level Modbus requests ─────────────────────────────────────────────────
static uint16_t s_tid = 0;  // transaction ID counter

// FC03: read `count` holding registers starting at `start_addr`
// Returns true on success, fills `out` with `count` values.
static bool mb_read_regs(sock_t s, uint16_t start_addr, uint16_t count, uint16_t* out) {
    uint8_t req[12];
    ++s_tid;
    req[0] = static_cast<uint8_t>(s_tid >> 8);
    req[1] = static_cast<uint8_t>(s_tid & 0xFF);
    req[2] = 0; req[3] = 0;      // protocol ID
    req[4] = 0; req[5] = 6;      // remaining length
    req[6] = 1;                   // unit ID
    req[7] = 0x03;                // FC03
    req[8] = static_cast<uint8_t>(start_addr >> 8);
    req[9] = static_cast<uint8_t>(start_addr & 0xFF);
    req[10]= static_cast<uint8_t>(count >> 8);
    req[11]= static_cast<uint8_t>(count & 0xFF);

    if (!send_all(s, req, 12)) return false;

    uint8_t resp_hdr[9];
    if (!recv_all(s, resp_hdr, 9)) return false;
    uint8_t byte_count = resp_hdr[8];
    if (byte_count != count * 2) return false;

    std::vector<uint8_t> data(byte_count);
    if (!recv_all(s, data.data(), byte_count)) return false;

    for (uint16_t i = 0; i < count; ++i) {
        out[i] = static_cast<uint16_t>((data[i*2] << 8) | data[i*2+1]);
    }
    return true;
}

// FC16: write `count` registers starting at `start_addr`
static bool mb_write_regs(sock_t s, uint16_t start_addr, uint8_t count, const uint16_t* data) {
    uint16_t pdu_len = static_cast<uint16_t>(7 + count * 2);
    std::vector<uint8_t> req(6 + pdu_len);
    ++s_tid;
    req[0] = static_cast<uint8_t>(s_tid >> 8);
    req[1] = static_cast<uint8_t>(s_tid & 0xFF);
    req[2] = 0; req[3] = 0;
    req[4] = static_cast<uint8_t>(pdu_len >> 8);
    req[5] = static_cast<uint8_t>(pdu_len & 0xFF);
    req[6] = 1;      // unit ID
    req[7] = 0x10;   // FC16
    req[8] = static_cast<uint8_t>(start_addr >> 8);
    req[9] = static_cast<uint8_t>(start_addr & 0xFF);
    req[10]= static_cast<uint8_t>(0);
    req[11]= count;
    req[12]= static_cast<uint8_t>(count * 2);
    for (uint8_t i = 0; i < count; ++i) {
        req[13 + i*2] = static_cast<uint8_t>(data[i] >> 8);
        req[14 + i*2] = static_cast<uint8_t>(data[i] & 0xFF);
    }

    if (!send_all(s, req.data(), static_cast<int>(req.size()))) return false;

    // Read echo response (12 bytes)
    uint8_t resp[12];
    return recv_all(s, resp, 12);
}

// ── Client thread ─────────────────────────────────────────────────────────────
static void client_thread(std::string host, uint16_t port, uint32_t poll_ms) {
#ifdef _WIN32
    WSADATA wsa{};
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

    std::printf("[modbus_client] Connecting to %s:%d ...\n", host.c_str(), port);

    while (true) {
        // ── Connect ───────────────────────────────────────────────────────────
        sock_t s = socket(AF_INET, SOCK_STREAM, 0);
        if (s == SOCK_INVALID) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            continue;
        }

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port   = htons(port);
#ifdef _WIN32
        InetPtonA(AF_INET, host.c_str(), &addr.sin_addr);
#else
        inet_pton(AF_INET, host.c_str(), &addr.sin_addr);
#endif

        if (connect(s, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
            sock_close(s);
            std::this_thread::sleep_for(std::chrono::seconds(2));
            continue;
        }
        std::printf("[modbus_client] Connected to %s:%d\n", host.c_str(), port);

        // ── Poll loop ─────────────────────────────────────────────────────────
        bool ok = true;
        while (ok) {
            uint16_t total = Registry::get().total_regs();
            if (total > 0) {
                // 1. Push dirty variables (web dashboard → ESP32)
                Registry::get().drain_dirty([&](uint16_t mb_addr, uint8_t mb_count, const uint16_t* regs) {
                    if (!mb_write_regs(s, mb_addr, mb_count, regs)) ok = false;
                });

                // 2. Poll all registers (ESP32 → Registry → dashboard)
                if (ok) {
                    std::vector<uint16_t> regs(total, 0);
                    if (mb_read_regs(s, 0, total, regs.data())) {
                        Registry::get().apply_write_regs(0, total, regs.data());
                    } else {
                        ok = false;
                    }
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(poll_ms));
        }

        std::printf("[modbus_client] Connection lost. Reconnecting...\n");
        sock_close(s);
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

void start_modbus_client(const std::string& host, uint16_t port, uint32_t poll_ms) {
    std::thread(client_thread, host, port, poll_ms).detach();
}

}  // namespace myplc::sim
