#include "freemap.h"

#include "bitmap.h"

static struct mfs_bitmap freemap = {
    .map = NULL,
    .bits = 0,
};

int mfs_load_freemap(struct block_device *bdev,uint64_t blocks) 
{
    return mfs_load_bitmap(bdev,MFS_FREEMAP_POS,&freemap,blocks);
}

int mfs_save_freemap(struct block_device *bdev) 
{
    return 0;
}

void mfs_destroy_freemap(void) 
{
    mfs_destroy_bitmap(&freemap);
}