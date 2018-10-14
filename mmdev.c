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
int mmdev_debug = 1;

module_param(mmdev_nr_dev, int, 0444);
module_param(mmdev_debug, int, 0644);

MODULE_AUTHOR("Abhishek Kumar");
MODULE_LICENSE("Dual BSD/GPL");

struct mmdev_dev *mmdev_devices;
u32 mmdev_major;
u32 mmdev_minor = 0;

void mmdev_setup_cdev(struct mmdev_dev *mmdev, u32 index)
{
    u32 res;
    dev_t dev = MKDEV(mmdev_major, mmdev_minor + index);

    cdev_init(&mmdev->cdev, &mmdev_fops);
    mmdev->cdev.owner = THIS_MODULE;
    res = cdev_add(&mmdev->cdev, dev, 1);
    if(res < 0)
        KDBG("Unable to add cdev\n");
}

void mmdev_exit_module(void)
{
    KDBG("Module exit\n");
}

int mmdev_init_module(void)
{
    dev_t dev;
    u32 result, i;

    result = alloc_chrdev_region(&dev, mmdev_minor, mmdev_nr_devs);
    if(result<0){
        KDBG("Chardev allocation error");
        return result;
    }
    mmdev_major = MAJOR(dev);
   
    mm_device = kmalloc(mmdev_nr_devs * sizeof(struct mmdev_dev));
    if(!mm_device){
        KDBG("Memory allocation error");
        goto out;
    }

    for(i=0; i<mmdev_nr_devs; i++){
        mm_device[i].mmdev_size = mmdev_size;
        sema_init(&mm_device[i].sem, 1);
        mmdev_setup_cdev(&mm_device[i], i); 
    }

    out :
        mmdev_exit_module();
        return 1;
}

module_init(mmdev_init_module);
module_exit(mmdev_exit_module);
