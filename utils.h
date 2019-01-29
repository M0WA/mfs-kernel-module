#pragma once

#include <linux/fs.h>
#include <linux/time.h>

void mfs_timet_to_timespec(uint64_t t, struct timespec64* ts);

int mfs_read_blockdev(struct super_block *sb,sector_t block,size_t offset,size_t len,void *data);
int mfs_write_blockdev(struct super_block *sb,sector_t block,size_t offset,size_t len,void *data);