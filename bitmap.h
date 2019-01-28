#pragma once

#include <linux/types.h>
#include <linux/fs.h>

struct mfs_bitmap {
    unsigned long *map;
    uint64_t bits;
};

void mfs_set_bit_bitmap(struct mfs_bitmap *bitmap,uint64_t bit,int set);
int mfs_load_bitmap(struct super_block *sb,uint64_t pos,struct mfs_bitmap *bitmap,uint64_t bits);
int mfs_save_bitmap(struct super_block *sb,uint64_t pos,struct mfs_bitmap *bitmap);
void mfs_destroy_bitmap(struct mfs_bitmap *bitmap);