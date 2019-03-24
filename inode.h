#pragma once

#ifdef __KERNEL__
#include <linux/fs.h>
#include <linux/types.h>

extern const struct inode_operations mfs_inode_ops;

int mfs_read_disk_inode(struct super_block *sb, struct inode **i, sector_t block);

#define MFS_INODE(x) ((struct mfs_inode*)(x)->i_private)
#else
#include <stdint.h>
#endif

#define MFS_INODE_NUMBER_ROOT 0
#define MFS_MAX_NAME_LEN      255

struct mfs_file_inode {
    size_t size;
    sector_t data_block;
};

struct mfs_dir_inode {
    uint64_t children;
    sector_t data_block;
};

struct mfs_inode {
    char name[MFS_MAX_NAME_LEN];
	mode_t mode;
	uint64_t inode_no;
    uint64_t created;
    uint64_t modified;
    sector_t inode_block;
    union {
        struct mfs_file_inode file;
        struct mfs_dir_inode  dir;
    };
};
