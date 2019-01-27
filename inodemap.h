#pragma once

#include <linux/fs.h>

int mfs_load_inodemap(struct block_device *bdev,uint64_t blocks);
int mfs_save_inodemap(struct block_device *bdev);
void mfs_destroy_inodemap(void);