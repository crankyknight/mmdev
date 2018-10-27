#ifndef _MMDEV_H_
#define _MMDEV_H_

#define MMDEV_NR_DEV  (4)
#define MMDEV_SIZE    (4096)

#define MAGIC_VAL     (0x5a)

#define MMDEV_DBG_GEN   (0x1)
#define MMDEV_DBG_RD    (0x2)
#define MMDEV_DBG_WR    (0x4)
#ifdef __KERNEL__
#       define KDBG(fmt, args...) \
        do{ \
            if(mmdev_debug & MMDEV_DBG_GEN) \
                printk(KERN_ERR "mmdev : " fmt, ##args); \
        }while(0)

#       define KDBGRD(fmt, args...) \
        do{ \
            if(mmdev_debug & MMDEV_DBG_RD) \
                printk(KERN_ERR "mmdev read : " fmt, ##args); \
        }while(0)

#       define KDBGWR(fmt, args...) \
        do{ \
            if(mmdev_debug & MMDEV_DBG_WR) \
                printk(KERN_ERR "mmdev write: " fmt, ##args); \
        }while(0)
#endif

struct mmdev_dev{
    void *data;
    u32 mmdev_size;
    loff_t curpos;
    struct cdev cdev;
    struct semaphore sem;
};

#endif /*_MMDEV_H_*/
