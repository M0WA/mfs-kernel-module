#include "freemap.h"

#include "bitmap.h"
#include "superblock.h"
#include "fs.h"
#include "utils.h"

#include <linux/bitops.h>

int mfs_alloc_freemap(struct super_block *sb,size_t old_bytes,size_t new_bytes,sector_t* block)
{
    int err;
    struct mfs_bitmap freemap;
    uint64_t old_block_count,new_block_count,diff_block_count;
    sector_t old_block;

    if(unlikely(old_bytes >= new_bytes)) {
        //TODO: implement shrink
        return 0;
    }

    old_block        = *block;
    old_block_count  = DIV_ROUND_UP(old_bytes,sb->s_blocksize);
    new_block_count  = DIV_ROUND_UP(new_bytes,sb->s_blocksize);
    diff_block_count = new_block_count - old_block_count;

    err = mfs_load_bitmap(sb,MFS_SB(sb).freemap_block,&freemap,MFS_SB(sb).block_count);
    if(unlikely(err)) {
        return err;
    }

    if(!old_block) {
        *block = bitmap_find_next_zero_area(freemap.map,freemap.bits,0,new_block_count,0);
    } else {
        size_t diff_block_count = old_block_count - new_block_count;
        sector_t append_block = bitmap_find_next_zero_area(freemap.map,freemap.bits,old_block + 1,diff_block_count,0);

        if( append_block != (old_block + 1) ){
            *block = bitmap_find_next_zero_area(freemap.map,freemap.bits,0,new_block_count,0);
            bitmap_set(freemap.map,*block,diff_block_count);

            err = mfs_copy_blockdev(sb,old_block,0,(*block),0,old_block_count);
            if(unlikely(!err)) {
                return err;
            }

            bitmap_clear(freemap.map,old_block,old_block_count);
        } else {
            bitmap_set(freemap.map,append_block,diff_block_count);
        }
    }

    if(unlikely(!err)) {
        err = mfs_save_bitmap(sb,MFS_SB(sb).freemap_block,&freemap);
    }

    mfs_destroy_bitmap(&freemap);
    return err;    
}
