#include "freemap.h"

#include "bitmap.h"

static struct mfs_bitmap freemap = {
    .map = NULL,
    .bits = 0,
};

int mfs_load_freemap(struct super_block *sb,uint64_t blockcount)
{
    return mfs_load_bitmap(sb,MFS_FREEMAP_POS,&freemap,blockcount);
}

int mfs_save_freemap(struct super_block *sb)
{
    return 0;
}

void mfs_destroy_freemap(void)
{
    mfs_destroy_bitmap(&freemap);
}