#include <monitor.hpp>
#include <string>
#include <thread>
#include <vector>

#pragma once

namespace Im {

class ActivityService {

public:
    explicit ActivityService(const std::vector<std::string>& paths);
    ~ActivityService();

private:
    fsw::monitor* activity_monitor;
    std::thread activity_thread;
};
}
