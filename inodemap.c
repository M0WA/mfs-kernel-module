#include "inodemap.h"

#include "superblock.h"
#include "bitmap.h"
#include "freemap.h"
#include "fs.h"

#include <linux/bitmap.h>

static struct mfs_bitmap inodemap = {
    .map = NULL,
    .bits = 0,
};

int mfs_load_inodemap(struct super_block *sb) 
{
    return mfs_load_bitmap(sb,MFS_SB(sb).inodemap_block,&inodemap,MFS_SB(sb).block_count);
}

int mfs_save_inodemap(struct super_block *sb) 
{
    return 0;
}

void mfs_destroy_inodemap(void) 
{
    mfs_destroy_bitmap(&inodemap);
}