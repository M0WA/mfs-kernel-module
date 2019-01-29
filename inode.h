#pragma once

#ifdef __KERNEL__
#include <linux/fs.h>
#include <linux/types.h>

extern const struct inode_operations mfs_inode_ops;

int mfs_read_disk_inode(struct super_block *sb, struct inode **i, uint64_t block);
#else
#include <stdint.h>
#endif

#define MFS_INODE_NUMBER_ROOT 0
#define MFS_MAX_FILENAME      512

struct mfs_inode {
	mode_t mode;
	uint64_t inode_no;
    uint64_t created;
    uint64_t modified;
    char     name[MFS_MAX_FILENAME];
	union {
		uint64_t file_size;
        union {
            uint64_t dir_children_count;
            uint64_t dir_children[0];
        };
	};
};