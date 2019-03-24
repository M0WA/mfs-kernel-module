#pragma once

#include <linux/fs.h>

#define MFS_MAX_FILENAME 512

extern const struct file_operations mfs_file_operations;

struct mfs_file_record {
    char name[MFS_MAX_FILENAME];
    uint64_t size;
};
