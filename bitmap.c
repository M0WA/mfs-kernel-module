#include "bitmap.h"

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/bitmap.h>
#include <linux/slab.h>
#include <linux/bitops.h>

#include "superblock.h"

int mfs_load_bitmap(struct super_block *sb,sector_t block,struct mfs_bitmap *bitmap,uint64_t bits)
{
    int err = 0;
    size_t mapsize = BITS_TO_LONGS(bits) * sizeof(long unsigned int);

    bitmap->map = kmalloc(mapsize, GFP_KERNEL);
    if(!bitmap->map) {
        err = -ENOMEM;
        goto release;
    }
    bitmap->bits = bits;

    err = mfs_read_blockdev(sb,block,0,mapsize,bitmap->map);
    if(err != 0) {
        err = -EINVAL;
        goto release;
    }

release:
    if( err != 0 && bitmap->map) {
        kfree(bitmap->map);
        bitmap->map = NULL;
        bitmap->bits = 0;
    }
    return err;
}

int mfs_save_bitmap(struct super_block *sb,sector_t block,struct mfs_bitmap *bitmap)
{
    return 0;
}

void mfs_set_bit_bitmap(struct mfs_bitmap *bitmap,uint64_t bit,int set)
{
    if(set) {
        set_bit(bit, bitmap->map);
    } else {
        clear_bit(bit, bitmap->map);
    }
}

void mfs_destroy_bitmap(struct mfs_bitmap *bitmap)
{
    if(bitmap->map) {
        kfree(bitmap->map);
        bitmap->map = NULL;
        bitmap->bits = 0;
    }
}