#include "type.h"
#include "soc.h"
#include "bsp.h"
#include "common.h"
#include "i2c.h"
// #include "io.h"
#include "i2cDemo.h"
#include "dmasg.h"
#include "dmasg_config.h"
#include "isp.h"
#include "apb3_cam.h"
#include "axi4_hw_accel.h"
#include "PiCamDriver.h"
#include "my_def.h"
#include "my_font.h"
#include "PiCamV3Driver.h"
#include <linux/iopoll.h>

#define PICAM_V3        1
#define PICAM_V2        0

#if 0
#define GRAYSCALE_START_ADDR get_CAM_START_ADDR(1)
#define grayscale_array ((volatile uint32_t *)get_CAM_START_PTR(1))

#define CAPTURE_START_ADDR get_CAM_START_ADDR(3)
#define capture_array ((volatile uint32_t *)get_CAM_START_PTR(3))
#endif

volatile struct device *this_dev_ptr;

volatile u32 select_demo_mode = 0;
volatile u8 capture_mode = 0;

struct mutex dma_lock;

/*******************************************************DMA-RELATED FUNCTIONS******************************************************/

// For DMA interrupt
uint32_t hw_accel_mm2s_active;
uint32_t hw_accel_s2mm_active;
uint32_t cam_s2mm_active;
uint32_t display_mm2s_active;

volatile u32 tx_cyc_idx = 1;
volatile u32 rx_cyc_idx = 1;
volatile u8 init_tx_buf = 1;

volatile u32 buf_is_ready = 0;
volatile u32 lay0_is_ready = 0;

volatile u8 lay0_beg_row = 0;
volatile u8 lay0_beg_col = 0;

volatile u8 tx_buf_idx = 0;
volatile u8 rx_buf_idx = 2;
struct task_struct *evsoc_thread_ptr_rx = NULL;

#define  TITLE_HIGH  18
uint32_t title_msg_buf[FRAME_WIDTH * TITLE_HIGH];
volatile u32 title_is_ready = 0;

u32 get_tx_buf(void)
{
   return get_CAM_START_ADDR(tx_buf_idx);
}

u32 get_rx_buf(void)
{
   return get_CAM_START_ADDR(rx_buf_idx);
}

u32 *get_tx_ptr(void)
{
   return get_CAM_START_PTR(tx_buf_idx);
}

u32 *get_rx_ptr(void)
{
   return get_CAM_START_PTR(rx_buf_idx);
}



