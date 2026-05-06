#include "commands.hpp"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>

namespace fs = std::filesystem;

namespace {
const std::string kCurrentVersion = "0.2.0";
const std::string kLatestInfoUrl = "https://raw.githubusercontent.com/DevZunxBR/Astra/main/latest.json";

std::string read_file(const fs::path& p) {
    std::ifstream in(p, std::ios::binary);
    if (!in) return "";
    return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

std::string extract_json_field(const std::string& json, const std::string& key) {
    std::regex re("\"" + key + "\"\\s*:\\s*\"([^\"]+)\"");
    std::smatch m;
    if (std::regex_search(json, m, re) && m.size() > 1) return m[1].str();
    return "";
}

int compare_version(const std::string& a, const std::string& b) {
    std::regex dot("\\.");
    std::sregex_token_iterator ia(a.begin(), a.end(), dot, -1), ib(b.begin(), b.end(), dot, -1), end;
    while (ia != end || ib != end) {
        int va = (ia != end) ? std::stoi(ia->str()) : 0;
        int vb = (ib != end) ? std::stoi(ib->str()) : 0;
        if (va < vb) return -1;
        if (va > vb) return 1;
        if (ia != end) ++ia;
        if (ib != end) ++ib;
    }
    return 0;
}
}

namespace cli {
int update_command(const std::vector<std::string>&) {
#ifdef _WIN32
    fs::path temp = fs::temp_directory_path() / "astra-update";
    fs::create_directories(temp);
    fs::path latest_json = temp / "latest.json";
    fs::path installer = temp / "AstraInstall.msi";

    std::string cmd_latest =
        "powershell -NoProfile -ExecutionPolicy Bypass -Command \""
        "$ProgressPreference='SilentlyContinue'; "
        "Invoke-WebRequest -UseBasicParsing -Uri '" + kLatestInfoUrl + "' -OutFile '" + latest_json.string() + "'\"";
    if (std::system(cmd_latest.c_str()) != 0) {
        std::cout << "Falha ao baixar latest.json\n";
        return 1;
    }

    const std::string json = read_file(latest_json);
    const std::string latest_version = extract_json_field(json, "version");
    const std::string msi_url = extract_json_field(json, "msi_url");
    if (latest_version.empty() || msi_url.empty()) {
        std::cout << "latest.json invalido. Campos obrigatorios: version, msi_url\n";
        return 1;
    }

    if (compare_version(latest_version, kCurrentVersion) <= 0) {
        std::cout << "Astra ja esta atualizado (versao " << kCurrentVersion << ").\n";
        return 0;
    }

    std::cout << "Nova versao encontrada: " << latest_version << "\n";
    std::cout << "Baixando atualizacao...\n";

    std::string cmd_msi =
        "powershell -NoProfile -ExecutionPolicy Bypass -Command \""
        "$ProgressPreference='SilentlyContinue'; "
        "Invoke-WebRequest -UseBasicParsing -Uri '" + msi_url + "' -OutFile '" + installer.string() + "'\"";
    if (std::system(cmd_msi.c_str()) != 0) {
        std::cout << "Falha ao baixar MSI de atualizacao.\n";
        return 1;
    }

    std::cout << "Instalando atualizacao...\n";
    std::string install_cmd = "msiexec /i \"" + installer.string() + "\" /passive /norestart";
    int code = std::system(install_cmd.c_str());
    if (code != 0) {
        std::cout << "Falha na instalacao automatica. Rode manualmente: " << installer.string() << "\n";
        return 1;
    }

    std::cout << "Atualizacao iniciada com sucesso.\n";
    return 0;
#else
    std::cout << "astra update disponivel apenas no Windows por enquanto.\n";
    return 1;
#endif
}
}
