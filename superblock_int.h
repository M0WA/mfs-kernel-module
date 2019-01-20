#pragma once

#include <linux/fs.h>

#include "superblock.h"


int mfs_fill_sb(struct super_block *sb, void *data, int silent);