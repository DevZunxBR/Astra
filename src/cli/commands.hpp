#pragma once
#include <string>
#include <vector>

namespace cli {
int create_command(const std::vector<std::string>& args);
int init_command(const std::vector<std::string>& args);
int dev_command(const std::vector<std::string>& args);
int build_command(const std::vector<std::string>& args);
int deploy_command(const std::vector<std::string>& args);
int run_command(const std::vector<std::string>& args);
int update_command(const std::vector<std::string>& args);
}
