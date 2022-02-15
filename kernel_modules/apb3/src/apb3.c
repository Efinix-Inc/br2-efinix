/*
 * Copyright (C) 2022 Efinix Inc. All rights reserved.
 *
 * SPDX-License-Identifier: GPL-3.0
 *
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/ioctl.h>
#include <linux/io.h>

#define CLASS_NAME "CLASS_CDEV"
#define BASE 0xf8100000        

static void __iomem *base = NULL;
volatile static int is_open = 0;

/*register as char device*/
static struct class *driver_class = NULL;
static dev_t devname;
static int dev_major;
static int dev_valid;
static struct cdev c_dev;

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Sample apb3 kernel module");


ssize_t apb3_read(struct file *filep, char __user * msg, size_t nbytes, loff_t *offset)
{
    uint32_t randnum = 0;
    uint32_t bytes_read = 0;
    char msg_out[32];    
    int len_out = 0;

    randnum = readl(base);
    sprintf(msg_out, "random number :%d", randnum);
    len_out = strlen(msg_out);

    while((bytes_read < nbytes) && (*offset < len_out)) {
        put_user(msg_out[*offset], &msg[bytes_read]);
        *offset=*offset + 1;
        bytes_read++;
    }
    return bytes_read;
}

ssize_t apb3_write(struct file *filep, const char __user * imsg, size_t nbytes, loff_t *offset)
{
    int i = 0;
    char ch;
    uint32_t reg;
    uint32_t value;
    char msg_in[32];
    int len_in = 0;
        
    while( i < nbytes) {
        get_user(ch, &imsg[i]);
        msg_in[len_in] = ch;
        msg_in[len_in+1] = '\0';

        if(msg_in[len_in] == '\n') {
            reg = 0;
            msg_in[len_in] = 0x00;
            value = simple_strtol(msg_in, NULL, 0);
            reg = (reg | value);
            pr_info("Register value to be written:%x\n", reg);
            writel(reg,base+4);
            len_in = 0;
        }
        else {
            len_in++;
        }
        *offset = *offset + 1;
        i++;
    }
    return 0;

}

int apb3_open(struct inode * inodep, struct file * filep)
{
    if(is_open == 1) {
        pr_info("apb3 device already open\n");
        return -EBUSY;
    }
    is_open = 1;
    try_module_get(THIS_MODULE);
    return 0;
}

int apb3_release(struct inode * inodep, struct file * filep)
{
    if(is_open == 0) {
        pr_info("apb3 device wasn't open\n");
        return -EBUSY;
    }
    is_open = 0;
    module_put(THIS_MODULE);
    return 0;
}

struct file_operations fops = {
 	.read   	    = apb3_read,
 	.write  	    = apb3_write,
 	.open   	    = apb3_open,
 	.release	    = apb3_release
};

static int apb3_init(void)
{
	pr_info("Initializing apb3 character driver\n");
    base = ioremap(BASE, SZ_4K);
    dev_valid = alloc_chrdev_region(&devname, 0, 1,"chardriver");
    dev_major = MAJOR(devname);
    pr_info("apb3 device major number:%d\n", dev_major);
    if(dev_valid < 0) {
        pr_info("Failed to get allocation for apb3 device!\n");
        return -EBUSY;
    }
    driver_class=class_create(THIS_MODULE, CLASS_NAME);
    if(driver_class == NULL) {
        pr_info("Failed to create driver class!\n");
        unregister_chrdev_region(devname,1);
        return -EBUSY;
    }
    if(device_create(driver_class, NULL, devname, NULL, "apb") == NULL) {
        pr_info("Failed to create device!\n");
        class_destroy(driver_class);
        unregister_chrdev_region(devname,1);
        return -EBUSY;
    }
    cdev_init(&c_dev, &fops);
    if(cdev_add(&c_dev, devname, 1) == -1) {
        pr_info("Failed to create char device!\n");
        device_destroy(driver_class, devname);
        class_destroy(driver_class);
        unregister_chrdev_region(devname, 1);
        return -EBUSY;
    }
    
	return 0;
}

static void apb3_exit(void)
{
    cdev_del(&c_dev);
    device_destroy(driver_class, devname);
    class_destroy(driver_class);
    unregister_chrdev_region(devname, 1);
	pr_info("Removed apb3 character driver\n");
}

module_init(apb3_init);
module_exit(apb3_exit);
