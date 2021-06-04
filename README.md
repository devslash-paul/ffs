
<img src="FFS.svg" alt="Test" width="200px"/>

## Flattened File System

FFS is a FUSE filesystem aimed to provide a readonly flattened view over an 
existing filesystem.

Mounting a directory is as simple as 

`FFS --base=/path/to/folder <mountPoint>`

Allowing for a directory structure such as:

```text
Photos/
├─ Germany Day 1/
│  ├─ 1.jpg
│  ├─ 2.jpg
│  ├─ 3.jpg
├─ Germany Day 2/
│  ├─ 4.jpg
│  ├─ 5.jpg
```

to be mounted as

```text
1.jpg
2.jpg
3.jpg
4.jpg
5.jpg
```

### Usage 

```text
usage: ./src/FFS [options] <mountpoint>

FFS specific options
   --base=<s>,-b   Base directory to flatten
   --include-dirs  Allow directories to be included in the listing (all folders will be empty)
   --ftype=<1>,<2> File types to filter for, comma separated
usage:  mountpoint [options]
```

### Duplicate Files

In the event that your flatted mount will have duplicate filenames, the subsequent
files will be displayed with different names. 

By default, the second instance of a filename will be 
mounted as `<filename>-<num>.<ext>`. Where num starts at 1 in the second file. 

Therefore you would see

* file.jpg
* file-1.jpg
* file-2.jpg

This means that any extension should remain stable. 