static int do_rx_proc(int pMode)
{
   u32 status = 0;
   u32 lBuf = get_rx_buf();
   u32 reg;
   int ret;

   // SELECT RGB or grayscale output from camera pre-processing block.
   if (pMode == 2 || pMode == 3)
   {
      EXAMPLE_APB3_REGW(EXAMPLE_APB3_SLV, EXAMPLE_APB3_SLV_REG3_OFFSET, 0x00000001); // grayscale
   }
   else
   {
      EXAMPLE_APB3_REGW(EXAMPLE_APB3_SLV, EXAMPLE_APB3_SLV_REG3_OFFSET, 0x00000000); // RGB
   }

   // Trigger camera DMA
   dmasg_input_stream(DMASG_BASE, DMASG_CAM_S2MM_CHANNEL, DMASG_CAM_S2MM_PORT, 1, 0);
   dmasg_output_memory(DMASG_BASE, DMASG_CAM_S2MM_CHANNEL, lBuf, 16);
   dmasg_direct_start(DMASG_BASE, DMASG_CAM_S2MM_CHANNEL, (FRAME_WIDTH * FRAME_HEIGHT) * 4, 0);

   // Indicate start of S2MM DMA to camera building block via APB3 slave
   EXAMPLE_APB3_REGW(EXAMPLE_APB3_SLV, EXAMPLE_APB3_SLV_REG4_OFFSET, 0x00000001);
   EXAMPLE_APB3_REGW(EXAMPLE_APB3_SLV, EXAMPLE_APB3_SLV_REG4_OFFSET, 0x00000000);

   // Trigger storage of one captured frame via APB3 slave
   EXAMPLE_APB3_REGW(EXAMPLE_APB3_SLV, EXAMPLE_APB3_SLV_REG2_OFFSET, 0x00000001);
   EXAMPLE_APB3_REGW(EXAMPLE_APB3_SLV, EXAMPLE_APB3_SLV_REG2_OFFSET, 0x00000000);

   // Wait for DMA transfer completion
   while (dmasg_busy(DMASG_BASE, DMASG_CAM_S2MM_CHANNEL))
      ;

   if (lay0_is_ready > 0)
   {
      int lTmpRow;
      int lTmpCol;
      u32 *lFromBuf = layer0_array;
      u32 *lToBuf = get_rx_ptr();
      int tmp_pos;
      for (lTmpRow = 0; lTmpRow < FRAME_HEIGHT; lTmpRow++)
      {
         for (lTmpCol = 0; lTmpCol < FRAME_WIDTH; lTmpCol++)
         {
            tmp_pos = lTmpRow * FRAME_WIDTH + lTmpCol;
            if ((lFromBuf[tmp_pos] & 0xFF000000) != 0)
            {
               lToBuf[tmp_pos] = (lFromBuf[tmp_pos] & 0x00FFFFFF);
            }
         }
      }

      flush_data_cache();
   }

   /*******************************************************RISC-V Processing***********************************************************/

   if (capture_mode == CMD_SET_CAPTURE_FULL)
   {
      memcpy((void *)capture_array, (void *)get_rx_ptr(), (FRAME_WIDTH * FRAME_HEIGHT) * 4);
      send_signal(SIG_TYPE_CAPTURE_DONE, capture_mode); 
      capture_mode = 0;

   }
   else if (capture_mode == CMD_SET_CAPTURE_PART)
   {
      // memcpy((void *)capture_array, (void *)get_rx_ptr(), (FRAME_WIDTH * FRAME_HEIGHT) * 4);

      int i, j;
      int lTo, lFrom;
      uint32_t * lToPtr = capture_array;
      uint32_t * lFrPtr = get_rx_ptr();

      for ( i = 0; i < (FRAME_HEIGHT / 2) ; ++i)
      {
         for ( j = 0; j < (FRAME_WIDTH / 2); ++j)
         {
            lTo   = i * (FRAME_WIDTH / 2) + j;
            lFrom = i * 2 * FRAME_WIDTH + j * 2;
            lToPtr[lTo] = lFrPtr[lFrom];
         }
      }

      send_signal(SIG_TYPE_CAPTURE_DONE, capture_mode); 
      capture_mode = 0;
   }

   if (title_is_ready > 0)
   {
      int lTmpRow;
      int lTmpCol;
      u32 *lFromBuf = title_msg_buf;
      u32 *lToBuf = get_rx_ptr();
      int tmp_pos, tmp_2_pos;
      for (lTmpRow = 0; lTmpRow < TITLE_HIGH; lTmpRow++)
      {
         for (lTmpCol = 0; lTmpCol < FRAME_WIDTH; lTmpCol++)
         {
            tmp_pos = lTmpRow * FRAME_WIDTH + lTmpCol;
            if ((lFromBuf[tmp_pos] & 0xFF000000) != 0)
            {
               tmp_2_pos = (lTmpRow + 3) * FRAME_WIDTH + (lTmpCol + 3);
               lToBuf[tmp_2_pos] = (lFromBuf[tmp_pos] & 0x00FFFFFF);
            }
         }
      }

      flush_data_cache();
   }

   if (pMode == 1)
   {
      u32 *lBufPtr = get_rx_ptr();
      rgb2grayscale(lBufPtr, grayscale_array, FRAME_WIDTH, FRAME_HEIGHT);
   }

   if (pMode > 0)
   {

#ifdef EXAMPLE_AXI4_SLV
      // SET Sobel edge detection threshold via AXI4 slave
      write_u32(100, EXAMPLE_AXI4_SLV + EXAMPLE_AXI4_SLV_REG0_OFFSET); // Default value 100; Range 0 to 255

      // SELECT HW accelerator mode - Make sure match with DMA transfer length setting
      if (pMode == 3)
      {
         write_u32(0x00000001, EXAMPLE_AXI4_SLV + EXAMPLE_AXI4_SLV_REG1_OFFSET); // 2'd1: Sobel+Dilation
      }
      else
      {
         write_u32(0x00000000, EXAMPLE_AXI4_SLV + EXAMPLE_AXI4_SLV_REG1_OFFSET); // 2'd0: Sobel only
      }
      // write_u32(0x00000002, EXAMPLE_AXI4_SLV+EXAMPLE_AXI4_SLV_REG1_OFFSET);   //2'd2: Sobel+Erosion

      // Trigger HW accel MM2S DMA
      // SELECT start address of DMA input to HW accel block
      if (pMode == 2 || pMode == 3)
      {
         dmasg_input_memory(DMASG_BASE, DMASG_HW_ACCEL_MM2S_CHANNEL, lBuf, 16); // Camera pre-processing block performs HW RGB2grayscale conversion
      }
      else
      {
         dmasg_input_memory(DMASG_BASE, DMASG_HW_ACCEL_MM2S_CHANNEL, GRAYSCALE_START_ADDR, 16); // RISC-V performs SW RGB2grayscale conversion
      }

      dmasg_output_stream(DMASG_BASE, DMASG_HW_ACCEL_MM2S_CHANNEL, DMASG_HW_ACCEL_MM2S_PORT, 0, 0, 1);

      // SELECT dma transfer length - Make sure match with HW accelerator mode selection
      // Additonal data is required to be fed for line buffer(s) data flushing
      if (pMode == 3)
      {
         dmasg_direct_start(DMASG_BASE, DMASG_HW_ACCEL_MM2S_CHANNEL, ((FRAME_WIDTH * FRAME_HEIGHT) + (2 * FRAME_WIDTH + 2)) * 4, 0); // Sobel + Dilation/Erosion
      }
      else
      {
         dmasg_direct_start(DMASG_BASE, DMASG_HW_ACCEL_MM2S_CHANNEL, ((FRAME_WIDTH * FRAME_HEIGHT) + (FRAME_WIDTH + 1)) * 4, 0); // Sobel only
      }

      // Trigger HW accel S2MM DMA
      dmasg_input_stream(DMASG_BASE, DMASG_HW_ACCEL_S2MM_CHANNEL, DMASG_HW_ACCEL_S2MM_PORT, 1, 0);
      dmasg_output_memory(DMASG_BASE, DMASG_HW_ACCEL_S2MM_CHANNEL, lBuf, 16);
      dmasg_direct_start(DMASG_BASE, DMASG_HW_ACCEL_S2MM_CHANNEL, (FRAME_WIDTH * FRAME_HEIGHT) * 4, 0);

      // Indicate start of S2MM DMA to HW accel building block via APB3 slave
      write_u32(0x00000001, EXAMPLE_AXI4_SLV + EXAMPLE_AXI4_SLV_REG2_OFFSET);
      write_u32(0x00000000, EXAMPLE_AXI4_SLV + EXAMPLE_AXI4_SLV_REG2_OFFSET);

      // Wait for DMA transfer completion
      while (dmasg_busy(DMASG_BASE, DMASG_HW_ACCEL_MM2S_CHANNEL) || dmasg_busy(DMASG_BASE, DMASG_HW_ACCEL_S2MM_CHANNEL))
#endif
   }

   {
      u8 lTmpIdx1 = tx_buf_idx;
      u8 lTmpIdx2 = rx_buf_idx;
      tx_buf_idx = lTmpIdx2;
      rx_buf_idx = lTmpIdx1;
   }

   return 1;
}

