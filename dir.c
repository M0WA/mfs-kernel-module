#include "dir.h"

static int mfs_dir_iterate(struct file *filp, struct dir_context *ctx)
{
	return -ENOTDIR;
}

const struct file_operations mfs_dir_operations = {
	.owner = THIS_MODULE,
	.iterate = mfs_dir_iterate,
};