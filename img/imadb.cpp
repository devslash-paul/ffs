#include "imadb.h"
#include "direc_discovery.h"
#include "types.h"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <libgen.h>
#include <map>
#include <string>
#include <utility>
#include <vector>

using namespace Im;

ImDB::ImDB(const std::string& base, std::list<std::string> filters)
    : base(base)
    , knownPaths(path_only_cmp)
    , filters(std::move(filters))
{
    // TODO - support multiple base paths
    std::vector<std::string> basePaths({ base });
    // Set this up as the basis subscriber to all directory updates..
    // This means that rather than blocking i should really just supply
    // all the startup events to teh fsWatchStream
    Im::fsWatchStream.setSubscriber(this);
}

inline bool hasEnding(const std::string& fullString, const std::string& ending)
{
    if (fullString.length() >= ending.length()) {
        return fullString.compare(fullString.length() - ending.length(),
                   ending.length(), ending)
            == 0;
    } else {
        return false;
    }
}

// This should iam to allow filters to change over time
std::vector<Im::Node> ImDB::paths_for(const std::string& path)
{
    std::vector<Im::Node> nodes;
    std::copy_if(
        this->knownPaths.begin(), this->knownPaths.end(),
        std::back_inserter(nodes), [&](Im::Node it) {
            if (this->filters.empty()) {
                return true;
            }
            return std::any_of(
                this->filters.begin(), this->filters.end(),
                [&](const std::string& filter) { return hasEnding(it.name, filter); });
        });
    return nodes;
}

Im::Node* ImDB::node_for(const std::string& path)
{
    auto found = this->mapped.find(path);
    if (found != this->mapped.end()) {
        return &found->second;
    }
    return nullptr;
}

void ImDB::receive(const FileEvent& event)
{
    switch (event.type) {
    case DELETED:
        removeFile(event);
        break;
    case RENAMED:
        break;
    case CREATED:
    case UPDATED:
        updateFile(event);
        break;
    }
}

void ImDB::updateFile(const FileEvent& event)
{
    auto cpath = event.path.c_str();
    auto baseName = basename(strdup(cpath));
    struct stat s {
    };
    stat(event.path.c_str(), &s);
    const Node& pathNode = Node {
        .name = baseName,
        .size = s.st_size,
        .created = s.st_ctime,
        .modified = s.st_mtime,
        .accessed = s.st_atime,
        .path = event.path,
        .type = event.nodeType,
    };

    // I can probably just change things rather than do all of this
    // TODO: Make this more efficient by updating in place
    if (this->knownPaths.find(pathNode) != this->knownPaths.end()) {
        this->knownPaths.erase(pathNode);
    }

    if (event.nodeType == DT_REG) {
        //TODO: Directories don't seen to be working
        this->knownPaths.insert(pathNode);
        const std::string& normalPath = normalizePath(event.path);
        this->mapped["/" + pathNode.name] = pathNode;
    }
}

void ImDB::removeFile(const FileEvent& event)
{
    this->knownPaths.erase(Node {
        .path = event.path });
    const auto normalPath = normalizePath(event.path);
    this->mapped.erase(normalPath);
}

std::string ImDB::normalizePath(const std::string& path)
{
    // Here we just want to strip the base
    return path.substr(this->base.length(), path.size() - this->base.length());
}
