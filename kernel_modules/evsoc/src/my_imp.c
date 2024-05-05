#include <linux/io.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>

#include "my_def.h"

#define ADR_I2C_CTRL_MIPI   SYSTEM_I2C_0_IO_CTRL
#define ADR_EXAMPLE_APB_SLV IO_APB_SLAVE_1_INPUT
#define ADR_DMASG_BASE      IO_APB_SLAVE_2_INPUT
#define ADR_EXAMPLE_AXI_SLV SYSTEM_AXI_A_BMB

static void __iomem *i2c_base = NULL;
static void __iomem *example_apb_base = NULL;
static void __iomem *dmasg_base = NULL;
static void __iomem *example_axi_base = NULL;

// struct task_struct * kthread_create (int (* threadfn(void *data), void *data, const char namefmt[], ...);

#define MAX_DMA_BUF_NUM 4
dma_addr_t cam_dma_handle[MAX_DMA_BUF_NUM];
uint32_t * cam_dma_cpu_arry[MAX_DMA_BUF_NUM] = {NULL};
int cam_dma_size = 1024 * 1024 * 4;

struct task_struct *evsoc_thread = NULL; 

void my_init(struct device *dev)
{
    int lTmpI;


    i2c_base = ioremap(ADR_I2C_CTRL_MIPI, SZ_4K);
    example_apb_base = ioremap(ADR_EXAMPLE_APB_SLV, SZ_64K);
    dmasg_base = ioremap(ADR_DMASG_BASE, SZ_64K);
    example_axi_base = ioremap(ADR_EXAMPLE_AXI_SLV, SZ_64K);
    
    pr_info("resource init ...\n");
    for (lTmpI = 0; lTmpI < MAX_DMA_BUF_NUM; lTmpI++)
    {
        cam_dma_cpu_arry[lTmpI] = kmalloc(cam_dma_size, GFP_KERNEL | GFP_DMA);    
        if (cam_dma_cpu_arry[lTmpI])
        {
            cam_dma_handle[lTmpI] = virt_to_phys(cam_dma_cpu_arry[lTmpI]);
        }
        else
        {
            pr_info("kmalloc Fail %d\n", lTmpI);
        }   
    }
    // pr_info("my_init done\n");
}

u32 get_CAM_START_ADDR(int pIdx)
{
    if (pIdx >= MAX_DMA_BUF_NUM)
    {
        printk("Error Idx\n");
        return 0;
    }
    else
    {
        return cam_dma_handle[pIdx];
    }
}

uint32_t * get_CAM_START_PTR(int pIdx)
{
    if (pIdx >= MAX_DMA_BUF_NUM)
    {
        printk("Error Idx\n");
        return 0;
    }
    else
    {
        return cam_dma_cpu_arry[pIdx];
    }
}

void set_CAM_START_ADDR(int pIdx, u32 pAddr, u32 pVal)
{
    if (pIdx >= MAX_DMA_BUF_NUM)
    {
        return;
    }
    if ((pAddr >= cam_dma_size) ||  !cam_dma_cpu_arry[pIdx])
    {
        return;
    }

    cam_dma_cpu_arry[pIdx][pAddr] = pVal;
}

void set_evsoc_thread(struct task_struct * pPtr)
{
    evsoc_thread = pPtr;
}

void my_exit(struct device *dev)
{
    int lTmpI;
    for (lTmpI = 0; lTmpI < MAX_DMA_BUF_NUM; lTmpI++)
    {
        kfree(cam_dma_cpu_arry[lTmpI]);
    }  

    iounmap(i2c_base);
    iounmap(example_apb_base);
    iounmap(dmasg_base);
    iounmap(example_axi_base);
}

#define my_addr_pt(_addr) (  \
    ((_addr >= ADR_I2C_CTRL_MIPI) && (_addr < (ADR_I2C_CTRL_MIPI + SZ_4K))) ? (i2c_base + (_addr - ADR_I2C_CTRL_MIPI)): \
    ((_addr >= ADR_EXAMPLE_APB_SLV) && (_addr < (ADR_EXAMPLE_APB_SLV + SZ_64K))) ? (example_apb_base + (_addr - ADR_EXAMPLE_APB_SLV)) : \
    ((_addr >= ADR_EXAMPLE_AXI_SLV) && (_addr < (ADR_EXAMPLE_AXI_SLV + SZ_64K))) ? (example_axi_base + (_addr - ADR_EXAMPLE_AXI_SLV)) : \
    ((_addr >= ADR_DMASG_BASE) && (_addr < (ADR_DMASG_BASE + SZ_64K))) ? (dmasg_base + (_addr - ADR_DMASG_BASE)): 0 \
    )

u32 read_u32(u32 address)
{
#ifdef SIM_MEM
    return 1;
#else
    return ioread32(my_addr_pt(address));
#endif
}

void write_u32(u32 data, u32 address)
{
#ifdef SIM_MEM
#else
    iowrite32(data, my_addr_pt(address));
#endif
}

u16 read_u16(u32 address)
{

#ifdef SIM_MEM
    return 1;
#else

    return ioread16(my_addr_pt(address));
#endif
}

void write_u16(u16 data, u32 address)
{
#ifdef SIM_MEM
#else
    iowrite16(data, my_addr_pt(address));
#endif
}

u8 read_u8(u32 address)
{
#ifdef SIM_MEM
    return 1;
#else
    return ioread8(my_addr_pt(address));
#endif
}

void write_u8(u8 data, u32 address)
{
#ifdef SIM_MEM
#else
    iowrite8(data, my_addr_pt(address));
#endif
}

void write_u32_ad(u32 address, u32 data)
{
#ifdef SIM_MEM
#else
    iowrite32(data, my_addr_pt(address));
#endif
}


static int g_pid = 0;

void set_pid(int pPid)
{
    g_pid = pPid;
}

void send_signal(unsigned char pType, int pVal)
{
	int ret;
    int sig_no = SIGUSR1;
	struct kernel_siginfo info;
	struct task_struct *my_task = NULL;
	if (0 == g_pid)
	{
		// 说明应用程序没有设置自己的 PID
	    // printk("pid[%d] is not valid! \n", g_pid);
	    return;
	}

	// printk("send signal %d to pid %d \n", sig_no, g_pid);

	// 构造信号结构体
	memset(&info, 0, sizeof(struct kernel_siginfo));
	info.si_signo = sig_no;
	info.si_errno = 100;
	info.si_code = 200;
    info.si_value.sival_int = (pType &0xff) | ((pVal << 8) & 0xffffff00);


	rcu_read_lock();
	my_task = pid_task(find_vpid(g_pid), PIDTYPE_PID);
	rcu_read_unlock();

	if (my_task == NULL)
	{
	    printk("get pid_task failed! \n");
	    return;
	}

	// 发送信号
	ret = send_sig_info(sig_no, &info, my_task);
	if (ret < 0) 
	{
	       printk("send signal failed! \n");
	}
}

