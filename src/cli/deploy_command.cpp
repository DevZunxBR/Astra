#include "commands.hpp"
#include "server/server.hpp"
#include "transpiler/transpiler.hpp"
#include <atomic>
#include <filesystem>
#include <iostream>

namespace cli {
namespace fs = std::filesystem;

static fs::path detect_project_root() {
    fs::path cwd = fs::current_path();
    if (fs::exists(cwd / "astra.config.json")) return cwd;

    fs::path found;
    int count = 0;
    for (const auto& entry : fs::directory_iterator(cwd)) {
        if (!entry.is_directory()) continue;
        fs::path candidate = entry.path();
        if (fs::exists(candidate / "astra.config.json")) {
            found = candidate;
            ++count;
        }
    }

    if (count == 1) return found;
    return {};
}

int deploy_command(const std::vector<std::string>&) {
    fs::path project_root = detect_project_root();
    if (project_root.empty()) {
        std::cout << "Deploy falhou: rode dentro da pasta do projeto Astra (onde tem astra.config.json).\n";
        return 1;
    }

    fs::path out_path = project_root / "dist";
    if (!transpiler::compile_project(project_root.string(), out_path.string(), {true})) {
        std::cout << "Deploy falhou: erro no build\n";
        return 1;
    }

    std::cout << "Deploy local em http://localhost:3000\nCtrl+C para sair\n";
    std::atomic<bool> running{true};
    return server::start_http_server(out_path.string(), 3000, running);
}
}
