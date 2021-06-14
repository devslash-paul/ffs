#include "event_sync.h"
#include "types.h"

using namespace Im;

void EventStream::accept(const Im::FileEvent& evt) const
{
    if (this->subscriber != nullptr) {
        this->subscriber->receive(evt);
    }
}

void EventStream::setSubscriber(EventSubscriber* sub)
{
    this->subscriber = sub;
}
