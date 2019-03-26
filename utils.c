#include "utils.h"

#include <linux/buffer_head.h>
#include <linux/slab.h>

void mfs_timet_to_timespec(uint64_t t, struct timespec64* ts) {
    ts->tv_sec = t;
    ts->tv_nsec = 0;
}

int mfs_read_blockdev(struct super_block *sb,sector_t block,size_t offset,size_t len,void *data)
{
    struct buffer_head *bh;
    unsigned char *tmp, *buf;
    size_t cpy, i, blocklen,loops;
    int err = 0;

    if(unlikely(offset >= sb->s_blocksize)) {
        block += offset/sb->s_blocksize;
        offset = offset % sb->s_blocksize; }

    buf = (unsigned char *)data;
    blocklen = offset + len;
    loops    = DIV_ROUND_UP(blocklen,sb->s_blocksize);

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

        if(cpy == 0) {
            break; }
        
        //pr_info("read from blockdev loop at block %.6zu: %.4zu/%.4zu (%.8zu/%.8zu)\n",block + i,i,loops,cpy,len);
        memcpy(buf,tmp,cpy);

        len -= cpy;
        buf += cpy;

        brelse(bh);
    }

    return err;
}

int mfs_write_blockdev(struct super_block *sb,sector_t block,size_t offset,size_t len,void const * data)
{
    struct buffer_head *bh;
    unsigned char *tmp,*buf;
    size_t cpy, i,blocklen,loops;
    int err = 0;

    if(unlikely(offset >= sb->s_blocksize)) {
        block += offset/sb->s_blocksize;
        offset = offset % sb->s_blocksize; }

    buf      = (unsigned char *)data;
    blocklen = offset + len;
    loops    = DIV_ROUND_UP(blocklen,sb->s_blocksize);

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

        if(cpy == 0) {
            break; }

        //pr_info("write to blockdev loop at block %.6zu, %lx %.4zu/%.4zu (%.8zu/%.8zu)\n",block + i,(block + i)*sb->s_blocksize,i,loops,cpy,len);
        memcpy(tmp,buf,cpy);

        len -= cpy;
        buf += cpy;

        mark_buffer_dirty(bh);
        sync_dirty_buffer(bh);
        brelse(bh);
    }
    return err;
}

int mfs_copy_blockdev(struct super_block *sb,sector_t src_block,size_t offset_src,sector_t dst_block,size_t offset_dst,size_t len_bytes)
{
    int err;
    unsigned char *block_buf;
    size_t buf_byte_base,buf_byte_written,buf_byte_len,src_start_offset,dst_block_count;

    if(unlikely(offset_src >= sb->s_blocksize)) {
        src_block += offset_src/sb->s_blocksize;
        offset_src = offset_src % sb->s_blocksize; }

    if(unlikely(offset_dst >= sb->s_blocksize)) {
        dst_block += offset_dst/sb->s_blocksize;
        offset_dst = offset_dst % sb->s_blocksize; }

    block_buf = kmalloc(sb->s_blocksize * 2, GFP_KERNEL);
    if(unlikely(!block_buf)) {
        pr_err("cannot allocate buffer in mfs_copy_blockdev\n");
        return -ENOMEM; }

    buf_byte_base = 0;
    buf_byte_len = (sb->s_blocksize - offset_src);
    src_start_offset = 1;
    if(likely(buf_byte_len < len_bytes)) {
        size_t rest = len_bytes - buf_byte_len;
        buf_byte_len += (rest > sb->s_blocksize ? sb->s_blocksize : rest);
        src_start_offset++;
    }

    err = mfs_read_blockdev(sb,src_block,offset_src,buf_byte_len,block_buf);
    if(unlikely(err)) {
        pr_err("cannot fill buffer in mfs_copy_blockdev\n");
        return err; }
    src_block += src_start_offset;

    buf_byte_written = 0;
    dst_block_count = DIV_ROUND_UP(len_bytes + offset_dst,sb->s_blocksize);
    if(unlikely(offset_dst)) {
        size_t fill_block_size = sb->s_blocksize - offset_dst;
        err = mfs_write_blockdev(sb,dst_block,offset_dst,fill_block_size,block_buf);
        if(unlikely(err)) {
            pr_err("cannot write buffer in mfs_copy_blockdev\n");
            return err; }
        buf_byte_written += fill_block_size;
        dst_block_count--;
    }

    while(likely(dst_block_count)) {
        size_t read_pos   = buf_byte_written - buf_byte_base;
        size_t rest_buf   = buf_byte_len - (buf_byte_written - buf_byte_base);
        size_t rest_copy  = len_bytes - buf_byte_written;
        size_t write_size = rest_copy > sb->s_blocksize ? sb->s_blocksize : rest_copy;

        if(likely(rest_buf < write_size)) {
            memmove(block_buf,&block_buf[read_pos],rest_buf);
            buf_byte_base += read_pos;
            buf_byte_len = rest_buf;
            err = mfs_read_blockdev(sb,src_block,0,write_size,&block_buf[buf_byte_len]);
            if(unlikely(err)) {
                pr_err("cannot refill buffer in mfs_copy_blockdev\n");
                return err; }
            buf_byte_len += write_size;
            src_block++;
            continue; //try again
        }

        err = mfs_write_blockdev(sb,dst_block,0,write_size,&block_buf[read_pos]);
        if(unlikely(err)) {
            pr_err("cannot write copy in mfs_copy_blockdev\n");
            return err; }

        dst_block_count--;
    }

    kfree(block_buf);
    return 0;
}
