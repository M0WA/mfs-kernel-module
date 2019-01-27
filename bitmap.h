#pragma once

#include <linux/types.h>
#include <linux/fs.h>

struct mfs_bitmap {
    unsigned long *map;
    uint64_t bits;
};

int mfs_load_bitmap(struct block_device *bdev,uint64_t pos,struct mfs_bitmap *bitmap,uint64_t bits);
int mfs_save_bitmap(struct block_device *bdev,uint64_t pos,struct mfs_bitmap *bitmap);
void mfs_destroy_bitmap(struct mfs_bitmap *bitmap);