int evsoc_thread_fun_rx(void *pv)
{
   u32 lBuf;
   u32 this_cnt = 0;
   while (buf_is_ready == 0)
   {
      msleep(1000);
   }

   while (!kthread_should_stop())
   {
      if (init_tx_buf)
      {
         int x, y;
         int pos;
         u32 *lInitBuf = layer0_array;

         tx_buf_idx = 0;

         for (y = 0; y < FRAME_HEIGHT; y++)
         {
            for (x = 0; x < FRAME_WIDTH; x++)
            {
               pos = y * FRAME_WIDTH + x;
               lInitBuf[pos] = 0;
               if (pos < (FRAME_WIDTH * TITLE_HIGH))
               {
                  title_msg_buf[pos] = 0;
               }

               if ((x < 3 && y < 3) || (x >= FRAME_WIDTH - 3 && y < 3) || (x < 3 && y >= FRAME_HEIGHT - 3) || (x >= FRAME_WIDTH - 3 && y >= FRAME_HEIGHT - 3))
               {
                  // 0x000000FF; //RED  0x0000FF00; //GREEN x00FF0000; //BLUE
                  set_CAM_START_ADDR(0, pos, 0x000000FF);
               }
               else if (x < (FRAME_WIDTH / 4))
               {
                  set_CAM_START_ADDR(0, pos, 0x0000FF00);
               }
               else if (x < (FRAME_WIDTH / 4 * 2))
               {
                  set_CAM_START_ADDR(0, pos, 0x00FF0000);
               }
               else if (x < (FRAME_WIDTH / 4 * 3))
               {
                  set_CAM_START_ADDR(0, pos, 0x000000FF);
               }
               else
               {
                  set_CAM_START_ADDR(0, pos, 0x00FF0000);
               }
            }
         }
         msleep(500);
         continue;
      }

      if (this_cnt++ > 1000)
      {
         this_cnt = 0;
         if (select_demo_mode > 3)
         {
            select_demo_mode = 0;
         }

         // send_signal(SIGUSR1);
      }

      if (do_rx_proc(select_demo_mode))
      {
         msleep(50);
      }
      else
      {
         msleep(500);
      }
   }
   return 0;
}

