/*
 * Copyright (C) 2022 Efinix Inc. All rights reserved.
 *
 * SPDX-License-Identifier: GPL-3.0
 *
 */
#include "my_def.h"

#define CLASS_NAME "CLASS_CDEV"

struct st_EvsocDev
{
    void __iomem *base_ptr;
    dev_t devname;
    struct cdev c_dev;
    struct class *driver_class; 
    struct device *driver_device;
    int dev_major;
    int dev_minor;
    struct device_node *nd; 
    int key_gpio; 
    atomic_t keyvalue;
};

static struct st_EvsocDev evsoc_dev;
static volatile int is_open = 0;

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Sample evsoc kernel module");
MODULE_INFO(intree, "Y");

ssize_t evsoc_read(struct file *filep, char __user *msg, size_t nbytes, loff_t *offset)
{
    uint32_t randnum = 0;
    uint32_t bytes_read = 0;
    char msg_out[32];
    int len_out = 0;

    randnum = readl(evsoc_dev.base_ptr);
    sprintf(msg_out, "random number 1 :%d", randnum);
    len_out = strlen(msg_out);

    while ((bytes_read < nbytes) && (*offset < len_out))
    {
        put_user(msg_out[*offset], &msg[bytes_read]);
        *offset = *offset + 1;
        bytes_read++;
    }
    return bytes_read;
}

ssize_t evsoc_write(struct file *filep, const char __user *imsg, size_t nbytes, loff_t *offset)
{
    int i = 0;
    char ch;
    uint32_t reg;
    uint32_t value;
    char msg_in[32];
    int len_in = 0;

    while (i < nbytes)
    {
        get_user(ch, &imsg[i]);
        msg_in[len_in] = ch;
        msg_in[len_in + 1] = '\0';

        if (msg_in[len_in] == '\n')
        {
            reg = 0;
            msg_in[len_in] = 0x00;
            value = simple_strtol(msg_in, NULL, 0);
            reg = (reg | value);
            pr_info("Register value to be written:%x\n", reg);
            writel(reg, evsoc_dev.base_ptr + 4);
            len_in = 0;
        }
        else
        {
            len_in++;
        }
        *offset = *offset + 1;
        i++;
    }
    return 0;
}

int evsoc_open(struct inode *inodep, struct file *filep)
{
    if (is_open == 1)
    {
        pr_info("evsoc device already open\n");
        return -EBUSY;
    }
    is_open = 1;
    try_module_get(THIS_MODULE);
    return 0;
}

int evsoc_release(struct inode *inodep, struct file *filep)
{
    if (is_open == 0)
    {
        pr_info("evsoc device wasn't open\n");
        return -EBUSY;
    }
    is_open = 0;
    module_put(THIS_MODULE);
    return 0;
}

static long evsoc_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    if (_IOC_TYPE(cmd) != DEMO_MAGIC)
        return -ENOTTY;
        
    if  (main_proc(cmd, arg, NULL))
    {
        return 0;
    } 
    return  -EFAULT;

}

struct file_operations fops = {
    .read = evsoc_read,
    .write = evsoc_write,
    .open = evsoc_open,
    .release = evsoc_release,
    .unlocked_ioctl = evsoc_ioctl,
    .mmap = evsoc_mmap,
};

static void test_code(void)
{
}

static int evsoc_init(void)
{
    int dev_valid;
    pr_info("load evsoc module ...\n");
    evsoc_dev.base_ptr = ioremap(GPIO_BASE, SZ_4K);
    dev_valid = alloc_chrdev_region(&evsoc_dev.devname, 0, 1, "chardriver");
    evsoc_dev.dev_major = MAJOR(evsoc_dev.devname);
    // pr_info("evsoc device major number:%d\n", evsoc_dev.dev_major);
    if (dev_valid < 0)
    {
        pr_info("Failed to get allocation for evsoc device!\n");
        return -EBUSY;
    }
    evsoc_dev.driver_class = class_create(THIS_MODULE, CLASS_NAME);
    if (evsoc_dev.driver_class == NULL)
    {
        pr_info("Failed to create driver class!\n");
        unregister_chrdev_region(evsoc_dev.devname, 1);
        return -EBUSY;
    }
    evsoc_dev.driver_device = device_create(evsoc_dev.driver_class, NULL, evsoc_dev.devname, NULL, "evsoc");

    if (evsoc_dev.driver_device == NULL)
    {
        pr_info("Failed to create device!\n");
        class_destroy(evsoc_dev.driver_class);
        unregister_chrdev_region(evsoc_dev.devname, 1);
        return -EBUSY;
    }
    cdev_init(&(evsoc_dev.c_dev), &fops);
    if (cdev_add(&(evsoc_dev.c_dev), evsoc_dev.devname, 1) == -1)
    {
        pr_info("Failed to create char device!\n");
        device_destroy(evsoc_dev.driver_class, evsoc_dev.devname);
        class_destroy(evsoc_dev.driver_class);
        unregister_chrdev_region(evsoc_dev.devname, 1);
        return -EBUSY;
    }

    my_init(evsoc_dev.driver_device);
    main_init(evsoc_dev.driver_device);

    return 0;
}

static void evsoc_exit(void)
{
    main_exit(evsoc_dev.driver_device);
    my_exit(evsoc_dev.driver_device);

    cdev_del(&evsoc_dev.c_dev);
    device_destroy(evsoc_dev.driver_class, evsoc_dev.devname);
    class_destroy(evsoc_dev.driver_class);
    unregister_chrdev_region(evsoc_dev.devname, 1);
    pr_info("Removed evsoc character driver\n");
}

module_init(evsoc_init);
module_exit(evsoc_exit);
