#include "fs_int.h"

#include "mount.h"

struct file_system_type mfs_type = {
    .owner = THIS_MODULE,
    .name = MFS_DISPLAY_NAME,
    .mount = mfs_mount,
    .kill_sb = kill_block_super,
    .fs_flags = FS_REQUIRES_DEV,
    .next = NULL,
};