#include "commands.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

namespace {
void write_file(const fs::path& p, const std::string& c) {
    fs::create_directories(p.parent_path());
    std::ofstream out(p, std::ios::binary);
    out << c;
}

void create_base(const fs::path& root) {
    fs::create_directories(root / "src");
    fs::create_directories(root / "public");

    write_file(root / "astra.config.json",
        "{\n"
        "  \"name\": \"" + root.filename().string() + "\",\n"
        "  \"template\": \"default\",\n"
        "  \"build\": {\n"
        "    \"outDir\": \"dist\"\n"
        "  }\n"
        "}\n");

    write_file(root / "package.astra",
        "{\n"
        "  \"name\": \"" + root.filename().string() + "\",\n"
        "  \"version\": \"0.1.0\",\n"
        "  \"dependencias\": {\n"
        "    \"axios\": \"1.0\"\n"
        "  }\n"
        "}\n");

    write_file(root / "src/app.asf",
        "page App {\n"
        "  <main>\n"
        "    <h1>Astra</h1>\n"
        "    <p>Projeto criado com sucesso.</p>\n"
        "  </main>\n"
        "}\n");

    write_file(root / "src/app.asl",
        "main {\n"
        "  max-width: 760px;\n"
        "  margin: 0 auto;\n"
        "  padding: 2rem;\n"
        "  font-family: Arial, sans-serif;\n"
        "}\n"
        "h1 {\n"
        "  margin-bottom: .5rem;\n"
        "}\n");

}
}

namespace cli {
int create_command(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "Uso: astra create <nome-projeto>\n";
        return 1;
    }

    std::string name = args[0];
    fs::path root = fs::current_path() / name;
    if (fs::exists(root)) {
        std::cout << "Erro: pasta ja existe.\n";
        return 1;
    }

    create_base(root);
    std::cout << "Projeto criado em: " << root.string() << "\n";
    return 0;
}
}
