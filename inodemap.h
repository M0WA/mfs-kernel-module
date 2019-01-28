#pragma once

#include <linux/fs.h>

int mfs_load_inodemap(struct super_block *sb);
int mfs_save_inodemap(struct super_block *sb);
void mfs_destroy_inodemap(void);