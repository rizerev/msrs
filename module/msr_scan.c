#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/stat.h>
#include <linux/atomic.h>
#include <linux/workqueue.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>

#include "msr_scan.h"
#include "util.h"
#include "scan.h"
#include "msr_file.h"

static int msr_device_open(struct inode *inode, struct file *file);
static long msr_device_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param);
static ssize_t msr_device_read(struct file *file, char __user *user_buffer, size_t size, loff_t *offset);
static int msr_device_release(struct inode *inode, struct file *file);

extern bool scanning;
extern bool scan_error;
extern struct msr_file result_file;

static bool busy = false;
static spinlock_t busy_lock;

static struct file_operations file_ops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = msr_device_ioctl,
    .open = msr_device_open,
    .read = msr_device_read,
    .release = msr_device_release
};

static struct miscdevice msr_misc_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = MSR_DEVICE_NAME,
    .fops = &file_ops,
    .mode = S_IRUGO
};

static bool perform_scan(const int num_workers) {
    struct workqueue_struct *wq;
    struct work_struct *works;
    int wc;

    wq = alloc_workqueue(SCAN_WQ_NAME, WQ_UNBOUND, num_workers);
    if (!wq) {
        printk(OUT_PROMPT "ERROR! Could not create workqueue!\n");
        return false;
    }

    works = vmalloc(sizeof(struct work_struct) * num_workers);
    if (!works) {
        printk(OUT_PROMPT "ERROR! Could not allocate work_struct memory!\n");
        destroy_workqueue(wq);
        return false;
    }

    setup_scan(~(0ll));
    //setup_scan(0xffffff);

    for (wc = 0; wc < num_workers; wc++) {
        INIT_WORK(works + wc, scan_msrs);
        queue_work(wq, works + wc);
    }

    flush_workqueue(wq);
    destroy_workqueue(wq);
    vfree(works);

    scanning = false;

    return !scan_error;
}

static int msr_device_open(struct inode *inode, struct file *file) {
    printk(OUT_PROMPT "inside open\n");

    spin_lock(&busy_lock);
    if (busy) {
        spin_unlock(&busy_lock);
        return -EBUSY;
    }
    busy = true;
    spin_unlock(&busy_lock);

    return 0;
}

static long msr_device_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param) {
    const int pres = num_present_cpus();

    if (scanning) {
        return -EBUSY;
    }

    switch (ioctl_num) {
        case MSR_IOCTL_CMD_TRIGGER_SCAN:
            printk(OUT_PROMPT "Starting to scan...\n");
            if (((unsigned int) ioctl_param) == 0 || ((unsigned int) ioctl_param) > (pres * 2)) {
                printk(OUT_PROMPT "ERROR! Number of workers has to be > 0 and <= (num_present_cpus * 2) but is: %d\n", (int) ioctl_param);
                return -EINVAL;
            }

            if (!perform_scan((int) ioctl_param)) {
                printk(OUT_PROMPT "ERROR! Could not finish scan successfully\n");
                return -EFAULT;
            }
            printk(OUT_PROMPT "done\n");
            break;
        default:
            printk(OUT_PROMPT "ERROR! Invalid ioctl\n");
            return -EINVAL;
    }

    return 0;
}

static ssize_t msr_device_read(struct file *file, char __user *user_buffer, size_t size, loff_t *offset) {
    int err;
    ssize_t byte_count;
    const ptrdiff_t element_count = result_file.next_element_ - result_file.buffer_;
    loff_t adj_offset;

    adj_offset = *offset / sizeof(struct typed_msr);

    if (*offset < 0 || adj_offset >= element_count) {
        return 0;
    }

    byte_count = (result_file.next_element_ - (result_file.buffer_ + adj_offset)) * sizeof(struct typed_msr);

    printk(OUT_PROMPT "inside read. byte count is: %zd size is: %zu offset is: %lld element_count is %lu\n", byte_count, size, *offset, element_count);

    if (byte_count <= 0) {
        return 0;
    }

    if (byte_count > size) {
        byte_count = size;
    }

    err = copy_to_user(user_buffer, result_file.buffer_ + adj_offset, byte_count);

    if (err) {
        printk(OUT_PROMPT "ERROR! Could not write data to user!\n");
        return -EFAULT;
    }

    return byte_count;
}

static int msr_device_release(struct inode *inode, struct file *file) {
    spin_lock(&busy_lock);
    busy = false;
    spin_unlock(&busy_lock);
    return 0;
}

static int __init msr_scan_init(void) {
    int error;
    printk(OUT_PROMPT "init\n");

    spin_lock_init(&busy_lock);

    error = misc_register(&msr_misc_device);
    if (error) {
        printk(OUT_PROMPT "Could not create misc device! Code: %d\n", error);
        goto MSI_ERROR;
    }

    printk(OUT_PROMPT "done\n");
    return 0;
MSI_ERROR:
    return -ECANCELED;
}

static void __exit msr_scan_exit(void) {
    misc_deregister(&msr_misc_device);
    msr_file_reset(&result_file);
    printk(OUT_PROMPT "exit\n");
}

module_init(msr_scan_init);
module_exit(msr_scan_exit);
MODULE_AUTHOR("Martin Haubenwallner");
MODULE_LICENSE("GPL");
