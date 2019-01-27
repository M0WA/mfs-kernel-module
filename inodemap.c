#include "inodemap.h"

#include <linux/slab.h>

#include "bitmap.h"
#include "freemap.h"

static struct mfs_bitmap inodemap = {
    .map = NULL,
    .bits = 0,
};

int mfs_load_inodemap(struct block_device *bdev,uint64_t blocks) 
{
    uint64_t long_count = BITS_TO_LONGS(blocks);
    size_t bytes = long_count * sizeof(long);

    inodemap.map = kmalloc(bytes, GFP_KERNEL);
    if(!inodemap.map) {
        return -ENOMEM;
    }
    inodemap.bits = long_count;
    return mfs_load_bitmap(bdev,MFS_FREEMAP_POS + bytes,inodemap.map,bytes);
}

int mfs_save_inodemap(struct block_device *bdev) 
{
    return 0;
}

void mfs_destroy_inodemap(void) 
{
    if(inodemap.map) {
        kfree(inodemap.map);
        inodemap.map = NULL;
        inodemap.bits = 0;
    }
}