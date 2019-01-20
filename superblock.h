#pragma once

#define MFS_SUPERBLOCK_SIZE 4096
#define MFS_MAGIC_NUMBER    ((uint64_t)0x2410198324101983)
#define MFS_SUPERBLOCK_POS  0   //disk block where superblock is stored

struct mfs_super_block {
    uint64_t version;
    uint64_t magic;
    uint64_t block_size;

    /* FIXME: This should be moved to the inode store and not part of the sb */
    uint64_t inodes_count;

    uint64_t free_blocks;

    /** FIXME: move this into separate struct */
    //struct journal_s *journal;        
};

union mfs_padded_super_block { 
    struct mfs_super_block sb;
    unsigned char padding[MFS_SUPERBLOCK_SIZE];
};