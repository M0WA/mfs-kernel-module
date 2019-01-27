#include "freemap.h"

#include "bitmap.h"

#include <linux/slab.h>
#include <linux/bitops.h>

static struct mfs_bitmap freemap = {
    .map = NULL,
    .bits = 0,
};

int mfs_load_freemap(struct block_device *bdev,uint64_t blocks) 
{    
    uint64_t long_count = BITS_TO_LONGS(blocks);
    size_t bytes = long_count * sizeof(long);

    freemap.map = kmalloc(bytes, GFP_KERNEL);
    if(!freemap.map) {
        return -ENOMEM;
    }
    freemap.bits = long_count;
    return mfs_load_bitmap(bdev,MFS_FREEMAP_POS,freemap.map,bytes);
}

int mfs_save_freemap(struct block_device *bdev) 
{
    return 0;
}

void mfs_destroy_freemap(void) 
{
    if(freemap.map) {
        kfree(freemap.map);
        freemap.map = NULL;
        freemap.bits = 0;
    }
}