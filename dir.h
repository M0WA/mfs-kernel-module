#pragma once

#include <linux/fs.h>

#define MFS_MAX_DIRNAME 512

extern const struct file_operations mfs_dir_operations;

struct mfs_dir_record {
    char     name[MFS_MAX_DIRNAME];
    size_t   children_inodes_count;
};

struct inode;
struct super_block;

int mfs_append_child_dir(struct super_block *sb, struct inode *dir,uint64_t childblock);
