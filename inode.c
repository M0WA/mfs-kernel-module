#include "inode_int.h"

#include <linux/fs.h>

#include "dir.h"
#include "file.h"
#include "freemap.h"

static int mfs_inode_create_generic(struct inode *dir, struct dentry *dentry, umode_t mode);

static int mfs_inode_mkdir(struct inode *dir, struct dentry *dentry, umode_t mode)
{
    pr_info("creating directory %s\n", dentry->d_name.name);
    return mfs_inode_create_generic(dir, dentry, mode);
}

static struct dentry *mfs_inode_lookup(struct inode *parent_inode, struct dentry *child_dentry, unsigned int flags)
{
    return NULL;
}

static int mfs_inode_create(struct inode *dir, struct dentry *dentry, umode_t mode, bool excl)
{
    pr_info("creating file %s\n", dentry->d_name.name);
    return -EPERM;
}

const struct inode_operations mfs_inode_ops = {
	.create = mfs_inode_create,
	.lookup = mfs_inode_lookup,
	.mkdir  = mfs_inode_mkdir,
};

static int mfs_inode_create_generic(struct inode *dir, struct dentry *dentry, umode_t mode) 
{
    struct inode *inode;
    struct super_block *sb;
    int err = 0;
    uint64_t ino;

    if (!S_ISDIR(mode) && !S_ISREG(mode)) {
        pr_err("could not create %s, invalid mode\n", dentry->d_name.name);
        return -EINVAL;
    }

    sb = dir->i_sb;
    ino = mfs_reserve_freemap(sb,sizeof(struct mfs_inode));

	inode = new_inode(sb);
	if (!inode) {
		err = ENOMEM;
        goto release;
	}

	inode->i_sb = sb;
	inode->i_op = &mfs_inode_ops;
	inode->i_atime = inode->i_mtime = inode->i_ctime = current_time(inode);
	inode->i_ino = ino;
	if (S_ISDIR(mode)) {
		inode->i_fop = &mfs_dir_operations;
	} else if (S_ISREG(mode)) {
		inode->i_fop = &mfs_file_operations;
	}

release:
    if(err && ino) {
        //unreserve free blocks
        //unreserve indoe
    }
    return err;
}