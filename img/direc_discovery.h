#include "types.h"
#include "event_sync.h"
#include <list>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#pragma once

namespace Im {

typedef typename std::map<std::string, std::unique_ptr<std::list<std::string>>> namecache;
class DirectoryTraverser {

public:
    explicit DirectoryTraverser(bool, Im::EventStream&);
    ~DirectoryTraverser();
    // Get a flattened list of all nodes for a path
    void flatten_dir(const std::string&);

private:
    bool include_folders;
    namecache nc;
    Im::EventStream& evt;

    std::list<std::string> list_directory(const std::string& path);
    std::string findName(std::string);
    std::thread runThread;
};

}
