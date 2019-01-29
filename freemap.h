#pragma once

#include <linux/fs.h>

sector_t mfs_reserve_freemap(struct super_block *sb,uint64_t bytes);
int mfs_load_freemap(struct super_block *sb);
int mfs_save_freemap(struct super_block *sb);
void mfs_destroy_freemap(void);