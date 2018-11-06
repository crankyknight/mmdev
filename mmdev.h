#ifndef _MMDEV_H_
#define _MMDEV_H_

#define MMDEV_NR_DEV  (4)
#define MMDEV_SIZE    (4096)

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
    u32 w_size;
    u8 magic_val;
    wait_queue_head_t readq;
    u32 vmaps;
    struct cdev cdev;
    struct semaphore sem;
};

/* IOCTL definitions */
#define MMDEV_IOCTL_MAGIC ('x')

#define MMDEV_IOCRESET _IO(MMDEV_IOCTL_MAGIC, 0)
#define MMDEV_IOCGTOTSIZE _IOR(MMDEV_IOCTL_MAGIC, 1, u32)
#define MMDEV_IOCGCURSIZE _IOR(MMDEV_IOCTL_MAGIC, 2, u32)
#define MMDEV_IOCSTOTSIZE _IOW(MMDEV_IOCTL_MAGIC, 3, u32)
#define MMDEV_IOCSCURSIZE _IOW(MMDEV_IOCTL_MAGIC, 4, u32)
#define MMDEV_IOCXCURSIZE _IOWR(MMDEV_IOCTL_MAGIC, 5, u32)
#define MMDEV_IOCXFILLER  _IOWR(MMDEV_IOCTL_MAGIC, 6, u8)

#define MMDEV_IOCMAXNR (6)

#endif /*_MMDEV_H_*/
