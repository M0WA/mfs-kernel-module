#include "freemap.h"

#include "bitmap.h"
#include "superblock.h"
#include "fs.h"

#include <linux/bitops.h>

static struct mfs_bitmap freemap = {
    .map = NULL,
    .bits = 0,
};

int mfs_load_freemap(struct super_block *sb)
{
    return mfs_load_bitmap(sb,MFS_FREEMAP_POS,&freemap,MFS_SB(sb).block_count);
}

sector_t mfs_reserve_freemap(struct super_block *sb,uint64_t bytes) 
{
    sector_t free_block;
    uint64_t blocks = DIV_ROUND_UP(bytes,sb->s_blocksize);
    free_block = bitmap_find_next_zero_area(freemap.map,freemap.bits,0,blocks,0);
    bitmap_set(freemap.map,free_block,blocks);
    mfs_save_freemap(sb);
    return free_block;    
}

int mfs_save_freemap(struct super_block *sb)
{
    return mfs_save_bitmap(sb,MFS_FREEMAP_POS,&freemap);
}

void mfs_destroy_freemap(void)
{
    mfs_destroy_bitmap(&freemap);
}