#pragma once

#include <linux/fs.h>

#include "superblock.h"

extern struct mfs_super_block mfs_sb;

int mfs_fill_sb(struct super_block *sb, void *data, int silent);