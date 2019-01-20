#pragma once

#include <linux/fs.h>

#define MFS_MAGIC_NUMBER   0x24101983
#define MFS_SUPERBLOCK_POS 0          //disk block where superblock is stored

#define MFS_DEFAULT_MODE 0755
#define MFS_DISPLAY_NAME "mfs"

extern struct file_system_type mfs_type;

struct mfs_mount_opts {
	umode_t mode;
};

struct mfs_fs_info {
	struct mfs_mount_opts mount_opts;
};