#include "activity.h"
#include "event_sync.h"
#include <cmonitor.h>
#include <dirent.h>
#include <event.hpp>
#include <functional>
#include <libfswatch.h>
#include <monitor_factory.hpp>
#include <thread>

using namespace Im;

static void on_event(const std::vector<fsw::event>& evts, void* v)
{
    for (const auto& evt : evts) {
        for (auto flag : evt.get_flags()) {
            switch (flag) {
            case fsw_event_flag::Created:
                // eg - touch. Often followed by an update
                fsWatchStream.accept(FileEvent { //
                    .type = Im::CREATED,
                    .nodeType = DT_REG,
                    .path = evt.get_path() });
                break;
            case fsw_event_flag::Removed:
                fsWatchStream.accept({ //
                    .type = Im::DELETED,
                    .path = evt.get_path() });
                break;
            case fsw_event_flag::Renamed:
                fsWatchStream.accept({ //
                    .type = Im::RENAMED,
                    .path = evt.get_path() });
                break;
            case fsw_event_flag::Updated:
                fsWatchStream.accept(FileEvent { //
                    .type = Im::UPDATED,
                    .nodeType = DT_REG, // FIXME; not everything is a file
                    .path = evt.get_path() });
                break;
            default:
                break;
            }
        }
    }
}

ActivityService::ActivityService(const std::vector<std::string>& paths)
{
    auto mon = fsw::monitor_factory::create_monitor(fsw_monitor_type::system_default_monitor_type,
        paths,
        &on_event);
    mon->set_recursive(true);
    mon->set_follow_symlinks(false);
    this->activity_monitor = mon;
    // Kick off the EVT thread
    this->activity_thread = std::thread([&]() {
        this->activity_monitor->start();
    });
}

ActivityService::~ActivityService()
{
    this->activity_monitor->stop();
    this->activity_thread.join();
}
