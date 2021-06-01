#include "imadb.h"
#include "load_file.h"
#include <algorithm>
#include <assert.h>
#include <cstring>
#include <errno.h>
#include <fstream>
#include <iostream>

#define FUSE_USE_VERSION 35

#include <fuse.h>

static struct options {
  const char *base;
  const char *filter;
  int include_folders;
  int show_help;
} options;

#define OPTION(t, p)                                                           \
  { t, offsetof(struct options, p), 1 }

static const struct fuse_opt option_spec[] = {
    OPTION("--ftype=%s", filter), OPTION("--base=%s", base),
    OPTION("--include-dirs", include_folders),
    OPTION("--help", show_help), OPTION("-b=%s", base), FUSE_OPT_END};

const char *contents = strdup("HI\n");
static Im::ImDB *db = NULL;

static void *hello_init(struct fuse_conn_info *conn) {
  (void)conn;
  return NULL;
}

static int getattr_callback(const char *path, struct stat *stbuf) {
  memset(stbuf, 0, sizeof(struct stat));
  if (strcmp(path, "/") == 0) {
    stbuf->st_mode = S_IFDIR | 0755;
    stbuf->st_nlink = 2;
  } else {
    auto node = db->node_for(path);
    if (node == NULL) {
      return -ENOENT;
    } else if (node->type == DT_DIR) {
      stbuf->st_mode = S_IFDIR | 0755;
      stbuf->st_nlink = 1;
    } else {
      stbuf->st_mode = S_IFREG | 0755;
      stbuf->st_size = node->size;
      stbuf->st_nlink = 1;
    }
  }
  return 0;
}

static int readdir_callback(const char *path, void *buf, fuse_fill_dir_t filler,
                            off_t offset, struct fuse_file_info *info) {
  filler(buf, ".", NULL, 0);
  filler(buf, "..", NULL, 0);
  int i = 0;
  for (auto &x : db->paths_for(path)) {
    i++;
    filler(buf, x.name.c_str(), NULL, 0);
  }
  return 0;
}

static int open_callback(const char *path, fuse_file_info *info) { return 0; }

thread_local FFS::FileBuffer fbuf;
static int read_callback(const char *path, char *buf, size_t size, off_t off,
                         struct fuse_file_info *info) {
  memset(buf, 0, sizeof buf);
  auto file = db->node_for(path);
  if (file != nullptr) {

    auto pFile = fbuf.open(file->path.c_str(), "rb");
    // auto pFile = fopen(file->path.c_str(), "rb");
    if (pFile == NULL) {
      printf("Error opening file unexist.ent: %s\n", strerror(errno));
      return -ENOENT;
    }
    fseek(pFile, 0, SEEK_END);
    auto end = ftell(pFile);
    if (end - off < size) {
      size = end - off;
    }
    fseek(pFile, off, SEEK_SET);
    fread(buf, size, 1, pFile);
    fbuf.close(file->path.c_str());
    return size;
  }

  if (off > 0) {
    return 0;
  }
  memcpy(buf, contents, 3);
  return 3;
}

static const struct fuse_operations hello_oper = {
    .getattr = getattr_callback,
    .open = open_callback,
    .read = read_callback,
    .readdir = readdir_callback,
    .init = hello_init,
};

static void show_help(const char *progname) {
  printf("usage: %s [options] <mountpoint>\n\n", progname);
  printf("FFS specific options\n"
         "   --base=<s>      Base directory to flatten\n");
}

int main(int argc, char *argv[]) {
  struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
  if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1) {
    return 1;
  }

  if (options.show_help) {
    show_help(argv[0]);
    assert(fuse_opt_add_arg(&args, "--help") == 0);
    args.argv[0][0] = '\0';
  } else {
    std::list<std::string> tokens;
    if (options.filter != NULL) {
      auto delim = ",";
      size_t pos = 0;
      auto filter = std::string(options.filter);
      while ((pos = filter.find(delim)) != std::string::npos) {
        auto token = filter.substr(0, pos);
        tokens.push_back(token);
        filter.erase(0, pos + 1);
      }
      tokens.push_back(filter);
    }
    db = new Im::ImDB(options.base, tokens, options.include_folders);
  }

  int ret = fuse_main(args.argc, args.argv, &hello_oper, NULL);
  fuse_opt_free_args(&args);
  return ret;
}