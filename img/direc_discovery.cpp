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
{
    this->include_folders = include_folders;
}

void DirectoryTraverser::flatten_dir(const std::string& path)
{
    this->runThread = std::thread([&]() {
        this->listd(path);
    });
}

void DirectoryTraverser::listd(const std::string& path)
{
    std::vector<Im::Node> paths;
    auto base = opendir(path.c_str());

    if (base == nullptr) {
        std::cout << "Failed to open" << path << std::endl;
        std::cout << "Unable to open dir " << strerror(errno) << std::endl;
    }

    while (auto next = readdir(base)) {
        if (strcmp(next->d_name, ".") == 0 || strcmp(next->d_name, "..") == 0)
            continue;

        if (next->d_type == DT_DIR) {
            listd(path + "/" + next->d_name);
            if (this->include_folders) {
                this->evt.accept(Im::FileEvent {
                    .type = UPDATED,
                    .nodeType = next->d_type,
                    .path = path + "/" + next->d_name });
            }
        } else if (next->d_type == DT_REG) {
            // struct stat st {
            // };
            // stat((path + "/" + next->d_name).c_str(), &st);
            auto name = findName(std::string(next->d_name));
            this->evt.accept(Im::FileEvent {
                .type = UPDATED,
                .nodeType = next->d_type,
                .path = path + "/" + next->d_name });
            // paths.push_back((Im::Node) {
            //     .name = name,
            //     .size = st.st_size,
            //     .created = st.st_ctime,
            //     .modified = st.st_mtime,
            //     .accessed = st.st_atime,
            //     .path = path + "/" + next->d_name,
            //     .type = next->d_type,
            // });
        }
    }
    if (closedir(base) != 0) {
        std::cout << "Error closing dir " << base << strerror(errno) << std::endl;
    }
}

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
