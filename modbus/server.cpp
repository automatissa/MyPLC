#include "modbus/server.h"
#include "sim/registry.h"

#include <cstdio>
#include <cstring>
#include <thread>
#include <vector>

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
#  define SOCK_INVALID  INVALID_SOCKET
#  define sock_close    closesocket
#  define sock_errno    WSAGetLastError()
#else
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <unistd.h>
#  include <errno.h>
   using sock_t = int;
#  define SOCK_INVALID  (-1)
#  define sock_close    close
#  define sock_errno    errno
#endif

namespace myplc {

// ── Internal socket helpers ───────────────────────────────────────────────────
static bool _recv_all(sock_t s, uint8_t* buf, int n) {
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

static bool _send_all(sock_t s, const uint8_t* buf, int n) {
#ifdef _WIN32
    return send(s, reinterpret_cast<const char*>(buf), n, 0) == n;
#else
    return static_cast<int>(send(s, buf, static_cast<size_t>(n), 0)) == n;
#endif
}

static void _send_exception(sock_t s, const uint8_t* hdr, uint8_t fc, uint8_t code) {
    uint8_t resp[9] = {hdr[0], hdr[1], 0, 0, 0, 3, hdr[6],
                       static_cast<uint8_t>(fc | 0x80u), code};
    _send_all(s, resp, 9);
}

// ── Handle one client connection ──────────────────────────────────────────────
static void _handle_client(sock_t client) {
    uint8_t hdr[7];  // 6-byte MBAP + unit ID
    while (_recv_all(client, hdr, 7)) {
        uint16_t pdu_len = static_cast<uint16_t>((hdr[4] << 8) | hdr[5]);
        if (pdu_len < 2 || pdu_len > 256) break;

        std::vector<uint8_t> pdu(pdu_len - 1);
        if (!_recv_all(client, pdu.data(), static_cast<int>(pdu.size()))) break;

        uint8_t  fc   = pdu[0];
        uint16_t addr = static_cast<uint16_t>((pdu[1] << 8) | pdu[2]);
        uint16_t qty  = static_cast<uint16_t>((pdu[3] << 8) | pdu[4]);
        auto& reg     = sim::Registry::get();

        if (fc == 0x03) {
            // FC03 — Read Holding Registers
            if (qty < 1 || qty > 125) { _send_exception(client, hdr, fc, 0x03); continue; }
            std::vector<uint16_t> regs(qty, 0);
            reg.fill_response_regs(addr, qty, regs.data());

            std::vector<uint8_t> resp(9 + qty * 2);
            uint16_t rlen = static_cast<uint16_t>(3 + qty * 2);
            resp[0]=hdr[0]; resp[1]=hdr[1]; resp[2]=0; resp[3]=0;
            resp[4]=static_cast<uint8_t>(rlen>>8); resp[5]=static_cast<uint8_t>(rlen&0xFF);
            resp[6]=hdr[6]; resp[7]=fc; resp[8]=static_cast<uint8_t>(qty*2);
            for (uint16_t i = 0; i < qty; ++i) {
                resp[9+i*2] = static_cast<uint8_t>(regs[i] >> 8);
                resp[10+i*2]= static_cast<uint8_t>(regs[i] & 0xFF);
            }
            _send_all(client, resp.data(), static_cast<int>(resp.size()));

        } else if (fc == 0x06) {
            // FC06 — Write Single Register
            uint16_t val = static_cast<uint16_t>((pdu[3] << 8) | pdu[4]);
            reg.apply_write_regs(addr, 1, &val);
            // Echo
            _send_all(client, hdr, 7);
            _send_all(client, pdu.data(), static_cast<int>(pdu.size()));

        } else if (fc == 0x10) {
            // FC16 — Write Multiple Registers
            if (pdu.size() < 6u) { _send_exception(client, hdr, fc, 0x03); continue; }
            uint8_t bc = pdu[5];
            if (bc != qty * 2 || pdu.size() < static_cast<size_t>(6 + bc)) {
                _send_exception(client, hdr, fc, 0x03); continue;
            }
            std::vector<uint16_t> regs(qty);
            for (uint16_t i = 0; i < qty; ++i)
                regs[i] = static_cast<uint16_t>((pdu[6+i*2] << 8) | pdu[7+i*2]);
            reg.apply_write_regs(addr, qty, regs.data());

            uint8_t resp[12] = {hdr[0],hdr[1],0,0,0,6,hdr[6],fc,
                                pdu[1],pdu[2],pdu[3],pdu[4]};
            _send_all(client, resp, 12);

        } else {
            _send_exception(client, hdr, fc, 0x01);  // Illegal Function
        }
    }
    sock_close(client);
}

// ── Server thread ─────────────────────────────────────────────────────────────
static void _server_thread(uint16_t port) {
#ifdef _WIN32
    WSADATA wsa{};
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

    sock_t srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv == SOCK_INVALID) {
        std::fprintf(stderr, "[modbus] socket() failed (%d)\n", sock_errno);
        return;
    }
    int yes = 1;
#ifdef _WIN32
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&yes), sizeof(yes));
#else
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
#endif

    sockaddr_in sa{};
    sa.sin_family      = AF_INET;
    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_port        = htons(port);

    if (bind(srv, reinterpret_cast<sockaddr*>(&sa), sizeof(sa)) < 0) {
        std::fprintf(stdout,
            "[modbus] ERREUR bind(%d) (%d) — Windows: pare-feu ? Linux: sudo make run\n",
            port, sock_errno);
        std::fflush(stdout);
        sock_close(srv);
        return;
    }
    listen(srv, 8);
    std::printf("[modbus] Serveur prêt sur le port %d — en attente de l'ESP32...\n", port);
    std::fflush(stdout);

    while (true) {
        sock_t client = accept(srv, nullptr, nullptr);
        if (client == SOCK_INVALID) continue;
        std::printf("[modbus] ESP32 connecté !\n");
        std::fflush(stdout);
        std::thread([client]{ _handle_client(client); }).detach();
    }
}

// ── Public API ────────────────────────────────────────────────────────────────
void ModbusTcpServer::print_map() const {
    sim::Registry::get().print_mb_map();
}

void ModbusTcpServer::start(uint16_t port) {
    print_map();
    std::thread(_server_thread, port).detach();
}

}  // namespace myplc
