#include <cstdio>
#include <list>
#include <unordered_map>

namespace FFS {
class FileBuffer {
    typedef typename std::list<std::pair<const char*, FILE*>>::iterator list_iterator_t;

public:
    //FileBuffer();
    FILE* open(const char* filename, const char* mode);
    void close(const char* file);

private:
    std::unordered_map<const char*, list_iterator_t> cache;
    std::list<std::pair<const char*, FILE*>> order;
};
}
