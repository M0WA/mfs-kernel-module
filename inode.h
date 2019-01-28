#pragma once

struct mfs_inode {
	mode_t mode;
	uint64_t inode_no;
	union {
		uint64_t file_size;
		uint64_t dir_children_count;
	};
};