#include "superblock.h"

#include <linux/kernel.h>
#include <linux/time.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/parser.h>
#include <linux/seq_file.h>

#include "fs.h"
#include "inode.h"
#include "freemap.h"
#include "utils.h"

#define MAX_MOUNTS 255

struct mfs_fs_info mounts[MAX_MOUNTS];

static void mfs_put_super(struct super_block *sb)
{
    uint8_t mount_id;

    pr_info("Destroying mfs super block\n");

    MFS_SB(sb).mounted = 0;
    if (unlikely(mfs_save_sb(sb) != 0)) {
        pr_err("cannot set filesystem as unmounted\n"); }
    
    mount_id = MFS_FSINFO(sb)->mount_id;
    memset(&mounts[mount_id],0,sizeof(struct mfs_fs_info));
}

static void mfs_destroy_inode(struct inode *inode)
{
    pr_info("Destroying mfs inode\n");

    if(likely(inode->i_private)) {
        kfree(inode->i_private);
        inode->i_private = NULL;
    }
}

static int mfs_show_options(struct seq_file *m, struct dentry *root)
{
    struct mfs_fs_info *fsi = root->d_sb->s_fs_info;
    seq_printf(m, ",mode=%o", fsi->mount_opts.mode);
    return 0;
}

static const struct super_operations mfs_super_ops = {
    .put_super     = mfs_put_super,
    .destroy_inode = mfs_destroy_inode,
    .show_options  = mfs_show_options,
};

enum {
    Opt_mode,
    Opt_err
};

static const match_table_t tokens = {
    {Opt_mode, "mode=%o"},
    {Opt_err,  NULL}
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

static int mfs_read_disk_superblock(struct super_block *sb,struct mfs_fs_info *fsi)
{
    int err = mfs_read_blockdev(sb,MFS_SUPERBLOCK_BLOCK,0,MFS_SUPERBLOCK_SIZE,&fsi->sb);
    if(unlikely(err != 0)) {
        return err;
    }

    if(unlikely(fsi->sb.magic != MFS_MAGIC_NUMBER)) {
        pr_err("Invalid magic number for mfs, is: 0x%08llx, expected: 0x%08llx\n",fsi->sb.magic, MFS_MAGIC_NUMBER);
        return -EINVAL;
    }
    if(unlikely(fsi->sb.version != MFS_VERSION)) {
        pr_err("Invalid version for mfs, is: 0x%08llx, expected: 0x%08llx\n",fsi->sb.version, MFS_VERSION);
        return -EINVAL;
    } else {
        pr_info("Found mfs v%llu.%llu, blocks: %llu, blocksize: %u, size: %llu KB \n",
            MFS_GET_MAJOR_VERSION((fsi->sb.version)), MFS_GET_MINOR_VERSION((fsi->sb.version)),
            fsi->sb.block_count,
            fsi->sb.block_size,
            (fsi->sb.block_size * (fsi->sb.block_count)/1024 ));
    }

    return 0;
}

static int mfs_create_fs_info(char *data, struct mfs_fs_info **fsi)
{
    int err;
    uint8_t m_id;

    for(m_id = 0; m_id < MAX_MOUNTS; m_id++) {
        if(mounts[m_id].mount_id_used == 0) {
            break; }
    }

    memset(&mounts[m_id],0,sizeof(struct mfs_fs_info));
    if(likely(data)) {
        err = mfs_parse_options(data, &mounts[m_id].mount_opts);
        if (unlikely(err != 0)) {
            return -EINVAL; }
    }

    mounts[m_id].mount_id = m_id;
    mounts[m_id].mount_id_used = 1;
    *fsi = &mounts[m_id];
    return 0;
}

static int mfs_read_root_inode(struct super_block *sb)
{
    int err = 0;
    struct inode *root = NULL;

    err = mfs_read_disk_inode(sb, &root, MFS_SB(sb).rootinode_block);
    if(unlikely(err != 0)) {
        return err; }

    sb->s_root = d_make_root(root);
    if (unlikely(!sb->s_root)) {
        pr_err("root inode allocation failed\n");
        return -EINVAL;
    }

    return err;
}

int mfs_fill_sb(struct super_block *sb, void *data, int silent)
{
    struct mfs_fs_info *fsi;
    int err;

    err = mfs_create_fs_info((char*)data, &fsi);
    if (unlikely(err != 0)) {
        goto release; }
    sb->s_fs_info = fsi;

    err = mfs_read_disk_superblock(sb,fsi);
    if(unlikely(err != 0)) {
        goto release; }

    if(unlikely(MFS_SB(sb).mounted != 0)) {
        pr_err("filesystem is already mounted\n");
        err = -EIO;
        goto release; }

    MFS_SB(sb).mounted = 1;
    MFS_SB(sb).mount_cnt++;
    err = mfs_save_sb(sb);
    if (unlikely(err != 0)) {
        pr_err("cannot set filesystem as mounted\n");
        goto release; }

    if(unlikely(sb_set_blocksize(sb, MFS_SB(sb).block_size) != MFS_SB(sb).block_size)) {
        pr_err("could not set blocksize\n");
        err = -EINVAL;
        goto release;
    }

    sb->s_maxbytes = MAX_LFS_FILESIZE;
    sb->s_magic = MFS_MAGIC_NUMBER;
    sb->s_op = &mfs_super_ops;
    sb->s_time_gran = 1;

    err = mfs_read_root_inode(sb);
    if (unlikely(err != 0)) {
        pr_err("cannot load root inode\n");
        goto release; }

release:
    return err;
}

int mfs_save_sb(struct super_block *sb)
{
    return mfs_write_blockdev(sb,MFS_SUPERBLOCK_BLOCK,0,sizeof(struct mfs_super_block),&(MFS_SB(sb)));
}

uint64_t mfs_get_next_inode_no(struct super_block *sb)
{
    uint64_t ino = MFS_SB(sb).next_ino;
    MFS_SB(sb).next_ino++;
    mfs_save_sb(sb);
    return ino;
}

void mfs_init_mounts(void)
{
    memset(mounts,0,sizeof(struct mfs_fs_info) * MAX_MOUNTS);
}
