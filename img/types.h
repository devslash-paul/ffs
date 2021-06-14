#include <string>
#include <sys/stat.h>
#include <sys/types.h>

#pragma once

namespace Im {

struct Node {
    std::string name;
    off_t size;
    long created;
    long modified;
    long accessed;
    std::string path;
    unsigned char type;

//    bool operator<(const Node& y) const
//    {
//        return path.compare(y.path) < 0;
//    }
};

enum EventType {
    CREATED,
    DELETED,
    RENAMED,
    UPDATED
};

struct FileEvent {
    EventType type;
    unsigned char nodeType;
    std::string path;
};

}
