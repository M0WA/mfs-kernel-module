#include "utils.h"

#include <linux/buffer_head.h>

void mfs_timet_to_timespec(uint64_t t, struct timespec64* ts) {
    ts->tv_sec = t;
    ts->tv_nsec = 0;
}

int mfs_read_blockdev(struct super_block *sb,sector_t block,size_t offset,size_t len,void *data)
{
    struct buffer_head *bh;
    unsigned char *tmp;
    size_t cpy, i;
    int err = 0;
    unsigned char *buf = (unsigned char *)data;
    size_t blocklen = offset + len;
    size_t loops    = DIV_ROUND_UP(blocklen,sb->s_blocksize);

    for(i = 0; i <= loops; i++) {
        bh = sb_bread(sb, block + i);
        if(unlikely(!bh)) {
            pr_err("error reading from block device: block %lu\n",block + i);
            return -EINVAL; }

        tmp = bh->b_data;
        cpy = sb->s_blocksize;

        if(unlikely(offset != 0)) {
            tmp += offset;
            cpy -= offset;
            offset = 0; }

        if(unlikely(cpy > len)) {
            cpy = len; }

        memcpy(buf,tmp,cpy);

        len -= cpy;
        buf += cpy;

        brelse(bh);
    }

    return err;
}

int mfs_write_blockdev(struct super_block *sb,sector_t block,size_t offset,size_t len,void *data)
{
    struct buffer_head *bh;
    unsigned char *tmp;
    int err = 0;
    size_t cpy, i;
    unsigned char *buf = (unsigned char *)data;
    size_t blocklen = offset + len;
    size_t loops    = DIV_ROUND_UP(blocklen,sb->s_blocksize);

    for(i = 0; i <= loops; i++) {
        bh = sb_bread(sb, block + i);
        if(unlikely(!bh)) {
            pr_err("error reading from block device: block %lu\n",block + i);
            return -EINVAL; }

        tmp = bh->b_data;
        cpy = sb->s_blocksize;

        if(unlikely(offset != 0)) {
            tmp += offset;
            cpy -= offset;
            offset = 0; }

        if(unlikely(cpy > len)) {
            cpy = len; }

        memcpy(tmp,buf,cpy);

        len -= cpy;
        buf += cpy;

    }
    mark_buffer_dirty(bh);
    sync_dirty_buffer(bh);
    brelse(bh);
    return err;
}
