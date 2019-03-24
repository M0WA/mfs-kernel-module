#include "dir.h"

#include "inode.h"

#include <linux/slab.h>
#include <linux/string.h>

static int mfs_dir_iterate(struct file *filp, struct dir_context *ctx)
{
    struct inode *inode = file_inode(filp);
    struct mfs_inode *m_inode = MFS_INODE(inode);

    if( unlikely(!S_ISDIR(m_inode->mode)) ) {
        return -ENOTDIR;
    }
    if( unlikely(dir_emit_dots(filp, ctx)) ) {
        return 0;
    }
	return 0;
}

const struct file_operations mfs_dir_operations = {
	.owner = THIS_MODULE,
	.iterate = mfs_dir_iterate,
};
