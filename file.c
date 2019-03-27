#include "file.h"

#include "freemap.h"
#include "inode.h"
#include "utils.h"

static ssize_t mfs_file_write(struct file * filp, const char __user * buf, size_t len, loff_t * ppos)
{
    int err = 0;
    struct super_block *sb = NULL;
    struct inode *inode = NULL;
    struct mfs_inode *minode = NULL;
    size_t newlen = 0, offset = 0;

    inode = file_inode(filp);
    minode = MFS_INODE(inode);
    sb = inode->i_sb;
    offset = ( ppos ? *ppos : 0 );
    newlen = len + offset;

    if(unlikely(minode->file.size < newlen)) {

        err = mfs_alloc_freemap(sb,minode->file.size,newlen,&minode->file.data_block);
        if(unlikely(err)) {
            goto release;  }

        err = mfs_write_blockdev(sb,minode->inode_block,0,sizeof(struct mfs_inode),minode);
        if(unlikely(err)) {
            goto release;  }
    }

    err = mfs_write_blockdev(sb,minode->file.data_block,offset,newlen,buf);
    if(unlikely(err)) {
        goto release;  }

    if(ppos) {
        *ppos = *ppos + len;
    }

release:
    return err;
}

static ssize_t mfs_file_read(struct file * filp, char __user * buf, size_t len, loff_t * ppos)
{
    return -EFAULT;
}

const struct file_operations mfs_file_operations = {
	.read = mfs_file_read,
	.write = mfs_file_write,
};
