#include "dir.h"

#include "fs.h"
#include "inode.h"
#include "utils.h"

#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/string.h>

static int mfs_dir_iterate(struct file *filp, struct dir_context *ctx)
{
    size_t read_size,child;
    int err = 0;
    uint64_t *buf = NULL;
    struct super_block *sb;
    struct inode *inode;
    struct mfs_inode *p_minode, i;
    unsigned char dtype;

    inode = file_inode(filp);
    p_minode = MFS_INODE(inode);
    sb = inode->i_sb;

    if( unlikely(!S_ISDIR(p_minode->mode)) ) {
        return -ENOTDIR;
    }

    if( unlikely(dir_emit_dots(filp, ctx)) ) {
        return 0;
    }

    read_size = sizeof(uint64_t) * p_minode->dir.children;
    buf = kmalloc(read_size,GFP_KERNEL);
    if(unlikely(!buf)) {
        pr_err("oom while iterator lookup (buffer)\n");
        goto release; }

    err = mfs_read_blockdev(sb,p_minode->dir.data_block,0,read_size,buf);
    if(unlikely(err)) {
        pr_err("cannot read from blockdev while dir iter\n");
        goto release; }

    for(child = 0; child < p_minode->dir.children; child++) {
        memset(&i,0,sizeof(struct mfs_inode));

        err = mfs_read_blockdev(sb,buf[child],0,sizeof(struct mfs_inode),&i);
        if(unlikely(err)) {
            pr_err("cannot read from blockdev while inode at block %llu\n",buf[child]);
            goto release; }

        pr_info("emitting inode %s at block %llu",(i.inode_no == MFS_INODE_NUMBER_ROOT ? "/" : i.name),buf[child]);

        dtype = DT_UNKNOWN;
        if(S_ISDIR(p_minode->mode)) {
            dtype = DT_DIR;
        } else if(S_ISREG(p_minode->mode)) {
            dtype = DT_REG;
        }

		dir_emit(ctx, i.name, MFS_MAX_NAME_LEN,i.inode_no, dtype);
        ctx->pos++;
    }

release:
    if(buf) {
        kfree(buf); }
	return err;
}

const struct file_operations mfs_dir_operations = {
	.owner = THIS_MODULE,
	.iterate = mfs_dir_iterate,
};
