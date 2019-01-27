#include "inodemap.h"

#include "bitmap.h"
#include "freemap.h"

#include <linux/bitmap.h>

static struct mfs_bitmap inodemap = {
    .map = NULL,
    .bits = 0,
};

int mfs_load_inodemap(struct super_block *sb,uint64_t blockcount) 
{
    uint64_t long_count = BITS_TO_LONGS(blockcount);
    size_t bytes = long_count * sizeof(long unsigned int);
    return mfs_load_bitmap(sb,MFS_FREEMAP_POS + bytes,&inodemap,blockcount);
}

int mfs_save_inodemap(struct super_block *sb) 
{
    return 0;
}

void mfs_destroy_inodemap(void) 
{
    mfs_destroy_bitmap(&inodemap);
}