#include "dir.h"

#include "inode.h"
#include "record.h"

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

int mfs_append_child_dir(struct super_block *sb, struct inode *dir,uint64_t childblock)
{
    int err;
    struct mfs_record record;
    uint64_t *children = NULL, *tmpchild = NULL;   
    struct mfs_inode *p_dir = MFS_INODE(dir);

    err = mfs_read_disk_record(sb, &record, &children, p_dir->record_block);
    if(err != 0) {
        goto release;
    }

    tmpchild = krealloc(children, record.dir.children_inodes_count + 1, GFP_KERNEL);
    if(!tmpchild) {
        err = -ENOMEM;
        goto release;
    }
    children = tmpchild;
    children[record.dir.children_inodes_count] = childblock;
    record.dir.children_inodes_count++;

    //TODO: write back record + children

release:
    if(children) {
        kfree(children);
    }
    return 0;
}