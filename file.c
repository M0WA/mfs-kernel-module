#include "file.h"

static ssize_t mfs_file_write(struct file * filp, const char __user * buf, size_t len, loff_t * ppos)
{
    return -EFAULT;
}

static ssize_t mfs_file_read(struct file * filp, char __user * buf, size_t len, loff_t * ppos)
{
    return -EFAULT;
}

const struct file_operations mfs_file_operations = {
	.read = mfs_file_read,
	.write = mfs_file_write,
};
