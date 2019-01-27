#include "bitmap.h"

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/buffer_head.h>
#include <linux/bitmap.h>
#include <linux/slab.h>
#include <linux/bitops.h>

int mfs_load_bitmap(struct block_device *bdev,uint64_t pos,struct mfs_bitmap *bitmap,uint64_t bits) 
{     
    struct buffer_head *bh;
    uint64_t long_count = BITS_TO_LONGS(bits);
    size_t bytes = long_count * sizeof(long unsigned int);
    int err = 0;

    bh = __bread_gfp(bdev, pos, bytes, __GFP_MOVABLE);
    if(unlikely(!bh)) {
        pr_err("Could not read bitmap for mfs\n");
        return -EINVAL;
    } else {
        pr_debug("Read bitmap for mfs\n");
    }

    bitmap->map = kmalloc(bytes, GFP_KERNEL);
    if(!bitmap->map) {
        err = -ENOMEM;
        goto release;
    }
    bitmap->bits = long_count;   
    bitmap_copy(bitmap->map,(const long unsigned int*)bh->b_data,bits);

release:
    __brelse(bh);
    return err;
}

int mfs_save_bitmap(struct block_device *bdev,uint64_t pos,struct mfs_bitmap *bitmap) 
{
    return 0;
}

void mfs_destroy_bitmap(struct mfs_bitmap *bitmap)
{
    if(bitmap->map) {
        kfree(bitmap->map);
        bitmap->map = NULL;
        bitmap->bits = 0;
    }
}