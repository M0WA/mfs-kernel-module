#include "mount.h"

#include <linux/kernel.h>
#include <linux/fs.h>

#include "superblock_int.h"

struct dentry *mfs_mount(struct file_system_type *type, int flags, char const *dev, void *data)
{
    struct dentry *entry = NULL;
    pr_info("Mounting mfs on device %s\n",dev);
    entry = mount_bdev(type, flags, dev, data, mfs_fill_sb);
    if (unlikely(IS_ERR(entry)))
        pr_err("Error mounting simplefs");
    else
        pr_info("Mounted mfs on [%s]\n", dev);
    return entry;
}