#include "load_file.h"
#include <cstdio>
#include <cstring>
#include <errno.h>
#include <iostream>

using namespace FFS;

const int LOAD_FILE_MAX = 30;

//TODO
// Make it so you can specify the naming pattern when duplicates occur
// General duplication strategy
// Some way to get the original file location

// So what i do is have the map that points to the index
FILE* FileBuffer::open(const char* filename, const char* mode)
{
    auto it = this->cache.find(filename);

    // If we don't have it, lets open the file and put it in
    if (it == this->cache.end()) {
        // Open up the file,
        auto file = fopen(filename, mode);
        // Put it in the order list
        this->order.push_front(std::make_pair(filename, file));
        // Add it to the map
        this->cache.insert({ filename, this->order.begin() });

        // Then we should remove some
        if (this->cache.size() > LOAD_FILE_MAX) {
            auto back = this->order.back();
            if (fclose(back.second) != 0) {
                std::cout << "Failed: " << strerror(errno) << std::endl;
            }
            this->cache.erase(back.first);
            this->order.pop_back();
        }
    } else {
        this->order.splice(this->order.begin(), this->order, it->second);
    }

    it = this->cache.find(filename);
    // It is the
    auto value = it->second->second;
    return value;
}

void FileBuffer::close(const char* file)
{
    // Only close it if it's not in the list right now
    auto found = this->cache.find(file);
    if (found == this->cache.end()) {
        fclose(found->second->second);
    }
}
