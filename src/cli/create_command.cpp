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

    write_file(root / "src/index.html",
        "<!doctype html>\n"
        "<html lang=\"pt-BR\">\n"
        "  <head>\n"
        "    <meta charset=\"utf-8\" />\n"
        "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" />\n"
        "    <title>Astra App</title>\n"
        "    <link rel=\"stylesheet\" href=\"./main.css\" />\n"
        "  </head>\n"
        "  <body>\n"
        "    <main>\n"
        "      <h1>Astra</h1>\n"
        "      <p>Projeto criado com sucesso.</p>\n"
        "    </main>\n"
        "  </body>\n"
        "</html>\n");

    write_file(root / "src/main.css",
        "*{box-sizing:border-box}\n"
        "body{margin:0;font-family:Arial,sans-serif;background:#f6f7fb;color:#111;padding:2rem}\n"
        "main{max-width:760px;margin:0 auto}\n"
        "h1{margin:0 0 .5rem}\n");

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
