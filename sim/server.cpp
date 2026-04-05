#include "sim/server.h"
#include "sim/registry.h"

#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <algorithm>

// ── Platform socket abstraction ───────────────────────────────────────────────
#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <winsock2.h>
    #include <ws2tcpip.h>
    using socket_t = SOCKET;
    #define INVALID_SOCK  INVALID_SOCKET
    #define CLOSE_SOCK(s) closesocket(s)
    static void plat_init() {
        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);
    }
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    using socket_t = int;
    #define INVALID_SOCK  (-1)
    #define CLOSE_SOCK(s) ::close(s)
    static void plat_init() {}
#endif

namespace myplc::sim {

// ── Embedded dashboard HTML ───────────────────────────────────────────────────
static const char* INDEX_HTML = R"HTML(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>MyPLC Simulator</title>
  <style>
    *, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }
    body {
      font-family: 'Courier New', monospace;
      background: #0d1117;
      color: #c9d1d9;
      padding: 24px;
      min-height: 100vh;
    }
    header {
      display: flex;
      align-items: center;
      gap: 14px;
      padding-bottom: 16px;
      margin-bottom: 20px;
      border-bottom: 1px solid #21262d;
    }
    h1 { color: #58a6ff; font-size: 1.4rem; letter-spacing: 0.02em; }
    .badge {
      padding: 3px 10px;
      border-radius: 12px;
      font-size: 0.72rem;
      font-weight: 700;
      letter-spacing: 0.05em;
    }
    .running { background:#1b4332; color:#3fb950; }
    .stopped { background:#3d1717; color:#f85149; }
    .stats { margin-left:auto; font-size:0.82rem; color:#8b949e; }

    table {
      width: 100%;
      border-collapse: collapse;
      background: #161b22;
      border-radius: 8px;
      overflow: hidden;
      box-shadow: 0 0 0 1px #30363d;
    }
    thead tr { background: #21262d; }
    th {
      padding: 10px 16px;
      text-align: left;
      color: #8b949e;
      font-size: 0.75rem;
      text-transform: uppercase;
      letter-spacing: 0.06em;
      border-bottom: 1px solid #30363d;
    }
    td { padding: 8px 16px; border-bottom: 1px solid #21262d; }
    tr:last-child td { border-bottom: none; }
    tr:hover td { background: #1c2128; }

    .n  { color: #79c0ff; font-weight: 600; }
    .t  { color: #d2a8ff; font-size: 0.78rem; }
    .v  { color: #3fb950; }
    .vf { color: #6e7681; }

    .form { display:flex; gap:6px; align-items:center; }
    input[type=text] {
      background:#0d1117; color:#c9d1d9;
      border:1px solid #30363d; border-radius:4px;
      padding:4px 8px; font-family:monospace; font-size:0.83rem; width:110px;
    }
    input[type=text]:focus { outline:none; border-color:#58a6ff; }
    button {
      background:#238636; color:#fff; border:none; border-radius:4px;
      padding:4px 12px; cursor:pointer; font-size:0.78rem; font-family:monospace;
    }
    button:hover { background:#2ea043; }

    @keyframes hi { from { background:#1b4332; } to { background:transparent; } }
    .flash td { animation: hi 0.4s ease; }
  </style>
</head>
<body>
  <header>
    <h1>&#9654; MyPLC Simulator</h1>
    <span id="badge" class="badge stopped">STOPPED</span>
    <div class="stats">
      Refresh&nbsp;<span id="hz">-</span>&nbsp;|&nbsp;Cycle&nbsp;<span id="cyc">0</span>
    </div>
  </header>

  <table>
    <thead>
      <tr><th>Variable</th><th>Type</th><th>Value</th><th>Write</th></tr>
    </thead>
    <tbody id="tb"></tbody>
  </table>

  <script>
    let ready = false, cyc = 0, t0 = Date.now();

    function fmt(type, val) {
      if (type === 'BOOL')
        return val === 'true'
          ? '<span class="v">&#9646;&#9646; TRUE</span>'
          : '<span class="vf">&#9633;&#9633; FALSE</span>';
      return '<span class="v">' + val + '</span>';
    }

    async function poll() {
      try {
        const r   = await fetch('/api/vars');
        const arr = await r.json();
        const tb  = document.getElementById('tb');

        if (!ready) {
          tb.innerHTML = '';
          arr.forEach(v => {
            const tr = document.createElement('tr');
            tr.id = 'r_' + v.name;
            tr.innerHTML =
              '<td class="n">' + v.name + '</td>' +
              '<td class="t">' + v.type + '</td>' +
              '<td id="v_' + v.name + '">' + fmt(v.type, v.value) + '</td>' +
              '<td><div class="form">' +
                '<input type="text" id="i_' + v.name + '" placeholder="' + v.value + '">' +
                '<button onclick="wr(\'' + v.name + '\',\'' + v.type + '\')">Set</button>' +
              '</div></td>';
            tb.appendChild(tr);
          });
          ready = true;
        } else {
          arr.forEach(v => {
            const el = document.getElementById('v_' + v.name);
            if (el) el.innerHTML = fmt(v.type, v.value);
          });
        }

        cyc++;
        const hz = (cyc / ((Date.now() - t0) / 1000)).toFixed(1);
        document.getElementById('cyc').textContent  = cyc;
        document.getElementById('hz').textContent   = hz + ' Hz';
        document.getElementById('badge').textContent = 'RUNNING';
        document.getElementById('badge').className  = 'badge running';
      } catch (_) {
        document.getElementById('badge').textContent = 'DISCONNECTED';
        document.getElementById('badge').className  = 'badge stopped';
        ready = false;
      }
    }

    async function wr(name, type) {
      let v = document.getElementById('i_' + name).value.trim();
      if (type === 'BOOL')
        v = (v === '1' || v.toLowerCase() === 'true') ? 'true' : 'false';
      await fetch('/api/vars/' + name, { method:'POST', body: v }).catch(()=>{});
      const row = document.getElementById('r_' + name);
      if (row) { row.classList.remove('flash'); void row.offsetWidth; row.classList.add('flash'); }
    }

    setInterval(poll, 500);
    poll();
  </script>
</body>
</html>
)HTML";

// ── HTTP helpers ──────────────────────────────────────────────────────────────
static void send_response(socket_t s, int code,
                           const std::string& ctype,
                           const std::string& body) {
    const char* reason =
        (code == 200) ? "OK" : (code == 204) ? "No Content" : "Not Found";
    std::ostringstream h;
    h << "HTTP/1.1 " << code << " " << reason << "\r\n"
      << "Content-Type: "                << ctype       << "\r\n"
      << "Access-Control-Allow-Origin: " << "*"         << "\r\n"
      << "Content-Length: "              << body.size() << "\r\n"
      << "Connection: close\r\n\r\n";
    std::string hdr = h.str();
    ::send(s, hdr.c_str(),  (int)hdr.size(),  0);
    if (!body.empty())
        ::send(s, body.c_str(), (int)body.size(), 0);
}

static void handle_client(socket_t client) {
    char buf[8192] = {};
    int  n = ::recv(client, buf, (int)sizeof(buf) - 1, 0);
    if (n <= 0) { CLOSE_SOCK(client); return; }

    std::string req(buf, n);

    // Parse first request line: METHOD PATH HTTP/...
    auto crlf = req.find("\r\n");
    std::string first = (crlf != std::string::npos) ? req.substr(0, crlf) : req;
    std::istringstream ls(first);
    std::string method, path;
    ls >> method >> path;

    // Strip query string
    auto q = path.find('?');
    if (q != std::string::npos) path = path.substr(0, q);

    if (method == "GET" && (path == "/" || path == "/index.html")) {
        send_response(client, 200, "text/html; charset=utf-8", INDEX_HTML);
    }
    else if (method == "GET" && path == "/api/vars") {
        send_response(client, 200, "application/json",
                      Registry::get().to_json());
    }
    else if (method == "POST" && path.size() > 10 &&
             path.substr(0, 10) == "/api/vars/") {
        std::string name = path.substr(10);
        // Body follows the blank line after headers
        auto sep = req.find("\r\n\r\n");
        std::string body = (sep != std::string::npos) ? req.substr(sep + 4) : "";
        // Trim whitespace
        auto ltrim = [](std::string& s) {
            s.erase(0, s.find_first_not_of(" \t\r\n"));
        };
        auto rtrim = [](std::string& s) {
            auto p = s.find_last_not_of(" \t\r\n");
            if (p != std::string::npos) s.erase(p + 1);
        };
        ltrim(body); rtrim(body);
        ltrim(name); rtrim(name);
        Registry::get().write(name, body);
        send_response(client, 204, "text/plain", "");
    }
    else if (method == "OPTIONS") {
        send_response(client, 204, "text/plain", "");
    }
    else {
        send_response(client, 404, "text/plain", "Not Found");
    }
    CLOSE_SOCK(client);
}

// ── Server loop (runs in background thread) ───────────────────────────────────
static void server_loop(int port) {
    plat_init();

    socket_t srv = ::socket(AF_INET, SOCK_STREAM, 0);
    if (srv == INVALID_SOCK) {
        std::cerr << "[sim] socket() failed\n";
        return;
    }
    int opt = 1;
    ::setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons((uint16_t)port);

    if (::bind(srv, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "[sim] bind() failed on port " << port << "\n";
        CLOSE_SOCK(srv);
        return;
    }
    ::listen(srv, 16);
    std::cout << "[sim] Dashboard  →  http://localhost:" << port << "\n";

    while (true) {
        socket_t client = ::accept(srv, nullptr, nullptr);
        if (client == INVALID_SOCK) continue;
        handle_client(client);
    }
}

void start_server(int port) {
    std::thread(server_loop, port).detach();
}

}  // namespace myplc::sim
