#ifndef SOC_H
#define SOC_H
#define SYSTEM_PLIC_SYSTEM_CORES_0_EXTERNAL_INTERRUPT 0
#define SYSTEM_PLIC_SYSTEM_CORES_0_EXTERNAL_SUPERVISOR_INTERRUPT 1
#define SYSTEM_PLIC_USER_INTERRUPT_A_INTERRUPT 25
#define SYSTEM_PLIC_SYSTEM_AXI_A_INTERRUPT 30
#define SYSTEM_PLIC_SYSTEM_UART_0_IO_INTERRUPT 1
#define SYSTEM_PLIC_SYSTEM_UART_1_IO_INTERRUPT 2
#define SYSTEM_PLIC_SYSTEM_SPI_0_IO_INTERRUPT 4
#define SYSTEM_PLIC_SYSTEM_SPI_1_IO_INTERRUPT 5
#define SYSTEM_PLIC_SYSTEM_SPI_2_IO_INTERRUPT 6
#define SYSTEM_PLIC_SYSTEM_I2C_0_IO_INTERRUPT 8
#define SYSTEM_PLIC_SYSTEM_I2C_1_IO_INTERRUPT 9
#define SYSTEM_PLIC_SYSTEM_I2C_2_IO_INTERRUPT 10
#define SYSTEM_PLIC_SYSTEM_GPIO_0_IO_INTERRUPTS_0 12
#define SYSTEM_PLIC_SYSTEM_GPIO_0_IO_INTERRUPTS_1 13
#define SYSTEM_CLINT_HZ 50000000
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
#define SYSTEM_UART_1_IO_PARAMETER_UART_CTRL_CONFIG_DATA_WIDTH_MAX 8
#define SYSTEM_UART_1_IO_PARAMETER_UART_CTRL_CONFIG_CLOCK_DIVIDER_WIDTH 20
#define SYSTEM_UART_1_IO_PARAMETER_UART_CTRL_CONFIG_PRE_SAMPLING_SIZE 1
#define SYSTEM_UART_1_IO_PARAMETER_UART_CTRL_CONFIG_SAMPLING_SIZE 5
#define SYSTEM_UART_1_IO_PARAMETER_UART_CTRL_CONFIG_POST_SAMPLING_SIZE 2
#define SYSTEM_UART_1_IO_PARAMETER_UART_CTRL_CONFIG_CTS_GEN 0
#define SYSTEM_UART_1_IO_PARAMETER_UART_CTRL_CONFIG_RTS_GEN 0
#define SYSTEM_UART_1_IO_PARAMETER_UART_CTRL_CONFIG_RX_SAMPLE_PER_BIT 8
#define SYSTEM_UART_1_IO_PARAMETER_INIT_CONFIG_BAUDRATE 115200
#define SYSTEM_UART_1_IO_PARAMETER_INIT_CONFIG_DATA_LENGTH 7
#define SYSTEM_UART_1_IO_PARAMETER_INIT_CONFIG_PARITY NONE
#define SYSTEM_UART_1_IO_PARAMETER_INIT_CONFIG_STOP ONE
#define SYSTEM_UART_1_IO_PARAMETER_BUS_CAN_WRITE_CLOCK_DIVIDER_CONFIG 1
#define SYSTEM_UART_1_IO_PARAMETER_BUS_CAN_WRITE_FRAME_CONFIG 1
#define SYSTEM_UART_1_IO_PARAMETER_TX_FIFO_DEPTH 128
#define SYSTEM_UART_1_IO_PARAMETER_RX_FIFO_DEPTH 128
#define SYSTEM_CORES_0_D_BUS 0x0
#define SYSTEM_FABRIC_D_BUS_COHERENT_BMB 0x0
#define SYSTEM_FABRIC_EXCLUSIVE_MONITOR_INPUT 0x0
#define SYSTEM_FABRIC_EXCLUSIVE_MONITOR_OUTPUT 0x0
#define SYSTEM_FABRIC_INVALIDATION_MONITOR_INPUT 0x0
#define SYSTEM_FABRIC_INVALIDATION_MONITOR_OUTPUT 0x0
#define SYSTEM_FABRIC_D_BUS_BMB 0x0
#define SYSTEM_BRIDGE_BMB 0x0
#define SYSTEM_RAM_A_CTRL 0xf9000000
#define SYSTEM_BMB_PERIPHERAL_BMB 0xf8000000
#define SYSTEM_PLIC_CTRL 0xf8c00000
#define SYSTEM_CLINT_CTRL 0xf8b00000
#define SYSTEM_UART_0_IO_CTRL 0xf8010000
#define SYSTEM_UART_1_IO_CTRL 0xf8011000
#define SYSTEM_SPI_0_IO_CTRL 0xf8014000
#define SYSTEM_SPI_1_IO_CTRL 0xf8015000
#define SYSTEM_SPI_2_IO_CTRL 0xf8016000
#define SYSTEM_I2C_0_IO_CTRL 0xf8018000
#define SYSTEM_I2C_1_IO_CTRL 0xf8019000
#define SYSTEM_I2C_2_IO_CTRL 0xf801a000
#define SYSTEM_GPIO_0_IO_CTRL 0xf8000000
#define IO_APB_SLAVE_0_INPUT 0xf8800000
#define IO_APB_SLAVE_1_INPUT 0xf8801000
#define SYSTEM_DDR_BMB 0x1000
#define SYSTEM_AXI_A_BMB 0xfa000000
#endif