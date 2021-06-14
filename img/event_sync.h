#include "types.h"

#pragma once

namespace Im {

class EventSubscriber {
public:
    virtual void receive(const Im::FileEvent&) {};
};

class EventStream {
public:
    void accept(const Im::FileEvent& evt) const;

    // Only one subscriber supported right now
    void setSubscriber(EventSubscriber* sub);

private:
    EventSubscriber* subscriber;
};

extern EventStream fsWatchStream;
}
