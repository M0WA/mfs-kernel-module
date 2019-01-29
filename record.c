#include "record.h"

#include <linux/slab.h>
#include <linux/string.h>

#include "utils.h"

static int mfs_read_dir_record_children(struct super_block *sb, struct mfs_record *record, uint64_t **data, uint64_t block) {
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