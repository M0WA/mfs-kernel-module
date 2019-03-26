#include "dir.h"

#include "fs.h"
#include "inode.h"
#include "utils.h"

#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/string.h>

static int mfs_dir_iterate(struct file *filp, struct dir_context *ctx)
{
    size_t read_size, pos;
    int err = 0;
    uint64_t *buf = NULL;
    struct super_block *sb;
    struct inode *inode = NULL;
    struct mfs_inode *p_minode = NULL, i;
    unsigned char dtype;
    size_t ctx_offset = 2;

    inode = file_inode(filp);
    p_minode = MFS_INODE(inode);
    sb = inode->i_sb;

    if( unlikely(!S_ISDIR(p_minode->mode)) ) {
        return -ENOTDIR; }

    if(unlikely(!p_minode->dir.children || !p_minode->dir.data_block)) {
        pr_err("no children: %llu: %s, data: %lu\n",ctx->pos,p_minode->name,p_minode->dir.data_block);
        return -ENOENT; }

    if(unlikely(ctx->pos == 0)) {
        //pr_err("dir_emit . %llu\n",p_minode->inode_no);
        dir_emit(ctx, ".", 1, p_minode->inode_no, DT_DIR);
        ctx->pos++;
        return 0;
    }
    
    if(unlikely(ctx->pos == 1)) {
        memset(&i,0,sizeof(struct mfs_inode));
        err = mfs_read_blockdev(sb,p_minode->parent_inode_block,0,sizeof(struct mfs_inode),&i);
        if(unlikely(err)) {
            pr_err("cannot read from blockdev while inode at block %llu\n",buf[ctx->pos-ctx_offset]);
            goto release; }
        //pr_err("dir_emit .. %llu\n",i.inode_no);
        dir_emit(ctx, "..",2, i.inode_no, DT_DIR);
        ctx->pos++;
        return 0;

        //d_instantiate_new(struct dentry *entry, struct inode *inode)
        //res = d_splice_alias(inode, entry);
	    //if (res) {
		//    dput(found);
		//    return res;
	    //}        
    }

    if(unlikely(ctx->pos < ctx_offset)) {
        pr_err("invalid ctx->pos: %llu\n",ctx->pos);
        return -ENOENT; }

    pos = ctx->pos - ctx_offset;

    //pr_err("dir iter ctx->pos: %llu\n",ctx->pos);
    if(unlikely( pos >= p_minode->dir.children)) {
        //pr_err("exit dir iter ctx->pos: %llu\n",ctx->pos);
        return 0; }

    read_size = sizeof(uint64_t) * p_minode->dir.children;
    buf = kmalloc(read_size,GFP_KERNEL);
    if(unlikely(!buf)) {
        pr_err("oom while iterator lookup (buffer)\n");
        goto release; }

    err = mfs_read_blockdev(sb,p_minode->dir.data_block,0,read_size,buf);
    if(unlikely(err)) {
        pr_err("cannot read from blockdev while dir iter\n");
        goto release; }

    //pr_err("dir iter children: %llu\n",p_minode->dir.children);
    memset(&i,0,sizeof(struct mfs_inode));

    err = mfs_read_blockdev(sb,buf[pos],0,sizeof(struct mfs_inode),&i);
    if(unlikely(err)) {
        pr_err("cannot read from blockdev while inode at block %llu\n",buf[pos]);
        goto release; }

    dtype = DT_UNKNOWN;
    if(S_ISDIR(p_minode->mode)) {
        dtype = DT_DIR;
    } else if(S_ISREG(p_minode->mode)) {
        dtype = DT_REG;
    }

    //pr_err("emitting inode %s at block %llu\n",i.name,buf[pos]);
	dir_emit(ctx, i.name, MFS_MAX_NAME_LEN,i.inode_no, dtype);
    ctx->pos++;

/*
    for(child = 0; child < p_minode->dir.children; child++) {
        memset(&i,0,sizeof(struct mfs_inode));

//        err = mfs_read_disk_inode(sb, &newinode, buf[ctx->pos-ctx_offset]);
        err = mfs_read_blockdev(sb,buf[child],0,sizeof(struct mfs_inode),&i);
        if(unlikely(err)) {
            pr_err("cannot read from blockdev while inode at block %llu\n",buf[child]);
            goto release; }

        //pr_err("emitting inode %s at block %llu\n",i.name,buf[child]);

        dtype = DT_UNKNOWN;
        if(S_ISDIR(p_minode->mode)) {
            dtype = DT_DIR;
        } else if(S_ISREG(p_minode->mode)) {
            dtype = DT_REG;
        }

		dir_emit(ctx, i.name, MFS_MAX_NAME_LEN,i.inode_no, dtype);
        ctx->pos++;
    }
*/

release:
    if(buf) {
        kfree(buf); }
	return err;
}

const struct file_operations mfs_dir_operations = {
	.owner = THIS_MODULE,
	.iterate_shared = mfs_dir_iterate,
};
