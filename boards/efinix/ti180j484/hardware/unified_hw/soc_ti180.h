#ifndef SOC_H
#define SOC_H
#define SYSTEM_PLIC_SYSTEM_CORES_0_EXTERNAL_INTERRUPT 0
#define SYSTEM_PLIC_SYSTEM_CORES_0_EXTERNAL_SUPERVISOR_INTERRUPT 1
#define SYSTEM_PLIC_SYSTEM_CORES_1_EXTERNAL_INTERRUPT 2
#define SYSTEM_PLIC_SYSTEM_CORES_1_EXTERNAL_SUPERVISOR_INTERRUPT 3
#define SYSTEM_PLIC_USER_INTERRUPT_B_INTERRUPT 17
#define SYSTEM_PLIC_USER_INTERRUPT_A_INTERRUPT 16
#define SYSTEM_PLIC_USER_INTERRUPT_D_INTERRUPT 23
#define SYSTEM_PLIC_USER_INTERRUPT_C_INTERRUPT 22
#define SYSTEM_PLIC_SYSTEM_AXI_A_INTERRUPT 30
#define SYSTEM_PLIC_SYSTEM_UART_0_IO_INTERRUPT 1
#define SYSTEM_PLIC_SYSTEM_SPI_0_IO_INTERRUPT 4
#define SYSTEM_PLIC_SYSTEM_SPI_1_IO_INTERRUPT 5
#define SYSTEM_PLIC_SYSTEM_I2C_1_IO_INTERRUPT 9
#define SYSTEM_PLIC_SYSTEM_I2C_0_IO_INTERRUPT 8
#define SYSTEM_PLIC_SYSTEM_I2C_2_IO_INTERRUPT 10
#define SYSTEM_PLIC_SYSTEM_USER_TIMER_0_INTERRUPTS_0 19
#define SYSTEM_PLIC_SYSTEM_USER_TIMER_1_INTERRUPTS_0 20
#define SYSTEM_PLIC_SYSTEM_GPIO_0_IO_INTERRUPTS_0 12
#define SYSTEM_PLIC_SYSTEM_GPIO_0_IO_INTERRUPTS_1 13
#define SYSTEM_RISCV_ISA_RV32I 1
#define SYSTEM_RISCV_ISA_EXT_M 1
#define SYSTEM_RISCV_ISA_EXT_A 1
#define SYSTEM_RISCV_ISA_EXT_C 1
#define SYSTEM_RISCV_ISA_EXT_F 1
#define SYSTEM_RISCV_ISA_EXT_D 1
#define SYSTEM_RISCV_ISA_EXT_ZICSR 1
#define SYSTEM_RISCV_ISA_EXT_ZIFENCE 1
#define SYSTEM_CLINT_HZ 200000000
#define SYSTEM_RAM_A_SIZE 2048
#define SYSTEM_UART_0_IO_PARAMETER_UART_CTRL_CONFIG_DATA_WIDTH_MAX 8
#define SYSTEM_UART_0_IO_PARAMETER_UART_CTRL_CONFIG_CLOCK_DIVIDER_WIDTH 20
#define SYSTEM_UART_0_IO_PARAMETER_UART_CTRL_CONFIG_PRE_SAMPLING_SIZE 1
#define SYSTEM_UART_0_IO_PARAMETER_UART_CTRL_CONFIG_SAMPLING_SIZE 5
#define SYSTEM_UART_0_IO_PARAMETER_UART_CTRL_CONFIG_POST_SAMPLING_SIZE 2
#define SYSTEM_UART_0_IO_PARAMETER_UART_CTRL_CONFIG_CTS_GEN 0
#define SYSTEM_UART_0_IO_PARAMETER_UART_CTRL_CONFIG_RTS_GEN 0
#define SYSTEM_UART_0_IO_PARAMETER_UART_CTRL_CONFIG_RX_SAMPLE_PER_BIT 8
#define SYSTEM_UART_0_IO_PARAMETER_INIT_CONFIG_BAUDRATE 115200
#define SYSTEM_UART_0_IO_PARAMETER_INIT_CONFIG_DATA_LENGTH 7
#define SYSTEM_UART_0_IO_PARAMETER_INIT_CONFIG_PARITY NONE
#define SYSTEM_UART_0_IO_PARAMETER_INIT_CONFIG_STOP ONE
#define SYSTEM_UART_0_IO_PARAMETER_BUS_CAN_WRITE_CLOCK_DIVIDER_CONFIG 1
#define SYSTEM_UART_0_IO_PARAMETER_BUS_CAN_WRITE_FRAME_CONFIG 1
#define SYSTEM_UART_0_IO_PARAMETER_TX_FIFO_DEPTH 128
#define SYSTEM_UART_0_IO_PARAMETER_RX_FIFO_DEPTH 128
#define SYSTEM_CORES_0_CFU 1
#define SYSTEM_CORES_0_FPU 1
#define SYSTEM_CORES_0_MMU 1
#define SYSTEM_CORES_0_ICACHE_WAYS 8
#define SYSTEM_CORES_0_ICACHE_SIZE 32768
#define SYSTEM_CORES_0_BYTES_PER_LINE 64
#define SYSTEM_CORES_0_DCACHE_WAYS 8
#define SYSTEM_CORES_0_DCACHE_SIZE 32768
#define SYSTEM_CORES_0_BYTES_PER_LINE 64
#define SYSTEM_CORES_0_SUPERVISOR 1
#define SYSTEM_CORES_1_CFU 1
#define SYSTEM_CORES_1_FPU 1
#define SYSTEM_CORES_1_MMU 1
#define SYSTEM_CORES_1_ICACHE_WAYS 8
#define SYSTEM_CORES_1_ICACHE_SIZE 32768
#define SYSTEM_CORES_1_BYTES_PER_LINE 64
#define SYSTEM_CORES_1_DCACHE_WAYS 8
#define SYSTEM_CORES_1_DCACHE_SIZE 32768
#define SYSTEM_CORES_1_BYTES_PER_LINE 64
#define SYSTEM_CORES_1_SUPERVISOR 1
#define SYSTEM_BRIDGE_BMB 0x0
#define SYSTEM_RAM_A_CTRL 0xf9000000
#define SYSTEM_RAM_A_CTRL_SIZE 0x800
#define SYSTEM_BMB_PERIPHERAL_BMB 0xf8000000
#define SYSTEM_BMB_PERIPHERAL_BMB_SIZE 0x1000000
#define SYSTEM_PLIC_CTRL 0xf8c00000
#define SYSTEM_PLIC_CTRL_SIZE 0x400000
#define SYSTEM_CLINT_CTRL 0xf8b00000
#define SYSTEM_CLINT_CTRL_SIZE 0x10000
#define SYSTEM_UART_0_IO_CTRL 0xf8010000
#define SYSTEM_UART_0_IO_CTRL_SIZE 0x40
#define SYSTEM_SPI_0_IO_CTRL 0xf8014000
#define SYSTEM_SPI_0_IO_CTRL_SIZE 0x1000
#define SYSTEM_SPI_1_IO_CTRL 0xf8015000
#define SYSTEM_SPI_1_IO_CTRL_SIZE 0x1000
#define SYSTEM_I2C_1_IO_CTRL 0xf8018000
#define SYSTEM_I2C_1_IO_CTRL_SIZE 0x100
#define SYSTEM_I2C_0_IO_CTRL 0xf8017000
#define SYSTEM_I2C_0_IO_CTRL_SIZE 0x100
#define SYSTEM_I2C_2_IO_CTRL 0xf8019000
#define SYSTEM_I2C_2_IO_CTRL_SIZE 0x100
#define SYSTEM_USER_TIMER_0_CTRL 0xf801a000
#define SYSTEM_USER_TIMER_0_CTRL_SIZE 0x1000
#define SYSTEM_USER_TIMER_1_CTRL 0xf801b000
#define SYSTEM_USER_TIMER_1_CTRL_SIZE 0x1000
#define SYSTEM_GPIO_0_IO_CTRL 0xf8016000
#define SYSTEM_GPIO_0_IO_CTRL_SIZE 0x100
#define IO_APB_SLAVE_0_INPUT 0xf8100000
#define IO_APB_SLAVE_0_INPUT_SIZE 0x10000
#define IO_APB_SLAVE_2_INPUT 0xf8120000
#define IO_APB_SLAVE_2_INPUT_SIZE 0x10000
#define IO_APB_SLAVE_1_INPUT 0xf8110000
#define IO_APB_SLAVE_1_INPUT_SIZE 0x10000
#define IO_APB_SLAVE_4_INPUT 0xf8140000
#define IO_APB_SLAVE_4_INPUT_SIZE 0x10000
#define IO_APB_SLAVE_3_INPUT 0xf8130000
#define IO_APB_SLAVE_3_INPUT_SIZE 0x10000
#define SYSTEM_DDR_BMB 0x1000
#define SYSTEM_DDR_BMB_SIZE 0xe0000000
#define SYSTEM_AXI_A_BMB 0xe1000000
#define SYSTEM_AXI_A_BMB_SIZE 0x0800000
#define SYSTEM_AXI_B_BMB 0xe1800000
#define SYSTEM_AXI_B_BMB_SIZE 0x10000
#define SYSTEM_AXI_C_BMB 0xe1810000
#define SYSTEM_AXI_C_BMB_SIZE 0x10000 
#define SYSTEM_AXI_SLAVE_0_IO_CTRL 0xe1000000
#define SYSTEM_AXI_SLAVE_0_IO_CTRL_SIZE 0x0800000
#define SYSTEM_AXI_SLAVE_1_IO_CTRL 0xe1800000
#define SYSTEM_AXI_SLAVE_1_IO_CTRL_SIZE 0x10000
#define SYSTEM_AXI_SLAVE_2_IO_CTRL 0xe1810000
#define SYSTEM_AXI_SLAVE_2_IO_CTRL_SIZE 0x10000

#endif