#include "activity.h"
#include "imadb.h"
#include "load_file.h"
#include <algorithm>
#include <cassert>
#include <cerrno>
#include <cstring>
#include <iostream>

#define FUSE_USE_VERSION 35
//TODO: Reintroduce naming fixup for collision files

#include <fuse.h>
#include <fuse/fuse_lowlevel.h>

std::unique_ptr<Im::DirectoryTraverser> dt;
// TODO: How the hell do i keep this updated
std::unique_ptr<Im::ActivityService> ct;
Im::ImDB* db;

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

static void* init(struct fuse_conn_info* conn)
{
    dt = std::make_unique<Im::DirectoryTraverser>(options.include_folders, Im::fsWatchStream);
    ct = std::make_unique<Im::ActivityService>(std::vector<std::string> { options.base });
    dt->flatten_dir(options.base);
    return nullptr;
}

static int getattr_callback(const char* path, struct stat* stbuf)
{
    memset(stbuf, 0, sizeof(struct stat));
    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
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
        return static_cast<int>(size);
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
            size_t pos;
            auto filter = std::string(options.filter);
            while ((pos = filter.find(delim)) != std::string::npos) {
                auto token = filter.substr(0, pos);
                tokens.push_back(token);
                filter.erase(0, pos + 1);
            }
            tokens.push_back(filter);
        }
        db = new Im::ImDB(options.base, tokens);
    }
    std::cout << "Ready. Kicking off." << std::endl;
    int ret = fuse_main(args.argc, args.argv, &operations, nullptr);
    fuse_opt_free_args(&args);
    return ret;
}
