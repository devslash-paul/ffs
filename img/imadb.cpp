#include "imadb.h"
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <dirent.h>
#include <iostream>
#include <map>
#include <sstream>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>

using namespace Im;

// FIXME
long i = 0;

std::vector<Im::Node> Im::ImDB::listd(std::string path) {
  std::vector<Im::Node> paths;
  auto base = opendir(path.c_str());
  // Docker fails var
  if (base == NULL) {
    std::cout << "Failed to open" << path << std::endl;
    std::cout << "Unable to open dir " << strerror(errno) << std::endl;
  }
  while (auto next = readdir(base)) {
    if (next == NULL) {
      continue;
    }
    if (strcmp(next->d_name, ".") == 0 || strcmp(next->d_name, "..") == 0)
      continue;

    if (next->d_type == DT_DIR) {
      auto inner = listd(path + "/" + next->d_name);
      if (this->include_folders) {
        paths.push_back((Im::Node){.name = next->d_name,
                                   .size = 0,
                                   .path = path,
                                   .type = next->d_type});
      }
      for (auto &x : inner) {
        paths.push_back(x);
      }
    } else if (next->d_type == DT_REG) {
      i++;
      struct stat st;
      stat((path + "/" + next->d_name).c_str(), &st);
      std::stringstream ss;
      ss << i << next->d_name;
      paths.push_back((Im::Node){
          .name = ss.str(),
          .size = st.st_size,
          .path = path + "/" + next->d_name,
          .type = next->d_type,
      });
    }
  }
  if (closedir(base) != 0) {
    std::cout << "Error closing dir " << base << strerror(errno) << std::endl;
  }
  return paths;
}

ImDB::ImDB(std::string base, std::list<std::string> filters,
           int include_folders) {
  this->base = base;
  this->filters = filters;
  this->include_folders = include_folders;
  std::cout << "Initializing...";
  std::cout.flush();
  auto paths = listd(base);
  this->knownPaths = paths;
  std::cout << " Done!\n";

  std::map<std::string, int> outMap;
  int i = 0;
  for (auto &path : paths) {
    outMap["/" + path.name] = i;
    i++;
  }
  this->mapped = outMap;
}

bool hasEnding(const std::string &fullString, const std::string &ending) {
  if (fullString.length() >= ending.length()) {
    return fullString.compare(fullString.length() - ending.length(),
                              ending.length(), ending) == 0;
  } else {
    return false;
  }
}

std::vector<Im::Node> ImDB::paths_for(std::string path) {
  std::vector<Im::Node> nodes;
  std::copy_if(
      this->knownPaths.begin(), this->knownPaths.end(),
      std::back_inserter(nodes), [this](Im::Node it) {
        if (this->filters.size() == 0) {
          return true;
        }
        if (it.name.size() > 3) {
          auto end = it.name.substr(it.name.size() - 3, it.name.size());
          return std::any_of(
              this->filters.begin(), this->filters.end(),
              [it](std::string filter) { return hasEnding(it.name, filter); });
        }
        return false;
      });
  return nodes;
}

Im::Node *ImDB::node_for(std::string path) {
  if (this->mapped.count(path) > 0) {
    return &(this->knownPaths[this->mapped[path]]);
  }
  return nullptr;
}
