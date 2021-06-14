#include "activity.h"
#include "direc_discovery.h"
#include "event_sync.h"
#include "types.h"
#include <dirent.h>
#include <libfswatch.h>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>

#pragma once

namespace Im {

auto path_only_cmp = [](const Node& a, const Node& b) {
    return a.path < b.path;
};

class ImDB : Im::EventSubscriber {

public:
    ImDB(const std::string&, std::list<std::string>, int include_folders);

    // Get path information
    std::vector<Im::Node> paths_for(const std::string&);
    Im::Node* node_for(const std::string&);

private:
    void receive(const Im::FileEvent& event) override;

    Im::DirectoryTraverser dirTraverser;
    std::string base;
    std::list<std::string> filters;
    std::set<Im::Node, decltype(path_only_cmp)> knownPaths;
    std::map<std::string, Im::Node> mapped;
    void updateFile(const FileEvent& event);
    std::string normalizePath(const std::string& basicString);
    void removeFile(const FileEvent& event);
};
}
