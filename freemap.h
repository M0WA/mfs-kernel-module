#pragma once

#include <linux/fs.h>

struct mfs_bitmap;

int mfs_alloc_freemap(struct super_block *sb,size_t old_bytes,size_t new_bytes,sector_t* block);
