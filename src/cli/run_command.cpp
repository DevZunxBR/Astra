#include "commands.hpp"
#include "transpiler/transpiler.hpp"
#include <iostream>

namespace cli {
int run_command(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "Uso: astra run <arquivo.astra>\n";
        return 1;
    }
    return transpiler::run_astra_script(args[0]) ? 0 : 1;
}
}
