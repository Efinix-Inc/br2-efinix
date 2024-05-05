#ifndef _MY_DEF_H_
#define _MY_DEF_H_

#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-variable"

#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/ioctl.h>

#include "soc.h"
#include "comm_def.h"



// #define bsp_uDelay(usec) clint_uDelay(usec, SYSTEM_CLINT_HZ, SYSTEM_CLINT_CTRL)
#define bsp_uDelay(usec) udelay(usec)
#define msDelay(msec) msleep(msec)
#define assert(pFun) pFun
#define uart_writeStr(reg, str)

#define PR_TRACE pr_info("%s, %d \n", __FILE__, __LINE__);
// static void uart_writeStr(unsigned int reg, const char* str){
//    while(*str) uart_write(reg, *str++);
// }
u32 get_CAM_START_ADDR(int pIdx);
uint32_t * get_CAM_START_PTR(int pIdx);

void set_CAM_START_ADDR(int pIdx, u32 pAddr, u32 pVal);
// void set_evsoc_thread(struct task_struct *pPtr);

void main_init(struct device *dev);
void main_exit(struct device *dev);
bool main_proc(unsigned int pCmd, unsigned long pArg, u8 * pBuf);
int evsoc_mmap(struct file *filp, struct vm_area_struct *vma);

void my_exit(struct device *dev);
void my_init(struct device *dev);


void set_pid(int pPid);
void send_signal(unsigned char pType, int pVal);

/////////

u32 read_u32(u32 address);
void write_u32(u32 data, u32 address);
u16 read_u16(u32 address);
void write_u16(u16 data, u32 address);
u8 read_u8(u32 address);
void write_u8(u8 data, u32 address);
void write_u32_ad(u32 address, u32 data);
#define writeReg_u32(name, offset)              \
    static inline void name(u32 reg, u32 value) \
    {                                           \
        write_u32(value, reg + offset);         \
    }

#define readReg_u32(name, offset)      \
    static inline u32 name(u32 reg)    \
    {                                  \
        return read_u32(reg + offset); \
    }


#define GRAYSCALE_START_ADDR get_CAM_START_ADDR(1)
#define grayscale_array      ((uint32_t *)get_CAM_START_PTR(1))

#define LAYER0_START_ADDR get_CAM_START_ADDR(1)
#define layer0_array         ((uint32_t *)get_CAM_START_PTR(1))

#define CAPTURE_START_ADDR get_CAM_START_ADDR(3)
#define capture_array        ((uint32_t *)get_CAM_START_PTR(3))

#define GPIO_BASE SYSTEM_GPIO_0_IO_CTRL

#endif