static irqreturn_t dma_tx_irq(int irq, void *_ndev)
{
   u32 lBuf = get_tx_buf();

   dmasg_input_memory(DMASG_BASE, DMASG_DISPLAY_MM2S_CHANNEL, lBuf, 16);
   dmasg_output_stream(DMASG_BASE, DMASG_DISPLAY_MM2S_CHANNEL, DMASG_DISPLAY_MM2S_PORT, 0, 0, 1);
   dmasg_interrupt_config(DMASG_BASE, DMASG_DISPLAY_MM2S_CHANNEL, DMASG_CHANNEL_INTERRUPT_CHANNEL_COMPLETION_MASK);
   dmasg_direct_start(DMASG_BASE, DMASG_DISPLAY_MM2S_CHANNEL, (FRAME_WIDTH * FRAME_HEIGHT) * 4, 0);

   return IRQ_HANDLED;
}


static irqreturn_t key_act_irq(int irq, void *_ndev)
{
   send_signal(SIG_TYPE_KEY_ACT, irq); 
   return IRQ_HANDLED;
}

void start_tx(void)
{
   init_tx_buf = 1;
   Set_MipiRst(1);
   Set_MipiRst(0);

   // SELECT start address of to be displayed data accordingly
   dmasg_input_memory(DMASG_BASE, DMASG_DISPLAY_MM2S_CHANNEL, get_tx_buf(), 16);

   dmasg_output_stream(DMASG_BASE, DMASG_DISPLAY_MM2S_CHANNEL, DMASG_DISPLAY_MM2S_PORT, 0, 0, 1);
   dmasg_interrupt_config(DMASG_BASE, DMASG_DISPLAY_MM2S_CHANNEL, DMASG_CHANNEL_INTERRUPT_CHANNEL_COMPLETION_MASK);
   dmasg_direct_start(DMASG_BASE, DMASG_DISPLAY_MM2S_CHANNEL, (FRAME_WIDTH * FRAME_HEIGHT) * 4, 0);
   display_mm2s_active = 1; // Display always active

   msDelay(5000); // Display test content for 5 seconds
   init_tx_buf = 0;
}

int evsoc_mmap(struct file *filp, struct vm_area_struct *vma)
{
   // uint32_t lVirtAdd = 1000;
   vma->vm_flags |= VM_SHARED;
   vma->vm_flags |= VM_READ;

   if (remap_pfn_range(vma, vma->vm_start, CAPTURE_START_ADDR >> PAGE_SHIFT, vma->vm_end - vma->vm_start, vma->vm_page_prot))
   {
      return -EAGAIN;
   }
   return 0;
}

volatile int tx_irq_num;
volatile int key_irq_num;

