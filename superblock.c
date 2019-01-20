#include "superblock_int.h"

#include <linux/kernel.h>
#include <linux/time.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/parser.h>
#include <linux/seq_file.h>
#include <linux/buffer_head.h>

#include "fs_int.h"

/*
struct super_operations {
   	struct inode *(*alloc_inode)(struct super_block *sb);
	void (*destroy_inode)(struct inode *);

   	void (*dirty_inode) (struct inode *, int flags);
	int (*write_inode) (struct inode *, struct writeback_control *wbc);
	int (*drop_inode) (struct inode *);
	void (*evict_inode) (struct inode *);
	void (*put_super) (struct super_block *);
	int (*sync_fs)(struct super_block *sb, int wait);
	int (*freeze_super) (struct super_block *);
	int (*freeze_fs) (struct super_block *);
	int (*thaw_super) (struct super_block *);
	int (*unfreeze_fs) (struct super_block *);
	int (*statfs) (struct dentry *, struct kstatfs *);
	int (*remount_fs) (struct super_block *, int *, char *);
	void (*umount_begin) (struct super_block *);

	int (*show_options)(struct seq_file *, struct dentry *);
	int (*show_devname)(struct seq_file *, struct dentry *);
	int (*show_path)(struct seq_file *, struct dentry *);
	int (*show_stats)(struct seq_file *, struct dentry *);
#ifdef CONFIG_QUOTA
	ssize_t (*quota_read)(struct super_block *, int, char *, size_t, loff_t);
	ssize_t (*quota_write)(struct super_block *, int, const char *, size_t, loff_t);
	struct dquot **(*get_dquots)(struct inode *);
#endif
	int (*bdev_try_to_free_page)(struct super_block*, struct page*, gfp_t);
	long (*nr_cached_objects)(struct super_block *,
				  struct shrink_control *);
	long (*free_cached_objects)(struct super_block *,
				    struct shrink_control *);
};
*/

static void mfs_put_super(struct super_block *sb)
{
    pr_info("Destroying mfs super block\n");
    if(sb->s_fs_info) {
        kfree(sb->s_fs_info); }
	//kill_block_super(sb);
}

static int mfs_show_options(struct seq_file *m, struct dentry *root)
{
	struct mfs_fs_info *fsi = root->d_sb->s_fs_info;
	seq_printf(m, ",mode=%o", fsi->mount_opts.mode);	
	return 0;
}

static const struct super_operations mfs_super_ops = {
    .put_super = mfs_put_super,
    .show_options = mfs_show_options,
};

enum {
	Opt_mode,
    Opt_blocksize,
	Opt_err
};

static const match_table_t tokens = {
	{Opt_mode,      "mode=%o"},
	{Opt_blocksize, "blocksize=%ul"},
	{Opt_err, NULL}
};

static int mfs_parse_options(char *data, struct mfs_mount_opts *opts)
{
	substring_t args[MAX_OPT_ARGS];
	int option;
	int token;
	char *p;

	opts->mode = MFS_DEFAULT_MODE;

	while ((p = strsep(&data, ",")) != NULL) {
		if (!*p)
			continue;

		token = match_token(p, tokens, args);
		switch (token) {
		case Opt_mode:
			if (match_octal(&args[0], &option))
				return -EINVAL;
			opts->mode = option & S_IALLUGO;
			break;
        case Opt_err:
        default:
            pr_warn("unknown argument for mfs module: %s\n",p);
            break;
		}
	}

	return 0;
}

static int mfs_read_disk_superblock(struct super_block **sb, struct buffer_head **bh, struct mfs_super_block **sb_disk)
{
    int err = 0;

    *bh = __bread_gfp((*sb)->s_bdev, MFS_SUPERBLOCK_POS, MFS_SUPERBLOCK_SIZE, __GFP_MOVABLE);
    if(unlikely(!*bh)) {
        pr_err("Could not read superblock for mfs\n");
        return -EINVAL;
    } else {
        pr_info("Read superblock for mfs\n");
    }
    
    *sb_disk = &((union mfs_padded_super_block *)(*bh)->b_data)->sb;
    if(unlikely((*sb_disk)->magic != MFS_MAGIC_NUMBER)) {
        pr_err("Invalid magic number for mfs, is: 0x%08llx, expected: 0x%08llx\n",(*sb_disk)->magic, MFS_MAGIC_NUMBER);
        err = -EINVAL;
        goto release;
    }
    if(unlikely((*sb_disk)->version != MFS_VERSION)) {
        pr_err("Invalid version for mfs, is: 0x%08llx, expected: 0x%08llx\n",(*sb_disk)->version, MFS_MAGIC_NUMBER);
        err = -EINVAL;
        goto release;
    } else {
        pr_info("Found mfs v%ul.%ul, blocksize: %llu \n",MFS_GET_MAJOR_VERSION((*sb_disk)->version),MFS_GET_MINOR_VERSION((*sb_disk)->version), MFS_MAGIC_NUMBER);
    }

release:
    if(err != 0 && *bh) {
        brelse(*bh);
        *bh = 0;
    }
    return err;
}

static int mfs_create_fs_info(char *data, struct mfs_fs_info **fsi) 
{
    int err;

    *fsi = kzalloc(sizeof(struct mfs_fs_info), GFP_KERNEL);
	if (unlikely(!*fsi)) {
        return -ENOMEM; }

    if(likely(data)) {
        err = mfs_parse_options(data, &(*fsi)->mount_opts);
        if (unlikely(err != 0)) {
            return -EINVAL; }
    }
    return 0;
}

int mfs_fill_sb(struct super_block *sb, void *data, int silent)
{
    struct mfs_fs_info *fsi;
    struct inode *root;
    struct mfs_super_block *sb_disk;
    int err;
    struct buffer_head *bh = NULL;
    
    err = mfs_read_disk_superblock(&sb, &bh, &sb_disk);
    if(unlikely(err != 0)) {
        return err; }

    err = mfs_create_fs_info((char *)data, &fsi);
	if (unlikely(err != 0)) {
		return err; }
	sb->s_fs_info = fsi;

	sb->s_maxbytes = MAX_LFS_FILESIZE;
	sb->s_blocksize	= PAGE_SIZE;
	sb->s_blocksize_bits = PAGE_SHIFT;
    sb->s_magic = MFS_MAGIC_NUMBER;
    sb->s_op = &mfs_super_ops;
    sb->s_time_gran = 1;

    root = new_inode(sb);
    if (unlikely(!root)) {
        pr_err("inode allocation failed\n");
        err = -ENOMEM;
        goto release;
    }

    root->i_ino = 0;
    root->i_sb = sb;    
    root->i_atime = root->i_mtime = root->i_ctime = current_time(root);
    inode_init_owner(root, NULL, S_IFDIR | fsi->mount_opts.mode);

    sb->s_root = d_make_root(root);
    if (unlikely(!sb->s_root)) {
        pr_err("root creation failed\n");
        err = -ENOMEM;
        goto release;
    }

release:
    if(err != 0 && bh) {
        brelse(bh); }
    return err;
}