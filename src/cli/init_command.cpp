#include "commands.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

namespace {
void write_file_if_missing(const fs::path& p, const std::string& c, bool& created_any) {
    if (fs::exists(p)) return;
    fs::create_directories(p.parent_path());
    std::ofstream out(p, std::ios::binary);
    out << c;
    created_any = true;
}

std::string project_name_from_path(const fs::path& p) {
    std::string name = p.filename().string();
    if (name.empty()) return "astra-project";
    return name;
}
}

namespace cli {
int init_command(const std::vector<std::string>&) {
    fs::path root = fs::current_path();
    bool created_any = false;

    fs::create_directories(root / "src");
    fs::create_directories(root / "public");

    const std::string project_name = project_name_from_path(root);

    write_file_if_missing(
        root / "astra.config.json",
        "{\n"
        "  \"name\": \"" + project_name + "\",\n"
        "  \"template\": \"default\",\n"
        "  \"build\": {\n"
        "    \"outDir\": \"dist\"\n"
        "  }\n"
        "}\n",
        created_any
    );

    write_file_if_missing(
        root / "package.astra",
        "{\n"
        "  \"name\": \"" + project_name + "\",\n"
        "  \"version\": \"0.1.0\",\n"
        "  \"dependencias\": {\n"
        "    \"axios\": \"1.0\"\n"
        "  }\n"
        "}\n",
        created_any
    );

    write_file_if_missing(
        root / "src/app.asf",
        "page App {\n"
        "  <main>\n"
        "    <h1>Astra</h1>\n"
        "    <p>Interface feita com .asf</p>\n"
        "  </main>\n"
        "}\n",
        created_any
    );

    write_file_if_missing(
        root / "src/app.asl",
        "main {\n"
        "  max-width: 760px;\n"
        "  margin: 0 auto;\n"
        "  padding: 2rem;\n"
        "  font-family: Arial, sans-serif;\n"
        "}\n"
        "h1 {\n"
        "  margin-bottom: .5rem;\n"
        "}\n",
        created_any
    );

    if (created_any) {
        std::cout << "Astra inicializado em: " << root.string() << "\n";
    } else {
        std::cout << "Astra ja estava configurado neste projeto.\n";
    }

    return 0;
}
}
