#ifndef _MMDEV_H_
#define _MMDEV_H_

#define MMDEV_NR_DEVS   (4)

#ifdef __KERNEL__
#   define KDBG(fmt, args...) printk(KERN_ALERT "mmdev : ", fmt, ##args)
#endif

#endif /*_MMDEV_H_*/
