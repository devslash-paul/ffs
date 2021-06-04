#include "imadb.h"
#include "load_file.h"
#include <algorithm>
#include <assert.h>
#include <cstring>
#include <errno.h>
#include <iostream>

#define FUSE_USE_VERSION 35

#include <fuse.h>

static struct options {
    const char* base;
    const char* filter;
    int include_folders;
    int show_help;
} options;

#define OPTION(t, p)                      \
    {                                     \
        t, offsetof(struct options, p), 1 \
    }

static const struct fuse_opt option_spec[] = {
    OPTION("--ftype=%s", filter),
    OPTION("--base=%s", base),
    OPTION("-b=%s", base),
    OPTION("--include-dirs", include_folders),
    OPTION("--help", show_help),
    FUSE_OPT_END
};

extern Im::ImDB* db;

static void* init(struct fuse_conn_info* conn)
{
    return nullptr;
}

int i = 0;
static int getattr_callback(const char* path, struct stat* stbuf)
{
    memset(stbuf, 0, sizeof(struct stat));
    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        i += 10000000;
        stbuf->st_mtime = i;
        stbuf->st_nlink = 2;
    } else {
        auto node = db->node_for(path);
        if (node == nullptr) {
            return -ENOENT;
        } else if (node->type == DT_DIR) {
            stbuf->st_mode = S_IFDIR | 0755;
            stbuf->st_nlink = 1;
        } else {
            stbuf->st_mode = S_IFREG | 0755;
            stbuf->st_size = node->size;
            stbuf->st_mtime = node->modified;
            stbuf->st_atime = node->accessed;
            stbuf->st_ctime = node->created;
            stbuf->st_nlink = 1;
        }
    }
    return 0;
}

static int readdir_callback(const char* path, void* buf, fuse_fill_dir_t filler,
    off_t offset, struct fuse_file_info* info)
{
    filler(buf, ".", nullptr, 0);
    filler(buf, "..", nullptr, 0);
    for (auto& x : db->paths_for(path)) {
        filler(buf, x.name.c_str(), nullptr, 0);
        std::cout << "Adding " << x.name << std::endl;
    }
    return 0;
}

static int open_callback(const char* path, fuse_file_info* info) { return 0; }

thread_local FFS::FileBuffer fbuf;
static int read_callback(const char* path, char* buf, size_t size, off_t off,
    struct fuse_file_info* info)
{
    auto file = db->node_for(path);
    if (file != nullptr) {

        auto pFile = fbuf.open(file->path.c_str(), "rb");
        if (pFile == nullptr) {
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

    return 0;
}

static const struct fuse_operations operations = {
    .getattr = getattr_callback,
    .open = open_callback,
    .read = read_callback,
    .readdir = readdir_callback,
    .init = init,
};

static void show_help(const char* progname)
{
    printf("usage: %s [options] <mountpoint>\n\n", progname);
    printf(
        "FFS specific options\n"
        "   --base=<s>,-b=<s>  Base directory to flatten\n"
        "   --include-dirs     Allow directories to be included in the listing "
        "(all folders will be empty)\n"
        "   --ftype=<1>,<2>    File types to filter for, comma separated\n");
}

int main(int argc, char* argv[])
{
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    if (fuse_opt_parse(&args, &options, option_spec, nullptr) == -1) {
        return 1;
    }

    if (options.show_help) {
        show_help(argv[0]);
        assert(fuse_opt_add_arg(&args, "--help") == 0);
        args.argv[0][0] = '\0';
    } else {
        std::list<std::string> tokens;
        if (options.filter != nullptr) {
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
    int ret = fuse_main(args.argc, args.argv, &operations, nullptr);
    fuse_opt_free_args(&args);
    return ret;
}
