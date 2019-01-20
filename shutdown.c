#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>

#include "fs_int.h"

static void __exit shutdown_mod(void)
{
    int res = 0;
	pr_info("Unloading mfs module\n");
    res = unregister_filesystem(&mfs_type);
    if(unlikely(res != 0)) {
        pr_err("Unloading mfs module failed\n");
    } else {
        pr_info("Unregistered mfs file-system\n");
    }
}

module_exit(shutdown_mod);