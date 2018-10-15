#ifndef _MMDEV_H_
#define _MMDEV_H_

#define MMDEV_NR_DEV   (4)
#define MMDEV_SIZE      (4096)


#ifdef __KERNEL__
#       define KDBG(fmt, args...) \
        do{ \
            if(mmdev_debug & 0x01) \
                printk(KERN_ERR "mmdev : " fmt, ##args); \
        }while(0)

#endif

struct mmdev_dev{
    void *data;
    u32 mmdev_size;
    struct cdev cdev;
    struct semaphore sem;
};

#endif /*_MMDEV_H_*/
