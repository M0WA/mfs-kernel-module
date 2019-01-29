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
#include "record.h"

static int mfs_inode_create_generic(struct inode *dir, struct dentry *dentry, mode_t mode);

static int mfs_inode_mkdir(struct inode *dir, struct dentry *dentry, umode_t mode)
{
    pr_info("Creating directory %s\n", dentry->d_name.name);
    return mfs_inode_create_generic(dir, dentry, S_IFDIR | mode);
}

static struct dentry *mfs_inode_lookup(struct inode *parent_inode, struct dentry *child_dentry, unsigned int flags)
{
    int err;
    size_t child;
    struct mfs_record record;
    uint64_t *children = NULL;    
    struct inode *i = NULL;
    char *childname = NULL;
    struct dentry *rc = NULL;
    struct super_block *sb = parent_inode->i_sb;
    struct mfs_inode *p_minode = MFS_INODE(parent_inode);

    pr_info("Lookup inode for %.*s",child_dentry->d_name.len,child_dentry->d_name.name);

    err = mfs_read_disk_record(sb, &record, &children, p_minode->record_block);
    if(err != 0) {        
        goto release;
    }

    for(child = 0; child < record.dir.children_inodes_count; child++) {
        err = mfs_read_disk_inode(sb, &i, children[child]);
        err = mfs_read_disk_record(sb, &record, NULL, MFS_INODE(i)->record_block);

        switch(record.type) {
        case MFS_DIR_RECORD:
            childname = record.dir.name;
            break;
        case MFS_FILE_RECORD:
            childname = record.file.name;
            break;
        default:
            continue;
        }

        if( strncmp(child_dentry->d_name.name,childname,child_dentry->d_name.len) == 0 ) {
            pr_info("Found inode for %.*s",child_dentry->d_name.len,child_dentry->d_name.name);
            rc = d_splice_alias(i,child_dentry);
            break;
        }

        kfree(i);
        i = NULL;
    }
    
release:
    if(children) {
        kfree(children); }
    return rc;
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

static int mfs_inode_create_generic(struct inode *dir, struct dentry *dentry, mode_t mode) 
{
    struct mfs_inode *m_inode;
    struct inode *inode;
    struct super_block *sb;
    int err = 0;
    sector_t block_inode, block_record;

    if (!S_ISDIR(mode) && !S_ISREG(mode)) {
        pr_err("could not create %s, invalid mode\n", dentry->d_name.name);
        return -EINVAL;
    }

    m_inode = kmalloc(sizeof(struct mfs_inode), GFP_KERNEL);
    if (unlikely(!m_inode)) {
        pr_err("mfs inode allocation failed\n");
        return -ENOMEM;
    }

    sb = dir->i_sb;
    block_inode  = mfs_reserve_freemap(sb,sizeof(struct mfs_inode));
    block_record = mfs_reserve_freemap(sb,sizeof(struct mfs_record));
    mfs_save_freemap(sb);

	inode = new_inode(sb);
	if (!inode) {
		err = ENOMEM;
        goto release; }
	//inode->i_sb = sb;
	inode->i_op = &mfs_inode_ops;
    inode->i_private = m_inode;
	if (S_ISDIR(mode)) {
		inode->i_fop = &mfs_dir_operations;
	} else if (S_ISREG(mode)) {
		inode->i_fop = &mfs_file_operations;
	}
    inode->i_atime = inode->i_mtime = inode->i_ctime = current_time(inode);

    m_inode->inode_no = inode->i_ino = mfs_get_next_inode_no(sb);
    m_inode->inode_block  = block_inode;
    m_inode->record_block = block_record;
    m_inode->mode = mode;
    m_inode->created = m_inode->modified = inode->i_atime.tv_nsec;

release:
    if(err && m_inode->inode_no) {
        //unreserve free blocks
    }
    return err;
}

int mfs_read_disk_inode(struct super_block *sb, struct inode **i, uint64_t block)
{
    int err;
    struct mfs_inode *m_inode;
    struct inode *tmp;

    *i = NULL;

    m_inode = kmalloc(sizeof(struct mfs_inode), GFP_KERNEL);
    if (unlikely(!m_inode)) {
        pr_err("mfs inode allocation failed\n");
        return -ENOMEM;
    }

    err = mfs_read_blockdev(sb,block,0,sizeof(struct mfs_inode),m_inode);
    if(unlikely(err != 0)) {        
        goto release;
    }

    tmp = new_inode(sb);
    if (unlikely(!tmp)) {
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
    if(err != 0) {
        kfree(m_inode);
        //destroy inode
    }
    return 0;
}