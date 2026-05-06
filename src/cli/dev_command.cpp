#include "commands.hpp"
#include "server/server.hpp"
#include <atomic>
#include <filesystem>
#include <iostream>

namespace cli {
int dev_command(const std::vector<std::string>&) {
    namespace fs = std::filesystem;
    fs::path project_root = fs::current_path();
    if (!fs::exists(project_root / "src")) {
        std::cout << "Erro: rode 'astra dev' dentro da pasta do projeto (onde existe src/).\n";
        return 1;
    }

    std::atomic<bool> running{true};
    server::LiveReloader lr;

    lr.start((project_root / "src").string(), [](){
        std::cout << "[astra] alteracao detectada em src/\n";
    });

    std::cout << "Servidor Astra em http://localhost:3000\nCtrl+C para sair\n";
    int code = server::start_http_server((project_root / "src").string(), 3000, running);
    lr.stop();
    return code;
}
}
