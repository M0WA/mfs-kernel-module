#include "freemap.h"

#include "bitmap.h"
#include "superblock_int.h"

#include <linux/bitops.h>

static struct mfs_bitmap freemap = {
    .map = NULL,
    .bits = 0,
};

int mfs_load_freemap(struct super_block *sb)
{
    return mfs_load_bitmap(sb,MFS_FREEMAP_POS,&freemap,mfs_sb.block_count);
}

uint64_t mfs_reserve_freemap(struct super_block *sb,uint64_t bytes) 
{
    uint64_t pos;
    uint64_t blocks = bytes/sb->s_blocksize;
    if( (bytes%mfs_sb.block_size) != 0 ) {
        blocks++; }

    pos = bitmap_find_next_zero_area(freemap.map,freemap.bits,0,blocks,0);
    bitmap_set(freemap.map,pos,blocks);
    mfs_save_freemap(sb);
    return pos;    
}

int mfs_save_freemap(struct super_block *sb)
{
    return mfs_save_bitmap(sb,MFS_FREEMAP_POS,&freemap);
}

void mfs_destroy_freemap(void)
{
    mfs_destroy_bitmap(&freemap);
}