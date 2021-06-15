#include "types.h"
#include <mutex>
#include <vector>
#include <memory>
#include <list>

#pragma once

namespace Im {

class EventSubscriber {
public:
    virtual void receive(const Im::FileEvent&) {};
};

class EventStream {
public:
    void accept(const Im::FileEvent& evt);

    // Only one subscriber supported right now
    void setSubscriber(EventSubscriber* sub);

private:
    std::vector<std::shared_ptr<const Im::FileEvent>> event_buffer;
    EventSubscriber* subscriber;
    std::mutex buffer_mutex;
};

extern EventStream fsWatchStream;
}