/****************************************************************MAIN**************************************************************/
void main_init(struct device *dev)
{
   /**************************************************SETUP PICAM & HDMI DISPLAY***************************************************/
   // u32 rdata;
   int y;
   int x;
   int ret;
   int lIntr = 17;

   this_dev_ptr = dev;

   tx_buf_idx = 0;
   rx_buf_idx = 2;

   mutex_init(&dma_lock);
   {
      struct device_node *nd = of_find_node_by_path("/evsoc");
      if (nd)
      {
         tx_irq_num = irq_of_parse_and_map(nd, 0);
         if (tx_irq_num)
         {
		 /*
            if (request_irq(tx_irq_num, dma_tx_irq, 0, "evsoc", NULL))
            {
               printk("%s, failed to request evsoc irq(%d)\n", __func__, tx_irq_num);
            }
	    */
	    ret = devm_request_irq(dev, tx_irq_num, dma_tx_irq, 0, "evsoc", NULL);
	    if (ret) {
	    	printk("%s, failed to request evsoc irq(%d)\n", __func__, tx_irq_num);
	    }

         }

         key_irq_num = irq_of_parse_and_map(nd, 1);
         if (key_irq_num)
         {
		 /*
            if (request_irq(key_irq_num, key_act_irq, 0, "evsoc_key", NULL))
            {
               printk("%s, failed to request evsoc irq(%d)\n", __func__, key_irq_num);
            }
	    */
	    ret = devm_request_irq(dev, key_irq_num, key_act_irq, 0, "evsoc_key", NULL);
	    if (ret) {
	    	printk("%s, failed to request evsoc irq(%d)\n", __func__, key_irq_num);
	    }
         }

      }
   }
   
   pr_info("mipi init ...\n");
   Set_MipiRst(1);
   Set_MipiRst(0);

   uart_writeStr(BSP_UART_TERMINAL, "\nHello Efinix Edge Vision SoC Demo!!\n\n"); // Mode selection using SW4 switch.

   mipi_i2c_init();
#ifdef PICAM_V2
   PiCam_init();
#endif
#ifdef PICAM_V3
   PiCamV3_Init();
#endif

   uart_writeStr(BSP_UART_TERMINAL, "Done !!\n\n");

#ifdef PICAM_V2
   Set_RGBGain(1, 5, 3, 4); // SET camera pre-processing RGB gain value
#endif
#ifdef PICAM_V3
   Set_RGBGain(1, 5, 3, 7);
#endif
   /**********************************************************SETUP DMA***********************************************************/

   uart_writeStr(BSP_UART_TERMINAL, "Init DMA.....");

   pr_info("dma init ...\n");
   dma_init();
#ifdef EXAMPLE_AXI4_SLV
   dmasg_priority(DMASG_BASE, DMASG_HW_ACCEL_MM2S_CHANNEL, 0, 0);
   dmasg_priority(DMASG_BASE, DMASG_HW_ACCEL_S2MM_CHANNEL, 0, 0);
#endif
   dmasg_priority(DMASG_BASE, DMASG_DISPLAY_MM2S_CHANNEL, 0, 0);
   dmasg_priority(DMASG_BASE, DMASG_CAM_S2MM_CHANNEL, 0, 0);

   uart_writeStr(BSP_UART_TERMINAL, "Done !!\n\n");

#ifdef PICAM_V3
   PiCamV3_StartStreaming();
#endif

   /*******************************************************Trigger Display********************************************************/

   // To check display functionality
   uart_writeStr(BSP_UART_TERMINAL, "Initialize test display content..\n");

   // struct task_struct *evsoc_thread_ptr_rx = NULL;
   evsoc_thread_ptr_rx = kthread_create(evsoc_thread_fun_rx, NULL, "Evsoc Rx");
   if (evsoc_thread_ptr_rx)
   {
      wake_up_process(evsoc_thread_ptr_rx);
   }
   else
   {
      printk(KERN_ERR "Cannot create kthread\n");
   }

   buf_is_ready = 1;
   start_tx();
}

void main_exit(struct device *dev)
{
   if (evsoc_thread_ptr_rx)
   {
      kthread_stop(evsoc_thread_ptr_rx);
   }
 
#ifdef PICAM_V3
   PiCamV3_StopStreaming();
#endif
   Set_MipiRst(1);
   Set_MipiRst(0);

}

