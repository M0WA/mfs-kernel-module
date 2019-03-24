#include "record.h"

#include <linux/slab.h>
#include <linux/string.h>

#include "utils.h"

static int mfs_read_dir_record_children(struct super_block *sb, struct mfs_record *record, uint64_t **data, uint64_t block) 
{
    int err;
    size_t offset      = sizeof(struct mfs_record) % sb->s_blocksize;
    size_t recordblock = sizeof(struct mfs_record) / sb->s_blocksize;
    size_t datasize    = sizeof(uint64_t) * record->dir.children_inodes_count;

    if(!data) {
        return 0;
    }
    *data = kmalloc(datasize, GFP_KERNEL);
    if(!*data) {
        return -ENOMEM;
    }

    err = mfs_read_blockdev(sb,recordblock,offset,datasize,*data);
    if(err != 0 && *data) {
        kfree(*data);
    }
    return err;
}

int mfs_read_disk_record(struct super_block *sb, struct mfs_record *record, uint64_t **data, uint64_t block)
{
    int err;
    err = mfs_read_blockdev(sb,block,0,sizeof(struct mfs_record),record);
    if(err != 0) {
        return err;}

    switch(record->type) {
    case MFS_DIR_RECORD:
        return mfs_read_dir_record_children(sb, record, data, block);
    default:
        break;
    }
    return 0;
}

int mfs_create_record(const struct dentry *dentry, mode_t mode, struct mfs_record *record)
{
    if (S_ISDIR(mode)) {
        record->type = MFS_FILE_RECORD;
        strncpy(record->file.name,dentry->d_name.name,MFS_MAX_FILENAME);
        record->file.size = 0;
    } else if (S_ISREG(mode)) {
        record->type = MFS_DIR_RECORD;
        strncpy(record->dir.name,dentry->d_name.name,MFS_MAX_DIRNAME);
        record->dir.children_inodes_count = 0;
    } else {
        return -EINVAL;
    }
    return 0;
}
