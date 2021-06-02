#include "types.h"
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

#pragma once

namespace Im {

typedef typename std::map<std::string, std::unique_ptr<std::list<std::string>>> namecache;
class DirectoryTraverser {

public:
    explicit DirectoryTraverser(bool);
    // Get a flattened list of all nodes for a path
    std::vector<Im::Node> flatten_dir(const std::string&);

private:
    bool include_folders;
    namecache nc;

    std::vector<Im::Node> listd(const std::string&);
    std::string findName(std::string);
};

};
