#pragma once


#ifdef __KERNEL__
#include <linux/types.h>
#include <linux/fs.h>
#else
#include <stdint.h>
#endif

#define MFS_SUPERBLOCK_SIZE 4096
#define MFS_MAGIC_NUMBER    ((uint64_t)0x2410198324101983)
#define MFS_SUPERBLOCK_POS  0   //disk block where superblock is stored

#define MFS_FSINFO(x) ((struct mfs_fs_info*)(x)->s_fs_info)
#define MFS_SB(x)     ((struct mfs_fs_info*)(x)->s_fs_info)->sb

struct mfs_super_block {
    uint64_t version;
    uint64_t magic;
    uint32_t block_size;
    uint64_t block_count;
    uint64_t freemap_block;
    uint64_t inodemap_block;
    uint64_t rootinode_block;
    uint64_t next_ino;
};

union mfs_padded_super_block { 
    struct mfs_super_block sb;
    unsigned char padding[MFS_SUPERBLOCK_SIZE];
};


#ifdef __KERNEL__

int mfs_fill_sb(struct super_block *sb, void *data, int silent);
int mfs_read_blockdev(struct super_block *sb,sector_t block,size_t offset,size_t len,void *data);
void mfs_init_mounts(void);

#endif