#include "imadb.h"
#include "activity.h"
#include "direc_discovery.h"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <libgen.h>
#include <map>
#include <utility>
#include <vector>

using namespace Im;

ImDB::ImDB(const std::string& base, std::list<std::string> filters,
    int include_folders)
{
    std::vector<std::string> basePaths({ base });
    this->base = base;
    this->filters = std::move(filters);
    this->include_folders = include_folders;

    Im::fsWatchStream.setSubscriber(this);

    std::cout << "Initializing activity server..." << std::endl;
    this->activity = new ActivityService(basePaths);

    std::cout << "Initializing dir traverser..." << std::endl;
    Im::DirectoryTraverser b(include_folders);

    std::cout << "Initializing...";
    std::cout.flush();

    namecache nameCache;
    auto paths = b.flatten_dir(base);
    this->knownPaths = paths;

    std::cout << " Done!\n";

    std::map<std::string, Im::Node> outMap;
    for (auto& path : paths) {
        outMap["/" + path.name] = path;
    }
    this->mapped = outMap;
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
    std::cout << "RECEIVED A FILE EVENT " << event.type << std::endl;
    switch (event.type) {
    case CREATED:
        //        insertFile(event);
        break;
    case DELETED:
        break;
    case RENAMED:
        break;
    case UPDATED:
        updateFile(event);
        break;
    }
}

void ImDB::insertFile(const FileEvent& event)
{
    std::vector<char> buf(event.path.begin(), event.path.end());
    auto baseName = basename(&buf[0]);
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
        .type = DT_REG
    };
    this->knownPaths.push_back(pathNode);
    this->mapped[event.path] = pathNode;
    std::cout << "INSERTED" << std::endl;
}

void ImDB::updateFile(const FileEvent& event)
{
    std::vector<char> buf(event.path.begin(), event.path.end());
    auto baseName = basename(&buf[0]);
    struct stat s {
    };
    stat(event.path.c_str(), &s);
    auto x = this->knownPaths.begin();
    for (; x != this->knownPaths.end(); x++) {
        if (std::strcmp(x->path.c_str(), &buf[0]) == 0) {
            this->knownPaths.erase(x);
            break;
        }
    }
    const Node& pathNode = Node {
        .name = baseName,
        .size = s.st_size,
        .created = s.st_ctime,
        .modified = s.st_mtime,
        .accessed = s.st_atime,
        .path = event.path,
        .type = DT_REG
    };
    this->knownPaths.push_back(pathNode);
    const std::string& normalPath = normalizePath(event.path);
    this->mapped[normalPath] = pathNode;
}

std::string ImDB::normalizePath(const std::string& path)
{
    // Here we just want to strip the base
    return path.substr(this->base.length(), path.size() - this->base.length());
}
