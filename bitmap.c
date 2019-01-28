#include "bitmap.h"

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/buffer_head.h>
#include <linux/bitmap.h>
#include <linux/slab.h>
#include <linux/bitops.h>

int mfs_load_bitmap(struct super_block *sb,uint64_t pos,struct mfs_bitmap *bitmap,uint64_t bits)
{
    struct buffer_head *bh;
    uint64_t long_count = BITS_TO_LONGS(bits);
    size_t bytes = long_count * sizeof(long unsigned int);
    size_t read  = 0;
    sector_t block = pos / sb->s_bdev->bd_block_size;
    int err = 0;
    unsigned char *buf = NULL;

    bitmap->map = kmalloc(bytes, GFP_KERNEL);
    if(!bitmap->map) {
        err = -ENOMEM;
        goto release;
    }
    bitmap->bits = bits;

    buf = (unsigned char *)bitmap->map;
    while( (bytes - read) > 0 ) {
        pr_debug("Reading bitmap for mfs block: %lu, bits: %llu, bytes: %zu, bs: %u, left: %zd \n",block,bits,bytes,sb->s_bdev->bd_block_size,(bytes - read));

        bh = sb_bread(sb, block);
        if(unlikely(!bh)) {
            pr_err("Could not read bitmap for mfs pos: %llu, bits: %llu, bytes: %zu, bs: %u\n",pos,bits,bytes,sb->s_bdev->bd_block_size);
            err = -EINVAL;
            goto release;
        }

        memcpy(&(buf[read]),bh->b_data, ( ((bytes-read) < sb->s_bdev->bd_block_size) ? (bytes-read) : sb->s_bdev->bd_block_size) );

        read += sb->s_bdev->bd_block_size;
        block++;
        __brelse(bh);
    }
    pr_debug("Read bitmap for mfs at byte pos: %llu, bytes: %zu\n",pos,bytes);

/*
    bitmap_copy(bitmap->map,(const long unsigned int*)bh->b_data,bits);
*/

release:
    if( err != 0 && bitmap->map) {
        kfree(bitmap->map);
        bitmap->map = NULL;
        bitmap->bits = 0;
    }
    return err;
}

int mfs_save_bitmap(struct super_block *sb,uint64_t pos,struct mfs_bitmap *bitmap)
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