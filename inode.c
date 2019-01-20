#include "inode_int.h"

#include <linux/fs.h>

static int mfs_create(struct inode *dir, struct dentry *dentry, umode_t mode, bool excl);
static struct dentry *mfs_lookup(struct inode *parent_inode, struct dentry *child_dentry, unsigned int flags);
static int mfs_mkdir(struct inode *dir, struct dentry *dentry, umode_t mode);

struct inode_operations mfs_inode_ops = {
	.create = mfs_create,
	.lookup = mfs_lookup,
	.mkdir  = mfs_mkdir,
};

static int mfs_mkdir(struct inode *dir, struct dentry *dentry, umode_t mode)
{
    return -EPERM;
}

static struct dentry *mfs_lookup(struct inode *parent_inode, struct dentry *child_dentry, unsigned int flags)
{
    return NULL;
}

static int mfs_create(struct inode *dir, struct dentry *dentry, umode_t mode, bool excl)
{
    return -EPERM;
}