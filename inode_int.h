#pragma once

#include <linux/types.h>
#include "inode.h"

extern const struct inode_operations mfs_inode_ops;
extern const struct file_operations mfs_dir_operations;