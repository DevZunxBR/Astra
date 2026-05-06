#include "commands.hpp"
#include "transpiler/transpiler.hpp"
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

int build_command(const std::vector<std::string>&) {
    fs::path project_root = detect_project_root();
    if (project_root.empty()) {
        std::cout << "Build falhou: rode dentro da pasta do projeto Astra (onde tem astra.config.json).\n";
        return 1;
    }

    fs::path out_path = project_root / "dist";
    if (!transpiler::compile_project(project_root.string(), out_path.string(), {true})) {
        std::cout << "Build falhou: verifique se a pasta src/ existe no projeto.\n";
        return 1;
    }

    std::cout << "Build concluido em " << out_path.string() << "\n";
    return 0;
}
}
