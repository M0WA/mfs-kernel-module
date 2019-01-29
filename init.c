#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>

#include "fs.h"
#include "superblock.h"

static int __init init_mod(void)
{
    int res = 0;
    pr_info("Loading mfs module\n");

    mfs_init_mounts();

    res = register_filesystem(&mfs_type);
    if(unlikely(res != 0)) {
        pr_err("Loading mfs module failed\n");
    } else {
        pr_info("Registered mfs file-system\n");
    }

    return res;
}

module_init(init_mod);