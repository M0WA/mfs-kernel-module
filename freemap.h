#pragma once

#include <linux/fs.h>

#include "superblock_int.h"

#define MFS_FREEMAP_POS MFS_SUPERBLOCK_POS + MFS_SUPERBLOCK_SIZE

int mfs_load_freemap(struct block_device *bdev,uint64_t blocks);
int mfs_save_freemap(struct block_device *bdev);
void mfs_destroy_freemap(void);