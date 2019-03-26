#include "inode.h"

#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/dcache.h>

#include "dir.h"
#include "file.h"
#include "freemap.h"
#include "utils.h"
#include "superblock.h"
#include "fs.h"

static int mfs_inode_create_generic(struct inode *dir, struct dentry *dentry, mode_t mode);

static int mfs_inode_mkdir(struct inode *dir, struct dentry *dentry, umode_t mode)
{
    pr_info("Creating directory %s\n", dentry->d_name.name);
    return mfs_inode_create_generic(dir, dentry, S_IFDIR | mode);
}

static struct dentry *mfs_inode_lookup(struct inode *parent_inode, struct dentry *child_dentry, unsigned int flags)
{
    size_t read_size,child;
    uint64_t *buf = NULL; 
    int err = 0; 
    struct mfs_inode *i = NULL;
    struct super_block *sb = parent_inode->i_sb;
    struct mfs_inode *p_minode = MFS_INODE(parent_inode);
    struct dentry *rc = NULL;

    pr_info("Lookup inode for %.*s",child_dentry->d_name.len,child_dentry->d_name.name);

    if(!S_ISDIR(p_minode->mode)) {
        pr_err("parent inode in lookup is not a directory\n");
        return NULL; }

    if(!p_minode->dir.children) {
        //pr_info("parent inode in lookup has no children\n");
        return NULL; }

    read_size = sizeof(uint64_t) * p_minode->dir.children;

    buf = kmalloc(read_size,GFP_KERNEL);
    if(unlikely(!buf)) {
        pr_err("oom while inode lookup (buffer) %s\n", child_dentry->d_name.name);
        goto release; }

    i = kmalloc(sizeof(struct mfs_inode),GFP_KERNEL);
    if(unlikely(!i)) {
        pr_err("oom while inode lookup (inode) %s\n", child_dentry->d_name.name);
        goto release; }

    err = mfs_read_blockdev(sb,p_minode->dir.data_block,0,read_size,buf);
    if(unlikely(err)) {
        pr_err("cannot read from blockdev while inode lookup %s\n", child_dentry->d_name.name);
        goto release; }

    for(child = 0; child < p_minode->dir.children; child++) {
        memset(i,0,sizeof(struct mfs_inode));

        err = mfs_read_blockdev(sb,buf[child],0,sizeof(struct mfs_inode),i);
        if(unlikely(err)) {
            pr_err("cannot read from blockdev while inode read %s at block %llu\n", child_dentry->d_name.name,buf[child]);
            goto release; }

        //pr_err("checking inode %s for %.*s at block %llu",(i->inode_no == MFS_INODE_NUMBER_ROOT ? "/" : i->name),child_dentry->d_name.len,child_dentry->d_name.name,buf[child]);

        if( i->name && child_dentry->d_name.len == strlen(i->name) && strncmp(child_dentry->d_name.name,i->name,child_dentry->d_name.len) == 0 ) {
            struct inode *found = 0;

            //pr_err("found inode for %.*s",child_dentry->d_name.len,child_dentry->d_name.name);

            err = mfs_read_disk_inode(sb, &found, buf[child]);
            if(unlikely(err)) {
                pr_err("cannot read inode from blockdev while inode lookup %s\n", child_dentry->d_name.name);
                goto release; }

            d_add(child_dentry, found);
/*
	        if (d_splice_alias(found, child_dentry)) {
		        dput(child_dentry);
	        }
*/
            rc = child_dentry;
            goto release;
        }
    }

release:
    if(buf) {
        kfree(buf); }
    if(err && i) {
        kfree(i); }
    return NULL;
}

static int mfs_inode_create(struct inode *dir, struct dentry *dentry, umode_t mode, bool excl)
{
    pr_info("Creating file %s\n", dentry->d_name.name);
    return mfs_inode_create_generic(dir, dentry, S_IFREG | mode);
}

const struct inode_operations mfs_inode_ops = {
	.create = mfs_inode_create,
	.lookup = mfs_inode_lookup,
	.mkdir  = mfs_inode_mkdir,
};

static int mfs_append_inode_child(struct super_block *sb,struct mfs_inode* parent,struct mfs_inode* child) {
    size_t oldsize,newsize;
    int err = 0;

    if(unlikely(!S_ISDIR(parent->mode))) {
        pr_err("can append to directories only");
        return -EINVAL; }

    oldsize = sizeof(uint64_t) * parent->dir.children;
    newsize = sizeof(uint64_t) * ++parent->dir.children;

    err = mfs_alloc_freemap(sb,oldsize,newsize,&parent->dir.data_block);
    if(unlikely(err)) {
        return err; }
    if(unlikely(!parent->dir.data_block)) {
        pr_err("cannot alloc parent data block");
        return -EIO; }

    //pr_err("writing child block: %lu, %llu",child->inode_block,parent->dir.children);
    err = mfs_write_blockdev(sb,parent->dir.data_block,sizeof(uint64_t) * (parent->dir.children-1),sizeof(uint64_t),&child->inode_block);
    if(unlikely(err)) {
        pr_err("cannot write child block to parent inode");
        return -EIO; }

    //pr_err("writing parent block: %lu, %zu",parent->inode_block,sizeof(struct mfs_inode));
    err = mfs_write_blockdev(sb,parent->inode_block,0,sizeof(struct mfs_inode),parent);
    if(unlikely(err)) {
        pr_err("cannot write parent inode");
        return -EIO; }

    return err;
}

