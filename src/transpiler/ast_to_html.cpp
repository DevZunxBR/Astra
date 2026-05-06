#include "transpiler.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>

namespace fs = std::filesystem;

namespace {
std::string read_file(const fs::path& p) {
    std::ifstream in(p, std::ios::binary);
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

void write_file(const fs::path& p, const std::string& c) {
    fs::create_directories(p.parent_path());
    std::ofstream out(p, std::ios::binary);
    out << c;
}

std::string minify_text(const std::string& src) {
    std::string out;
    out.reserve(src.size());
    bool prev_space = false;
    for (char ch : src) {
        if (ch == '\n' || ch == '\r' || ch == '\t' || ch == ' ') {
            if (!prev_space) out.push_back(' ');
            prev_space = true;
            continue;
        }
        out.push_back(ch);
        prev_space = false;
    }
    return out;
}

std::string extract_render(std::string src) {
    auto pos = src.find("render");
    if (pos == std::string::npos) return "<h1>Astra page</h1>";
    auto lb = src.find('{', pos);
    if (lb == std::string::npos) return "<h1>Astra page</h1>";

    size_t i = lb;
    int depth = 0;
    size_t rb = std::string::npos;
    for (; i < src.size(); ++i) {
        if (src[i] == '{') depth++;
        else if (src[i] == '}') {
            depth--;
            if (depth == 0) {
                rb = i;
                break;
            }
        }
    }
    if (rb == std::string::npos || rb <= lb) return "<h1>Astra page</h1>";

    std::string html = src.substr(lb + 1, rb - lb - 1);
    html = std::regex_replace(html, std::regex("\\{props\\.title\\}"), "Astra App");
    html = std::regex_replace(html, std::regex("\\{[^}]*\\}"), "");
    html = std::regex_replace(html, std::regex("<layout[^>]*>"), "<div>");
    html = std::regex_replace(html, std::regex("</layout>"), "</div>");
    html = std::regex_replace(html, std::regex("for [^{]*\\{"), "");
    return html;
}

std::string extract_style(std::string src) {
    auto pos = src.find("style");
    if (pos == std::string::npos) return "";
    auto lb = src.find('{', pos);
    if (lb == std::string::npos) return "";

    size_t i = lb;
    int depth = 0;
    size_t rb = std::string::npos;
    for (; i < src.size(); ++i) {
        if (src[i] == '{') depth++;
        else if (src[i] == '}') {
            depth--;
            if (depth == 0) {
                rb = i;
                break;
            }
        }
    }
    if (rb == std::string::npos || rb <= lb) return "";
    return src.substr(lb + 1, rb - lb - 1);
}

void copy_static_tree(const fs::path& src_root, const fs::path& out_root, bool& generated_any_page) {
    for (auto& e : fs::recursive_directory_iterator(src_root)) {
        if (!e.is_regular_file()) continue;
        auto ext = e.path().extension().string();
        if (ext != ".js") continue;
        auto rel = fs::relative(e.path(), src_root);
        write_file(out_root / rel, read_file(e.path()));
        if (ext == ".html") generated_any_page = true;
    }
}

std::string extract_asf_markup(std::string src) {
    auto pos = src.find("page");
    if (pos == std::string::npos) return "<h1>Astra page</h1>";
    auto lb = src.find('{', pos);
    if (lb == std::string::npos) return "<h1>Astra page</h1>";
    size_t i = lb;
    int depth = 0;
    size_t rb = std::string::npos;
    for (; i < src.size(); ++i) {
        if (src[i] == '{') depth++;
        else if (src[i] == '}') {
            depth--;
            if (depth == 0) { rb = i; break; }
        }
    }
    if (rb == std::string::npos || rb <= lb) return "<h1>Astra page</h1>";
    return src.substr(lb + 1, rb - lb - 1);
}

void compile_astra_files(const fs::path& src_root, const fs::path& out_root, bool& generated_any_page) {
    std::string markup;
    std::string style;
    for (auto& e : fs::recursive_directory_iterator(src_root)) {
        if (!e.is_regular_file()) continue;
        const std::string ext = e.path().extension().string();
        if (ext == ".asf") {
            markup = extract_asf_markup(read_file(e.path()));
            generated_any_page = true;
        } else if (ext == ".asl") {
            style += read_file(e.path()) + "\n";
        }
    }
    if (generated_any_page) {
        if (style.empty()) style = "body{font-family:Arial,sans-serif;padding:2rem}";
        write_file(out_root / "index.html", "<!doctype html><html><head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><title>Astra App</title><link rel=\"stylesheet\" href=\"./app.css\"></head><body>" + markup + "<script src=\"./app.js\"></script></body></html>");
        write_file(out_root / "app.css", style);
        write_file(out_root / "app.js", "console.log('Astra app loaded');");
    }
}

bool is_safe_out_dir(const fs::path& root, const fs::path& out_root) {
    if (out_root.empty()) return false;
    std::error_code ec;
    fs::path root_abs = fs::weakly_canonical(root, ec);
    if (ec) return false;

    fs::path parent = out_root.parent_path();
    if (parent.empty()) parent = root_abs;
    fs::path parent_abs = fs::weakly_canonical(parent, ec);
    if (ec) return false;

    std::string root_s = root_abs.generic_string();
    std::string parent_s = parent_abs.generic_string();
    if (!root_s.empty() && root_s.back() != '/') root_s.push_back('/');
    if (!parent_s.empty() && parent_s.back() != '/') parent_s.push_back('/');
    return parent_s.rfind(root_s, 0) == 0;
}
}

namespace transpiler {
bool compile_project(const std::string& project_root, const std::string& out_dir, const BuildOptions& options) {
    fs::path root = fs::path(project_root);
    fs::path src_root = root / "src";
    if (!fs::exists(src_root)) return false;

    fs::path out_root = fs::path(out_dir);
    if (!out_root.is_absolute()) out_root = root / out_root;

    if (!is_safe_out_dir(root, out_root)) {
        std::cout << "Build falhou: outDir invalido ou inseguro: " << out_root.string() << "\n";
        return false;
    }

    if (fs::exists(out_root)) fs::remove_all(out_root);
    fs::create_directories(out_root);

    bool generated_any_page = false;
    compile_astra_files(src_root, out_root, generated_any_page);
    copy_static_tree(src_root, out_root, generated_any_page);
    if (!generated_any_page) {
        write_file(out_root / "index.html", "<!doctype html><html><body><h1>Astra</h1></body></html>");
    }

    write_file(out_root / "app.js", "setInterval(()=>fetch(location.pathname).catch(()=>{}),2000);");

    if (options.minify) {
        for (auto& e : fs::recursive_directory_iterator(out_root)) {
            if (!e.is_regular_file()) continue;
            const std::string ext = e.path().extension().string();
            if (ext != ".html" && ext != ".css" && ext != ".js") continue;
            write_file(e.path(), minify_text(read_file(e.path())));
        }
    }
    return true;
}

bool run_astra_script(const std::string& file_path) {
    if (!fs::exists(file_path)) {
        std::cout << "Arquivo nao encontrado: " << file_path << "\n";
        return false;
    }
    auto src = read_file(file_path);
    std::cout << "[astra run] Executando script: " << file_path << "\n";
    std::cout << "--- Conteudo parseado ---\n" << extract_render(src) << "\n";
    return true;
}
}
