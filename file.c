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

	pr_err("Writing to file %s (%zu bytes, offset: %llu)\n",minode->name,len,(ppos ? *ppos : 0));

    if(unlikely(minode->file.size < newlen)) {

		//pr_err("realloc %zu bytes at data block %lu\n",newlen,minode->file.data_block);

        err = mfs_alloc_freemap(sb,minode->file.size,newlen,&minode->file.data_block);
        if(unlikely(err)) {
            goto release;  }

		//pr_err("realloced %zu bytes at data block %lu\n",newlen,minode->file.data_block);

        err = mfs_write_blockdev(sb,minode->inode_block,0,sizeof(struct mfs_inode),minode);
        if(unlikely(err)) {
            goto release;  }
    }

    err = mfs_write_blockdev(sb,minode->file.data_block,offset,newlen,buf);
    if(unlikely(err)) {
        goto release;  }

	inode->i_size = minode->file.size = newlen;	

    err = mfs_write_blockdev(sb,minode->inode_block,0,sizeof(struct mfs_inode),minode);
    if(unlikely(err)) {
        goto release;  }	

    if(ppos) {
        *ppos = *ppos + len;
    }

release:
	if(err) {
		return -err;
	} else {
		return len;
	}
}

static ssize_t mfs_file_read(struct file * filp, char __user * buf, size_t len, loff_t * ppos)
{
    int err = 0;
    struct super_block *sb = NULL;
    struct inode *inode = NULL;
    struct mfs_inode *minode = NULL;
    size_t offset = 0,read_len = 0;

    inode = file_inode(filp);
    minode = MFS_INODE(inode);
    sb = inode->i_sb;
    offset = ( ppos ? *ppos : 0 );
	read_len = min(minode->file.size-offset,len);

	if(offset >= minode->file.size) {
		return 0; }

	pr_err("Reading from file %s (size: %zu bytes, read: %zu bytes, offset: %llu, block: %lu)\n",minode->name,minode->file.size,read_len,(ppos ? *ppos : 0),minode->file.data_block);

	err = mfs_read_blockdev(sb,minode->file.data_block,offset,read_len,buf);
	if( unlikely(err) ) {
		return -err;
	} else {
		*ppos += read_len;
		return read_len;
	}
}

const struct file_operations mfs_file_operations = {
	.read = mfs_file_read,
	.write = mfs_file_write,
};
