#include "activity.h"
#include "event_sync.h"
#include <chrono>
#include <cmonitor.h>
#include <event.hpp>
#include <functional>
#include <libfswatch.h>
#include <monitor_factory.hpp>
#include <thread>

using namespace Im;

static void on_event(const std::vector<fsw::event>& evt, void* v)
{
    std::cout << "Event called " << std::endl;
    for (auto v : evt) {
        for (auto flag : v.get_flags()) {
            switch (flag) {
            case fsw_event_flag::Created:
                fsWatchStream.accept({ //
                    .type = Im::CREATED,
                    .path = v.get_path() });
                break;
            case fsw_event_flag::Removed:
                fsWatchStream.accept({ //
                    .type = Im::DELETED,
                    .path = v.get_path() });
                break;
            case fsw_event_flag::Renamed:
                fsWatchStream.accept({ //
                    .type = Im::RENAMED,
                    .path = v.get_path() });
                break;
            case fsw_event_flag::Updated:
                fsWatchStream.accept({ //
                    .type = Im::UPDATED,
                    .path = v.get_path() });
                break;
            default:
                break;
            }
        }
    }
}

ActivityService::ActivityService(const std::vector<std::string>& paths)
{
    this->paths = paths;
    auto mon = fsw::monitor_factory::create_monitor(fsw_monitor_type::system_default_monitor_type,
        paths,
        &on_event);
    mon->set_recursive(true);
    mon->set_follow_symlinks(false);
    this->activity_monitor = mon;
    // Kick off the EVT thread
    std::cout << "Kicking off thread" << std::endl;
    this->activity_thread = std::thread([&]() {
        this->activity_monitor->start();
    });
//    this->checker = std::thread([&]() {
//        while (true) {
//            std::cout << "IS RUNNING - " << this->activity_monitor->is_running() << std::endl;
//            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//        }
//    });
    std::cout << "Done" << std::endl;
}

ActivityService::~ActivityService()
{
    this->activity_monitor->stop();
    this->activity_thread.join();
}
