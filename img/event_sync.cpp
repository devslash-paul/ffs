#include "event_sync.h"
#include "types.h"
#include <iostream>
#include <utility>

using namespace Im;

//0x00007fd86b606550
//0x00007fd86b6062e0
void EventStream::accept(const Im::FileEvent& evt)
{
    std::cout << "Subscriber checked" << std::endl;
    if (this->subscriber != nullptr) {
        std::cout << "Subscriber sent" << std::endl;
        this->subscriber->receive(evt);
    }
}

void EventStream::setSubscriber(EventSubscriber* sub)
{
    std::cout << "Subscribe set" << std::endl;
    this->subscriber = sub;
}
