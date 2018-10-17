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
#include <linux/string.h> /* memset */

#include "mmdev.h"

int mmdev_nr_dev = MMDEV_NR_DEV;
int mmdev_debug = 7;

module_param(mmdev_nr_dev, int, 0444);
module_param(mmdev_debug, int, 0644);

MODULE_AUTHOR("Abhishek Kumar");
MODULE_LICENSE("Dual BSD/GPL");

struct mmdev_dev *mmdev_devices;
u32 mmdev_major;
u32 mmdev_minor = 0;

static int mmdev_open(struct inode* i_node, struct file* filp)
{
    struct mmdev_dev *device = container_of(i_node->i_cdev, struct mmdev_dev, cdev);
    filp->private_data = device;
    if((filp->f_flags & O_ACCMODE) == O_WRONLY){
        if(device->data){
            if(down_interruptible(&device->sem))
                return -ERESTARTSYS;
            kfree(device->data);
            up(&device->sem);
        }
    }
    return 0;
}

static int mmdev_release(struct inode* i_node, struct file* filp)
{
    return 0;
}

static ssize_t mmdev_read(struct file* filp, char __user* ubuf, size_t size, loff_t* offset)
{
    ssize_t ret;
    struct mmdev_dev *device = filp->private_data;
    KDBGRD("Starting module read\n");
    if(down_interruptible(&device->sem))
        return -ERESTARTSYS;
    if(!access_ok(VERIFY_WRITE, (void __user*)ubuf, (unsigned long)size)){
        KDBGRD("Access not ok\n");
        ret = -EFAULT;
        goto finish;
    }
    if(!device->data || (*offset > device->mmdev_size)){
        KDBGRD("Offset too large\n");
        ret = -EFAULT;
        goto finish;
    }
    size = ((*offset + size) > device->mmdev_size) ? device->mmdev_size - *offset : size; 
    if(copy_to_user(ubuf, &((char*)(device->data))[*offset], size)){
        ret = -EFAULT;
        goto finish;
    }
    /*Update file offset*/
    *offset += size;
    ret = size;

    finish :
        up(&device->sem);
        return ret;
}

static ssize_t mmdev_write(struct file* filp, const char __user* ubuf, size_t size, loff_t* offset)
{
    ssize_t ret;
    u32 rem;
    struct mmdev_dev *device = filp->private_data;

    KDBGWR("Starting module write\n");
    if(down_interruptible(&device->sem))
        return -ERESTARTSYS;

    if(!access_ok(VERIFY_READ, (void __user*)ubuf, (unsigned long)size)){
        KDBGWR("Access not ok\n");
        ret = -EFAULT;
        goto finish;
    }
    if(*offset > device->mmdev_size){
        KDBGWR("Offset too large\n");
        ret = -EFAULT;
        goto finish;
    }
    size = (*offset + size) > device->mmdev_size ? device->mmdev_size - *offset : size;
    KDBGWR("Data pointer = %p\n", device->data);
    if(!device->data){
        /*If not allocated, allocate now */
        device->data = kmalloc(device->mmdev_size, GFP_KERNEL);
        KDBGWR("Allocated data as %p\n", device->data);
    }

    if(copy_from_user(&((char*)(device->data))[*offset], ubuf, size)){
        KDBGWR("Copy failed\n");
        ret = -EFAULT;
        goto finish;
    }
    *offset += size;
    ret = size;
    rem = device->mmdev_size - *offset;
    if(rem){
        /*Fill remainder of device with MAGIC_VAL*/
        memset(&((char*)(device->data))[*offset], MAGIC_VAL, rem);
    }

    finish :
        up(&device->sem);
        return ret;
}

static struct file_operations mmdev_fops = {
    .owner   = THIS_MODULE,
    .open    = mmdev_open,
    .release = mmdev_release,
    .read    = mmdev_read,
    .write   = mmdev_write,
};

static void mmdev_exit_module(void)
{
    u32 i;
    if(mmdev_devices){
        for(i=0; i<mmdev_nr_dev; i++){
            if(mmdev_devices[i].data){
                kfree(mmdev_devices[i].data);
            }
            cdev_del(&mmdev_devices[i].cdev);
        }
        kfree(mmdev_devices);
    }

    unregister_chrdev_region(MKDEV(mmdev_major, mmdev_minor), mmdev_nr_dev);
    KDBG("Module exit complete\n");
}

static void mmdev_setup_cdev(struct mmdev_dev *mmdev, u32 index)
{
    u32 res;
    dev_t dev = MKDEV(mmdev_major, mmdev_minor + index);

    cdev_init(&mmdev->cdev, &mmdev_fops);
    mmdev->cdev.owner = THIS_MODULE;
    res = cdev_add(&mmdev->cdev, dev, 1);
    if(res < 0)
        KDBG("Unable to add cdev\n");
}


int mmdev_init_module(void)
{
    dev_t dev;
    u32 result, i;

    result = alloc_chrdev_region(&dev, mmdev_minor, mmdev_nr_dev, "mmdev");
    if(result<0){
        KDBG("Chardev allocation error");
        return result;
    }
    mmdev_major = MAJOR(dev);
   
    mmdev_devices = kmalloc(mmdev_nr_dev * sizeof(struct mmdev_dev), GFP_KERNEL);
    if(!mmdev_devices){
        KDBG("Memory allocation error");
        goto out;
    }

    for(i=0; i<mmdev_nr_dev; i++){
        mmdev_devices[i].data = NULL;
        mmdev_devices[i].mmdev_size = MMDEV_SIZE;
        sema_init(&mmdev_devices[i].sem, 1);
        mmdev_setup_cdev(&mmdev_devices[i], i); 
    }

    KDBG("Module init complete\n");
    return 0;

    out :
        mmdev_exit_module();
        return 1;
}

module_init(mmdev_init_module);
module_exit(mmdev_exit_module);
