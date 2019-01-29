#pragma once

#define MFS_MAJOR_VERSION  (uint64_t)1
#define MFS_MINOR_VERSION  (uint64_t)1
#define MFS_VERSION        ((uint64_t)( ( (MFS_MAJOR_VERSION) << 32 ) | (MFS_MINOR_VERSION) ))

#define MFS_GET_MAJOR_VERSION(x) ( ((uint64_t)((x) & 0xffffffff00000000)) >> 32 )
#define MFS_GET_MINOR_VERSION(x) ( ((uint64_t)(x)) & 0x00000000ffffffff )

#ifdef __KERNEL__

#include <linux/fs.h>

#include "superblock.h"

#define MFS_DEFAULT_MODE 0755
#define MFS_DISPLAY_NAME "mfs"

extern struct file_system_type mfs_type;

struct mfs_mount_opts {
	umode_t  mode;
};

struct mfs_fs_info {
	struct mfs_mount_opts mount_opts;
    struct mfs_super_block sb;
    uint8_t mount_id;
    int in_use;
};

#endif