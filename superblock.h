#pragma once

#include <linux/fs.h>

struct mfs_super_block {
	uint64_t version;
	uint64_t magic;
	uint64_t block_size;

	/* FIXME: This should be moved to the inode store and not part of the sb */
	uint64_t inodes_count;

	uint64_t free_blocks;

	/** FIXME: move this into separate struct */
	//struct journal_s *journal;

	char padding[4048];
};

int mfs_fill_sb(struct super_block *sb, void *data, int silent);