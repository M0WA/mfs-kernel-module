#pragma once

#include <linux/fs.h>

#define MFS_MAX_DIRNAME 512

extern const struct file_operations mfs_dir_operations;

struct mfs_dir_record {
    char     name[MFS_MAX_DIRNAME];
    size_t   children_inodes_count;
    uint64_t children_inode_blocks[0];
};
