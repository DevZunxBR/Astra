#pragma once
#include <string>

namespace transpiler {
struct BuildOptions {
    bool minify = false;
};

bool compile_project(const std::string& project_root, const std::string& out_dir, const BuildOptions& options);
bool run_astra_script(const std::string& file_path);
}
