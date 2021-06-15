#include "event_sync.h"
#include "types.h"
#include <mutex>
#include <thread>
#include <vector>

using namespace Im;

void EventStream::accept(const Im::FileEvent& evt)
{
    std::lock_guard<std::mutex> guard(this->buffer_mutex);
    if (this->subscriber != nullptr) {
        this->subscriber->receive(evt);
    } else {
        this->event_buffer.push_back(std::make_shared<Im::FileEvent>(evt));
    }
}

void EventStream::setSubscriber(EventSubscriber* sub)
{
    // FIXME: Probably can do this faster
    std::lock_guard<std::mutex> guard(this->buffer_mutex);
    // is underway
    this->subscriber = sub;
    // Now we clear the buffer
    for (const auto& item : this->event_buffer) {
        subscriber->receive(*item);
    }
    this->event_buffer.clear();
}
