#include "event_sync.h"
#include "types.h"
#include <iostream>
#include <utility>

using namespace Im;

//0x00007fd86b606550
//0x00007fd86b6062e0
void EventStream::accept(const Im::FileEvent& evt)
{
    if (this->subscriber != nullptr) {
        this->subscriber->receive(evt);
    }
}

void EventStream::setSubscriber(EventSubscriber* sub)
{
    this->subscriber = sub;
}
