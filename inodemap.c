#include "inodemap.h"

#include "bitmap.h"
#include "freemap.h"

#include <linux/bitops.h>

static struct mfs_bitmap inodemap = {
    .map = NULL,
    .bits = 0,
};

int mfs_load_inodemap(struct block_device *bdev,uint64_t blocks) 
{
    uint64_t long_count = BITS_TO_LONGS(blocks);
    size_t bytes = long_count * sizeof(long unsigned int);
    return mfs_load_bitmap(bdev,MFS_FREEMAP_POS + bytes,&inodemap,blocks);
}

int mfs_save_inodemap(struct block_device *bdev) 
{
    return 0;
}

void mfs_destroy_inodemap(void) 
{
    mfs_destroy_bitmap(&inodemap);
}