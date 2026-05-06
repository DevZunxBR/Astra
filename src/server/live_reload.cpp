#include "server.hpp"
#include <chrono>
#include <filesystem>
#include <thread>
#include <unordered_map>

namespace fs = std::filesystem;

namespace server {
static std::unordered_map<std::string, fs::file_time_type> scan(const std::string& root) {
    std::unordered_map<std::string, fs::file_time_type> map;
    if (!fs::exists(root)) return map;
    for (auto& e : fs::recursive_directory_iterator(root)) {
        if (!e.is_regular_file()) continue;
        if (e.path().extension() == ".astra") map[e.path().string()] = fs::last_write_time(e.path());
    }
    return map;
}

LiveReloader::LiveReloader() : running_(false) {}
LiveReloader::~LiveReloader() { stop(); }

void LiveReloader::start(const std::string& root, std::function<void()> on_change) {
    running_ = true;
    std::thread([this, root, on_change]() {
        auto last = scan(root);
        while (running_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(700));
            auto cur = scan(root);
            if (cur != last) {
                last = cur;
                on_change();
            }
        }
    }).detach();
}

void LiveReloader::stop() { running_ = false; }
}
