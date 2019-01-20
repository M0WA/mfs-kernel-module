#include "inode_int.h"

#include <linux/version.h>
#include <linux/fs.h>

static int mfs_create(struct inode *dir, struct dentry *dentry, umode_t mode, bool excl);
static struct dentry *mfs_lookup(struct inode *parent_inode, struct dentry *child_dentry, unsigned int flags);
static int mfs_mkdir(struct inode *dir, struct dentry *dentry, umode_t mode);

static int mfs_mkdir(struct inode *dir, struct dentry *dentry, umode_t mode)
{
    pr_info("creating directory %s\n", dentry->d_name.name);
    return -EPERM;
}

static struct dentry *mfs_lookup(struct inode *parent_inode, struct dentry *child_dentry, unsigned int flags)
{
    return NULL;
}

static int mfs_create(struct inode *dir, struct dentry *dentry, umode_t mode, bool excl)
{
    pr_info("creating file %s\n", dentry->d_name.name);
    return -EPERM;
}

const struct inode_operations mfs_inode_ops = {
	.create = mfs_create,
	.lookup = mfs_lookup,
	.mkdir  = mfs_mkdir,
};

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0)
static int mfs_iterate(struct file *filp, struct dir_context *ctx)
#else
static int mfs_readdir(struct file *filp, void *dirent, filldir_t filldir)
#endif
{
	return -ENOTDIR;
}

const struct file_operations mfs_dir_operations = {
	.owner = THIS_MODULE,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0)
	.iterate = mfs_iterate,
#else
	.readdir = mfs_readdir,
#endif
};