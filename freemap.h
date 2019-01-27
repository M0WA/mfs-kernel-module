#pragma once

#include <linux/fs.h>

#include "superblock_int.h"

#define MFS_FREEMAP_POS MFS_SUPERBLOCK_POS + MFS_SUPERBLOCK_SIZE

int mfs_load_freemap(struct super_block *sb,uint64_t blockcount);
int mfs_save_freemap(struct super_block *sb);
void mfs_destroy_freemap(void);