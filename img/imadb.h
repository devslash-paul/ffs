#include <dirent.h>
#include <list>
#include <map>
#include <string>
#include <sys/types.h>
#include <vector>

namespace Im {

struct Node {
  std::string name;
  off_t size;
  std::string path;
  unsigned char type;
};

class ImDB {
public:
  ImDB(std::string, std::list<std::string>, int);
  std::vector<Im::Node> paths_for(std::string);
  Im::Node *node_for(std::string);

private:
  std::vector<Im::Node> listd(std::string);
  std::string base;
  std::list<std::string> filters;
  std::vector<Im::Node> knownPaths;
  int include_folders;
  std::map<std::string, int> mapped;
};
}; // namespace Im
