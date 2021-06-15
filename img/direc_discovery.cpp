#include "direc_discovery.h"
#include "event_sync.h"
#include <cstring>
#include <dirent.h>
#include <iostream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

using namespace Im;

DirectoryTraverser::DirectoryTraverser(bool include_folders, Im::EventStream& evt)
    : evt(evt)
    , include_folders(include_folders)
{
}

void DirectoryTraverser::flatten_dir(const std::string& path)
{
    std::cout << "Flattening dir" << std::endl;
    //TODO: Could i spawn out more threads to get this done faster?
    std::thread([path, this]() {
        std::list<std::string> trials;
        std::cout << "starting with: " << path << std::endl;
        trials.push_back(path);
        do {
            auto thisPath = trials.front();
            trials.pop_front();
            auto nextPaths = list_directory(thisPath);
            trials.insert(trials.end(), nextPaths.begin(), nextPaths.end());
        } while (!trials.empty());
    }).detach();
}

std::list<std::string> DirectoryTraverser::list_directory(const std::string& path)
{
    auto base = opendir(path.c_str());
    std::list<std::string> res;

    if (base == nullptr) {
        std::cout << "Failed to open" << path << std::endl;
        std::cout << "Unable to open dir " << strerror(errno) << std::endl;
    }

    while (auto next = readdir(base)) {
        if (strcmp(next->d_name, ".") == 0 || strcmp(next->d_name, "..") == 0)
            continue;

        if (next->d_type == DT_DIR) {
            res.push_back(path + "/" + next->d_name);
            if (this->include_folders) {
                this->evt.accept(Im::FileEvent {
                    .type = UPDATED,
                    .nodeType = next->d_type,
                    .path = path + "/" + next->d_name });
            }
        } else if (next->d_type == DT_REG) {
            auto name = findName(std::string(next->d_name));
            this->evt.accept(Im::FileEvent {
                .type = UPDATED,
                .nodeType = next->d_type,
                .path = path + "/" + next->d_name });
        }
    }
    if (closedir(base) != 0) {
        std::cout << "Error closing dir " << base << strerror(errno) << std::endl;
    }
    return res;
}

//TODO: FindName isn't being used
std::string DirectoryTraverser::findName(std::string name)
{
    auto there = this->nc.find(name);
    if (there == this->nc.end()) {
        this->nc[name] = std::make_unique<std::list<std::string>>();
        this->nc[name]->push_back(name);
        return name;
    }

    // We have a clash. To keep OS's extensions happy, as well as have a semi stable ordering
    // a good solution might use the created date, or something more stable... I'm going to
    // just use the index in the list
    auto size = this->nc[name]->size();
    auto lastDot = name.find_last_of('.');
    if (lastDot == std::string::npos) {
        // No dots in the file. Oh well
        nc[name]->push_back(name + std::to_string(size));
    } else {
        nc[name]->push_back(name.substr(0, lastDot) + std::to_string(size) + name.substr(lastDot, name.size()));
    }
    return nc[name]->back();
}
DirectoryTraverser::~DirectoryTraverser() = default;