static int mfs_inode_create_generic(struct inode *dir, struct dentry *dentry, mode_t mode) 
{
    struct mfs_inode *m_inode,*m_pinode;    
    struct inode *inode;
    struct super_block *sb;
    sector_t block_inode;
    int err = 0;

    sb = dir->i_sb;
    m_pinode = MFS_INODE(dir);

    if(unlikely( (!S_ISDIR(mode) && !S_ISREG(mode)) || S_ISLNK(mode))) {
        pr_err("could not create %s, invalid mode\n", dentry->d_name.name);
        return -EINVAL;
    }

    if(unlikely(strlen(dentry->d_name.name) >= MFS_MAX_NAME_LEN)) {
        pr_err("name %s exceeds length limits\n", dentry->d_name.name);
        return -EINVAL;
    }

    m_inode = kmalloc(sizeof(struct mfs_inode), GFP_KERNEL);
    if(unlikely(!m_inode)) {
        pr_err("mfs inode allocation failed for %s\n", dentry->d_name.name);
        return -ENOMEM;
    }

    block_inode = 0;
    err = mfs_alloc_freemap(sb,0,sizeof(struct mfs_inode),&block_inode);
    if(unlikely(err) || block_inode == 0) {
        pr_err("cannot alloc free disk space for inode of %s\n", dentry->d_name.name);
	    err = -ENOMEM;
        goto release; 
    }
    //pr_err("new inode block is %zu\n", block_inode);

    inode = new_inode(sb);
    if (unlikely(!inode)) {
        pr_err("cannot create inode for %s\n", dentry->d_name.name);
	    err = -ENOMEM;
        goto release; }
    inode->i_op = &mfs_inode_ops;
    inode->i_private = m_inode;
    inode->i_atime = inode->i_mtime = inode->i_ctime = current_time(inode);
    inode_init_owner(inode, dir, mode);

    m_inode->mode        = mode;
    m_inode->inode_no    = inode->i_ino = mfs_get_next_inode_no(sb);
    m_inode->inode_block = block_inode;
    m_inode->parent_inode_block = m_pinode->inode_block;
    m_inode->created     = m_inode->modified = inode->i_atime.tv_nsec;
    strncpy(m_inode->name,dentry->d_name.name,MFS_MAX_NAME_LEN);

    if(S_ISDIR(mode)) {
	    inode->i_fop = &mfs_dir_operations;

        m_inode->dir.children   = 0;
        m_inode->dir.data_block = 0;
    } else if (S_ISREG(mode)) {
	    inode->i_fop = &mfs_file_operations;

        m_inode->file.size       = 0;
        m_inode->file.data_block = 0;
    }

    err = mfs_write_blockdev(sb,m_inode->inode_block,0,sizeof(struct mfs_inode),m_inode);
    if(unlikely(err)) {
        pr_err("cannot write inode %s to disk\n", dentry->d_name.name);
        goto release; }

    err = mfs_append_inode_child(sb,MFS_INODE(dir),m_inode);
    if(unlikely(err)) {
        pr_err("cannot append inode %s to parent directory\n", dentry->d_name.name);
        goto release; }

release:
    if(likely(!err)) {
        err = mfs_save_sb(sb);
        d_add(dentry, inode);
    } else {
        //TODO: rollback on error
    }
    return err;
}

int mfs_read_disk_inode(struct super_block *sb, struct inode **i, sector_t block)
{
    int err;
    struct mfs_inode *m_inode;
    struct inode *tmp;

    *i = NULL;

    m_inode = kmalloc(sizeof(struct mfs_inode), GFP_KERNEL);
    if(unlikely(!m_inode)) {
        pr_err("mfs inode allocation failed\n");
        return -ENOMEM;
    }

    err = mfs_read_blockdev(sb,block,0,sizeof(struct mfs_inode),m_inode);
    if(unlikely(err != 0)) {        
        goto release;
    }

    tmp = new_inode(sb);
    if(unlikely(!tmp)) {
        pr_err("inode allocation failed\n");
        err = -ENOMEM;
        goto release;
    }

    tmp->i_ino = m_inode->inode_no;
    tmp->i_atime = current_time(tmp);
    mfs_timet_to_timespec(m_inode->modified,&tmp->i_mtime);
    mfs_timet_to_timespec(m_inode->created,&tmp->i_ctime);
    tmp->i_op = &mfs_inode_ops;
    tmp->i_fop = &mfs_dir_operations;
    tmp->i_private = m_inode;
    inode_init_owner(tmp, NULL, m_inode->mode | MFS_FSINFO(sb)->mount_opts.mode);

    *i = tmp;

release:
    if(unlikely(err)) {
        kfree(m_inode);
        //destroy inode
    }
    return 0;
}
