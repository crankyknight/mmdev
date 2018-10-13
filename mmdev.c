#include <linux/module.h>
#include <linux/init.h>
#include <linux/moduleparam.h>

#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/seq_file.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>	/* copy_*_user */

#include "mmdev.h"

int mmdev_nr_dev = MMDEV_NR_DEVS;
int mmdev_debug = 0;

module_param(mmdev_nr_dev, int, 0644);
module_param(mmdev_debug, int, 0644);

MODULE_AUTHOR("Abhishek Kumar");
MODULE("Dual BSD/GPL");

struct mmdev_dev *mmdev_devices;

int mmdev_init_module(void)
{
    KDBG("Module init");

    fail :
        mmdev_exit_module();
        return 1;
}

void mmdev_exit_moduel(void)
{
    KDBG("Module exit");
}

module_init(mmdev_init_module);
module_exit(mmdev_exit_module);