bool main_proc(unsigned int pCmd, unsigned long pArg, u8 *pBuf)
{
   if (pCmd == CMD_SET_MODE)
   {
      if (pArg == 1)
      {
         select_demo_mode = 2;
      }
      else if (pArg == 2)
      {
         select_demo_mode = 3;
      }
      else
      {
         select_demo_mode = 0;
      }
   }
   else if (pCmd == CMD_SET_CAM_BINNING)
   {
      int lVal = pArg;
      if (lVal >= 3)
      {
         return false;
      }
      PiCam_SetBinningMode(lVal, lVal);
   }
   else if (pCmd == CMD_SET_CAPTURE)
   {
      capture_mode = pArg;
   }
   else if (pCmd == CMD_SET_PID)
   {
      set_pid(pArg);
   }
   else if (pCmd == CMD_SET_LAYER)
   {
      lay0_is_ready = pArg;

      if (pArg == 3)
      {
         u32 *lFromBuf = layer0_array;
         int lTmpI;
         lay0_is_ready = 0;
         for (lTmpI = 0; lTmpI < (FRAME_WIDTH * FRAME_HEIGHT); lTmpI++)
         {
            lFromBuf[lTmpI] = 0;
         }
      }
   }
   else if (pCmd == CMD_SET_CHAR)
   {
      u32 *lFromBuf;
      unsigned char lCh = pArg & 0xff;
      unsigned char lType = (pArg >> 8) & 0xff;
      unsigned char lPosY = (pArg >> 16) & 0xff;
      unsigned char lPosX = (pArg >> 24) & 0xff;

      int lY, lX;
      u32 lColor = 0x0100FF00;

      unsigned int lFontY = lPosY * 9;
      unsigned int lFontX = lPosX * 9;
      int tmp_pos;
      ////
      lay0_beg_row = lPosY;
      lay0_beg_col = lPosX;
      
      // 0 == green
      if (lType == 1)
      {
         lColor = 0x01FF0000;
      }
      else if (lType == 2)
      {
         lColor = 0x01FFFFFF;
      }
      else if (lType == 3)
      {
         lColor = 0x01000000;
      }

      ////
      lFromBuf = layer0_array;
      for (lY = 0; lY < 8; lY++)
      {
         for (lX = 0; lX < 8; lX++)
         {
            tmp_pos = (lFontY + lY) * FRAME_WIDTH + lFontX + lX;
            if (tmp_pos >= (FRAME_WIDTH * FRAME_HEIGHT))
            {
               continue;
            }

            if (is_ch_show(lCh, lY, lX, 1))
            {
               lFromBuf[tmp_pos] = lColor;
            }
            else
            {
               lFromBuf[tmp_pos] = 0;
            }
         }
      }
   }
   else if (pCmd == CMD_CLR_MSG)
   {
      int lTmpI;
      title_is_ready = pArg;
      if (pArg ==  3) 
      {
         for (lTmpI = 0; lTmpI < (FRAME_WIDTH * TITLE_HIGH); lTmpI++)
         {
            title_msg_buf[lTmpI] = 0;
         }
         title_is_ready = 0;
      }
   }
   else if (pCmd == CMD_SET_MSG)
   {
      /* this is original
      u32 *lFromBuf;
      unsigned char lCh = pArg & 0xff;
      unsigned char lType = (pArg >> 8) & 0xff;
      unsigned char lPosY = (pArg >> 16) & 0xff;
      unsigned char lPosX = (pArg >> 24) & 0xff;
      lPosY = 0;

      int lY, lX;
      u32 lColor = 0x0100FF00;

      unsigned int lFontY = lPosY * 16;
      unsigned int lFontX = lPosX * 16;
      int tmp_pos;
      */
      u32 *lFromBuf;
      unsigned char lCh = pArg & 0xff;
      unsigned char lType = (pArg >> 8) & 0xff;
      unsigned char lPosY = (pArg >> 16) & 0xff;
      unsigned char lPosX = (pArg >> 24) & 0xff;

      int lY, lX;
      u32 lColor = 0x0100FF00;

      unsigned int lFontY = lPosY * 16;
      unsigned int lFontX = lPosX * 16;
      int tmp_pos;
      lPosY = 0;
      ////
      
      // 0 == green
      if (lType == 1)
      {
         lColor = 0x01FF0000;
      }
      else if (lType == 2)
      {
         lColor = 0x01FFFFFF;
      }
      else if (lType == 3)
      {
         lColor = 0x01000000;
      }

      ////
      lFromBuf = layer0_array;
      for (lY = 0; lY < 16; lY++)
      {
         for (lX = 0; lX < 16; lX++)
         {
            tmp_pos = (lFontY + lY) * FRAME_WIDTH + lFontX + lX;
            if (tmp_pos >= (FRAME_WIDTH * TITLE_HIGH))
            {
               continue;
            }

            if (is_ch_show(lCh, lY, lX, 2))
            {
               title_msg_buf[tmp_pos] = lColor;
            }
            else
            {
               title_msg_buf[tmp_pos] = 0;
            }
         }
      }
   }   
   else
   {
      return false;
   }

   return true;
}
