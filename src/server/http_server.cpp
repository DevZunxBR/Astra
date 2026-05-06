#include "server.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace fs = std::filesystem;

namespace server {
namespace {
std::string content_type_for(const fs::path& p) {
    static const std::unordered_map<std::string, std::string> kMime = {
        {".html", "text/html; charset=utf-8"},
        {".css", "text/css; charset=utf-8"},
        {".js", "application/javascript; charset=utf-8"},
        {".json", "application/json; charset=utf-8"},
        {".svg", "image/svg+xml"},
        {".png", "image/png"},
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".ico", "image/x-icon"}
    };
    auto it = kMime.find(p.extension().string());
    if (it != kMime.end()) return it->second;
    return "application/octet-stream";
}

bool is_subpath(const fs::path& base, const fs::path& candidate) {
    std::error_code ec;
    fs::path base_abs = fs::weakly_canonical(base, ec);
    if (ec) return false;
    fs::path candidate_abs = fs::weakly_canonical(candidate, ec);
    if (ec) return false;
    std::string b = base_abs.generic_string();
    std::string c = candidate_abs.generic_string();
    if (!b.empty() && b.back() != '/') b.push_back('/');
    return c.rfind(b, 0) == 0 || c == base_abs.generic_string();
}
}

static std::string read_file(const fs::path& p) {
    std::ifstream in(p, std::ios::binary);
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

int start_http_server(const std::string& root_dir, int port, std::atomic<bool>& running) {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);
#endif
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) return 1;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(static_cast<uint16_t>(port));

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) return 1;
    if (listen(server_fd, 16) < 0) return 1;

    while (running) {
        sockaddr_in client{};
#ifdef _WIN32
        int len = sizeof(client);
#else
        socklen_t len = sizeof(client);
#endif
        int cfd = accept(server_fd, (sockaddr*)&client, &len);
        if (cfd < 0) continue;

        char buffer[4096] = {0};
#ifdef _WIN32
        int r = recv(cfd, buffer, sizeof(buffer)-1, 0);
#else
        int r = read(cfd, buffer, sizeof(buffer)-1);
#endif
        if (r <= 0) {
#ifdef _WIN32
            closesocket(cfd);
#else
            close(cfd);
#endif
            continue;
        }

        std::string req(buffer);
        std::string path = "/";
        auto p1 = req.find(" ");
        if (p1 != std::string::npos) {
            auto p2 = req.find(" ", p1 + 1);
            if (p2 != std::string::npos) path = req.substr(p1 + 1, p2 - p1 - 1);
        }
        if (path == "/") path = "/index.html";
        fs::path base = fs::path(root_dir);
        fs::path full = base / path.substr(1);
        if (!fs::exists(full) && path.find('.') == std::string::npos) full = fs::path(root_dir) / (path.substr(1) + ".html");

        std::string body;
        std::string status;
        std::string content_type = "text/html; charset=utf-8";
        if (fs::exists(full) && fs::is_regular_file(full) && is_subpath(base, full)) {
            body = read_file(full);
            status = "HTTP/1.1 200 OK\r\n";
            content_type = content_type_for(full);
        } else {
            body = "<h1>404</h1>";
            status = "HTTP/1.1 404 Not Found\r\n";
        }

        std::string headers = status + "Content-Type: " + content_type + "\r\nContent-Length: " + std::to_string(body.size()) + "\r\nConnection: close\r\n\r\n";
        std::string res = headers + body;
#ifdef _WIN32
        send(cfd, res.c_str(), (int)res.size(), 0);
        closesocket(cfd);
#else
        write(cfd, res.c_str(), res.size());
        close(cfd);
#endif
    }
#ifdef _WIN32
    closesocket(server_fd);
    WSACleanup();
#else
    close(server_fd);
#endif
    return 0;
}
}
