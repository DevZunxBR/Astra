#pragma once
#include <atomic>
#include <functional>
#include <string>

namespace server {
class LiveReloader {
public:
    LiveReloader();
    ~LiveReloader();
    void start(const std::string& root, std::function<void()> on_change);
    void stop();

private:
    std::atomic<bool> running_;
};

int start_http_server(const std::string& root_dir, int port, std::atomic<bool>& running);
}
