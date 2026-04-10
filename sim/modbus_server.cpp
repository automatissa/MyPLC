#include "sim/modbus_server.h"
#include "sim/registry.h"

#include <cstdint>
#include <cstring>
#include <thread>
#include <vector>
#include <cstdio>

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
#  define sock_error   WSAGetLastError()
#else
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <unistd.h>
#  include <errno.h>
   using sock_t = int;
#  define SOCK_INVALID (-1)
#  define sock_close   close
#  define sock_error   errno
#endif

namespace myplc::sim {

// ── Modbus ADU/PDU helpers ────────────────────────────────────────────────────
// Modbus TCP frame: [MBAP header 6 bytes] [Unit ID 1 byte] [PDU ...]
// MBAP: Transaction ID (2) | Protocol ID (2) | Length (2)
//       Length = count of remaining bytes (Unit ID + PDU)

static bool recv_all(sock_t s, uint8_t* buf, int n) {
    int received = 0;
    while (received < n) {
#ifdef _WIN32
        int r = recv(s, reinterpret_cast<char*>(buf + received), n - received, 0);
#else
        int r = static_cast<int>(recv(s, buf + received, static_cast<size_t>(n - received), 0));
#endif
        if (r <= 0) return false;
        received += r;
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

// Build Modbus exception response
static void send_exception(sock_t s, uint8_t* mbap, uint8_t fc, uint8_t code) {
    uint8_t resp[9];
    // MBAP: copy transaction/protocol IDs
    resp[0] = mbap[0]; resp[1] = mbap[1];  // transaction ID
    resp[2] = 0;       resp[3] = 0;          // protocol ID
    resp[4] = 0;       resp[5] = 3;          // length = 3 (unit + fc_error + code)
    resp[6] = mbap[6];                        // unit ID
    resp[7] = static_cast<uint8_t>(fc | 0x80u);
    resp[8] = code;
    send_all(s, resp, 9);
}

// Handle one complete Modbus TCP request from client socket
static void handle_request(sock_t client) {
    uint8_t header[7];  // 6-byte MBAP + 1-byte unit ID

    while (recv_all(client, header, 7)) {
        uint16_t pdu_len = static_cast<uint16_t>((header[4] << 8) | header[5]);
        if (pdu_len < 2 || pdu_len > 256) break;  // sanity check

        std::vector<uint8_t> pdu(pdu_len - 1);  // exclude unit ID already read
        if (!recv_all(client, pdu.data(), static_cast<int>(pdu.size()))) break;

        uint8_t fc    = pdu[0];
        uint16_t addr = static_cast<uint16_t>((pdu[1] << 8) | pdu[2]);
        uint16_t qty  = static_cast<uint16_t>((pdu[3] << 8) | pdu[4]);

        if (fc == 0x03) {
            // FC03 — Read Holding Registers
            if (qty < 1 || qty > 125) {
                send_exception(client, header, fc, 0x03);  // Illegal Data Value
                continue;
            }
            std::vector<uint16_t> regs(qty, 0);
            Registry::get().fill_response_regs(addr, qty, regs.data());

            // Build response
            uint8_t byte_count = static_cast<uint8_t>(qty * 2);
            std::vector<uint8_t> resp(9 + qty * 2);
            resp[0] = header[0]; resp[1] = header[1];
            resp[2] = 0;         resp[3] = 0;
            uint16_t resp_len = static_cast<uint16_t>(3 + qty * 2);
            resp[4] = static_cast<uint8_t>(resp_len >> 8);
            resp[5] = static_cast<uint8_t>(resp_len & 0xFF);
            resp[6] = header[6];
            resp[7] = fc;
            resp[8] = byte_count;
            for (uint16_t i = 0; i < qty; ++i) {
                resp[9  + i*2] = static_cast<uint8_t>(regs[i] >> 8);
                resp[10 + i*2] = static_cast<uint8_t>(regs[i] & 0xFF);
            }
            send_all(client, resp.data(), static_cast<int>(resp.size()));

        } else if (fc == 0x06) {
            // FC06 — Write Single Register
            uint16_t value = static_cast<uint16_t>((pdu[3] << 8) | pdu[4]);
            Registry::get().apply_write_regs(addr, 1, &value);

            // Echo request as response
            send_all(client, header, 7);
            send_all(client, pdu.data(), static_cast<int>(pdu.size()));

        } else if (fc == 0x10) {
            // FC16 — Write Multiple Registers
            if (pdu.size() < 6u) { send_exception(client, header, fc, 0x03); continue; }
            uint8_t  byte_count = pdu[5];
            if (byte_count != qty * 2 || pdu.size() < static_cast<size_t>(6 + byte_count)) {
                send_exception(client, header, fc, 0x03);
                continue;
            }
            std::vector<uint16_t> regs(qty);
            for (uint16_t i = 0; i < qty; ++i) {
                regs[i] = static_cast<uint16_t>((pdu[6 + i*2] << 8) | pdu[7 + i*2]);
            }
            Registry::get().apply_write_regs(addr, qty, regs.data());

            // Build response (echo addr + qty)
            uint8_t resp[12];
            resp[0] = header[0]; resp[1] = header[1];
            resp[2] = 0;         resp[3] = 0;
            resp[4] = 0;         resp[5] = 6;
            resp[6] = header[6];
            resp[7] = fc;
            resp[8] = pdu[1]; resp[9]  = pdu[2];  // addr
            resp[10]= pdu[3]; resp[11] = pdu[4];  // qty
            send_all(client, resp, 12);

        } else {
            send_exception(client, header, fc, 0x01);  // Illegal Function
        }
    }
    sock_close(client);
}

// ── Server thread ─────────────────────────────────────────────────────────────
static void server_thread(uint16_t port) {
#ifdef _WIN32
    WSADATA wsa{};
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

    sock_t srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv == SOCK_INVALID) {
        std::fprintf(stderr, "[modbus] socket() failed: %d\n", sock_error);
        return;
    }

    int yes = 1;
#ifdef _WIN32
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&yes), sizeof(yes));
#else
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
#endif

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(port);

    if (bind(srv, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        std::fprintf(stderr, "[modbus] bind(%d) failed: %d — try 'make run' as root or use port 5020\n",
                     port, sock_error);
        sock_close(srv);
        return;
    }

    listen(srv, 4);
    std::printf("[modbus] Modbus TCP server listening on port %d\n", port);

    while (true) {
        sock_t client = accept(srv, nullptr, nullptr);
        if (client == SOCK_INVALID) continue;
        // Detach a thread per client (PLC use-case: few clients, low overhead)
        std::thread([client]{ handle_request(client); }).detach();
    }
}

void start_modbus_server(uint16_t port) {
    std::thread(server_thread, port).detach();
}

}  // namespace myplc::sim
