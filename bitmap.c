#include "bitmap.h"

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/buffer_head.h>

int mfs_load_bitmap(struct block_device *bdev,uint64_t pos,unsigned long *bitmap,uint64_t size) 
{
    struct buffer_head *bh = __bread_gfp(bdev, pos, size, __GFP_MOVABLE);
    if(unlikely(!bh)) {
        pr_err("Could not read bitmap for mfs\n");
        return -EINVAL;
    } else {
        pr_debug("Read bitmap for mfs\n");
    }

    memcpy(bitmap,bh->b_data,size);
    return 0;
}

int mfs_save_bitmap(struct block_device *bdev,uint64_t pos,unsigned long *bitmap,uint64_t size) 
{
    return 0;
}