/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2013-2021 Efinix Inc. All rights reserved.
//
// Description:
// Example top file for EfxSapphireSoc
//
// Language:  Verilog 2001
//
// ------------------------------------------------------------------------------
// REVISION:
//  $Snapshot: $
//  $Id:$
//
// History:
// 1.0 Initial Release. 
/////////////////////////////////////////////////////////////////////////////////
`timescale 1ns / 1ps

module top_soc (
input [3:0] system_gpio_0_io_read,
output [3:0] system_gpio_0_io_write,
output [3:0] system_gpio_0_io_writeEnable,
output		io_ddrA_arw_valid,
input		io_ddrA_arw_ready,
output [31:0] io_ddrA_arw_payload_addr,
output [7:0] io_ddrA_arw_payload_id,
output [7:0] io_ddrA_arw_payload_len,
output [2:0] io_ddrA_arw_payload_size,
output [1:0] io_ddrA_arw_payload_burst,
output [1:0] io_ddrA_arw_payload_lock,
output		io_ddrA_arw_payload_write,
output [7:0] io_ddrA_w_payload_id,
output		io_ddrA_w_valid,
input		io_ddrA_w_ready,
output [127:0] io_ddrA_w_payload_data,
output [15:0] io_ddrA_w_payload_strb,
output		io_ddrA_w_payload_last,
input		io_ddrA_b_valid,
output		io_ddrA_b_ready,
input [7:0] io_ddrA_b_payload_id,
input		io_ddrA_r_valid,
output		io_ddrA_r_ready,
input [127:0] io_ddrA_r_payload_data,
input [7:0] io_ddrA_r_payload_id,
input [1:0] io_ddrA_r_payload_resp,
input		io_ddrA_r_payload_last,
input		io_memoryClk,
output		memoryClk_rstn,
input		memoryClk_locked,
output		system_spi_0_io_sclk_write,
output		system_spi_0_io_data_0_writeEnable,
input		system_spi_0_io_data_0_read,
output		system_spi_0_io_data_0_write,
output		system_spi_0_io_data_1_writeEnable,
input		system_spi_0_io_data_1_read,
output		system_spi_0_io_data_1_write,
output		system_spi_0_io_ss,
output		system_spi_1_io_sclk_write,
output		system_spi_1_io_data_0_writeEnable,
input		system_spi_1_io_data_0_read,
output		system_spi_1_io_data_0_write,
output		system_spi_1_io_data_1_writeEnable,
input		system_spi_1_io_data_1_read,
output		system_spi_1_io_data_1_write,
output		system_spi_1_io_ss,
input		jtag_inst1_TCK,
input		jtag_inst1_TDI,
output		jtag_inst1_TDO,
input		jtag_inst1_SEL,
input		jtag_inst1_CAPTURE,
input		jtag_inst1_SHIFT,
input		jtag_inst1_UPDATE,
input		jtag_inst1_RESET,
output		system_uart_0_io_txd,
input		system_uart_0_io_rxd,
output		system_i2c_0_io_sda_writeEnable,
output		system_i2c_0_io_sda_write,
input		system_i2c_0_io_sda_read,
output		system_i2c_0_io_scl_writeEnable,
output		system_i2c_0_io_scl_write,
input		system_i2c_0_io_scl_read,

output      memoryCheckerPass,
output      systemClk_rstn,
input       systemClk_locked,
input       io_systemClk,
input       io_asyncResetn

);
/////////////////////////////////////////////////////////////////////////////
//Reset and PLL
wire 		reset;
wire		io_systemReset;
wire 	    io_memoryReset;				
wire [1:0]  io_ddrA_b_payload_resp=2'b00;
wire [15:0] io_apbSlave_0_PADDR;
wire		io_apbSlave_0_PSEL;
wire		io_apbSlave_0_PENABLE;
wire		io_apbSlave_0_PREADY;
wire		io_apbSlave_0_PWRITE;
wire [31:0] io_apbSlave_0_PWDATA;
wire [31:0] io_apbSlave_0_PRDATA;
wire		io_apbSlave_0_PSLVERROR;


/////////////////////////////////////////////////////////////////////////////
//Reset and PLL
assign reset 	= ~( io_asyncResetn & systemClk_locked);
assign systemClk_rstn 	= 1'b1;
assign memoryClk_rstn=1'b1;
assign memoryCheckerPass=1'b0;
assign system_i2c_0_io_sda_writeEnable = !system_i2c_0_io_sda_write;
assign system_i2c_0_io_scl_writeEnable = !system_i2c_0_io_scl_write;


/////////////////////////////////////////////////////////////////////////////
apb3_slave #(
.ADDR_WIDTH(16)) apb_slave_0 (
.clk(io_systemClk),
.resetn(~io_systemReset),
.PADDR(io_apbSlave_0_PADDR),
.PSEL(io_apbSlave_0_PSEL),
.PENABLE(io_apbSlave_0_PENABLE),
.PREADY(io_apbSlave_0_PREADY),
.PWRITE(io_apbSlave_0_PWRITE),
.PWDATA(io_apbSlave_0_PWDATA),
.PRDATA(io_apbSlave_0_PRDATA),
.PSLVERROR(io_apbSlave_0_PSLVERROR));



/////////////////////////////////////////////////////////////////////////////

sapphire_rv32imafd_80Mhz soc_inst
(
.io_memoryClk(io_memoryClk),
.io_memoryReset(io_memoryReset),
.io_ddrA_arw_valid(io_ddrA_arw_valid),
.io_ddrA_arw_ready(io_ddrA_arw_ready),
.io_ddrA_arw_payload_addr(io_ddrA_arw_payload_addr),
.io_ddrA_arw_payload_id(io_ddrA_arw_payload_id),
.io_ddrA_arw_payload_len(io_ddrA_arw_payload_len),
.io_ddrA_arw_payload_size(io_ddrA_arw_payload_size),
.io_ddrA_arw_payload_burst(io_ddrA_arw_payload_burst),
.io_ddrA_arw_payload_lock(io_ddrA_arw_payload_lock[0]),
.io_ddrA_arw_payload_write(io_ddrA_arw_payload_write),
.io_ddrA_arw_payload_prot(),
.io_ddrA_arw_payload_qos(),
.io_ddrA_arw_payload_cache(),
.io_ddrA_arw_payload_region(),
.io_ddrA_w_payload_id(io_ddrA_w_payload_id),
.io_ddrA_w_valid(io_ddrA_w_valid),
.io_ddrA_w_ready(io_ddrA_w_ready),
.io_ddrA_w_payload_data(io_ddrA_w_payload_data),
.io_ddrA_w_payload_strb(io_ddrA_w_payload_strb),
.io_ddrA_w_payload_last(io_ddrA_w_payload_last),
.io_ddrA_b_valid(io_ddrA_b_valid),
.io_ddrA_b_ready(io_ddrA_b_ready),
.io_ddrA_b_payload_id(io_ddrA_b_payload_id),
.io_ddrA_b_payload_resp(io_ddrA_b_payload_resp),
.io_ddrA_r_valid(io_ddrA_r_valid),
.io_ddrA_r_ready(io_ddrA_r_ready),
.io_ddrA_r_payload_data(io_ddrA_r_payload_data),
.io_ddrA_r_payload_id(io_ddrA_r_payload_id),
.io_ddrA_r_payload_resp(io_ddrA_r_payload_resp),
.io_ddrA_r_payload_last(io_ddrA_r_payload_last),
.system_i2c_0_io_sda_write(system_i2c_0_io_sda_write),
.system_i2c_0_io_sda_read(system_i2c_0_io_sda_read),
.system_i2c_0_io_scl_write(system_i2c_0_io_scl_write),
.system_i2c_0_io_scl_read(system_i2c_0_io_scl_read),
.system_gpio_0_io_read(system_gpio_0_io_read),
.system_gpio_0_io_write(system_gpio_0_io_write),
.system_gpio_0_io_writeEnable(system_gpio_0_io_writeEnable),
.system_spi_1_io_sclk_write(system_spi_1_io_sclk_write),
.system_spi_1_io_data_0_writeEnable(system_spi_1_io_data_0_writeEnable),
.system_spi_1_io_data_0_read(system_spi_1_io_data_0_read),
.system_spi_1_io_data_0_write(system_spi_1_io_data_0_write),
.system_spi_1_io_data_1_writeEnable(system_spi_1_io_data_1_writeEnable),
.system_spi_1_io_data_1_read(system_spi_1_io_data_1_read),
.system_spi_1_io_data_1_write(system_spi_1_io_data_1_write),
.system_spi_1_io_data_2_writeEnable(),
.system_spi_1_io_data_2_read(),
.system_spi_1_io_data_2_write(),
.system_spi_1_io_data_3_writeEnable(),
.system_spi_1_io_data_3_read(),
.system_spi_1_io_data_3_write(),
.system_spi_1_io_ss(system_spi_1_io_ss),
.system_uart_0_io_txd(system_uart_0_io_txd),
.system_uart_0_io_rxd(system_uart_0_io_rxd),
.system_spi_0_io_sclk_write(system_spi_0_io_sclk_write),
.system_spi_0_io_data_0_writeEnable(system_spi_0_io_data_0_writeEnable),
.system_spi_0_io_data_0_read(system_spi_0_io_data_0_read),
.system_spi_0_io_data_0_write(system_spi_0_io_data_0_write),
.system_spi_0_io_data_1_writeEnable(system_spi_0_io_data_1_writeEnable),
.system_spi_0_io_data_1_read(system_spi_0_io_data_1_read),
.system_spi_0_io_data_1_write(system_spi_0_io_data_1_write),
.system_spi_0_io_data_2_writeEnable(),
.system_spi_0_io_data_2_read(),
.system_spi_0_io_data_2_write(),
.system_spi_0_io_data_3_writeEnable(),
.system_spi_0_io_data_3_read(),
.system_spi_0_io_data_3_write(),
.system_spi_0_io_ss(system_spi_0_io_ss),
.io_apbSlave_0_PADDR(io_apbSlave_0_PADDR),
.io_apbSlave_0_PSEL(io_apbSlave_0_PSEL),
.io_apbSlave_0_PENABLE(io_apbSlave_0_PENABLE),
.io_apbSlave_0_PREADY(io_apbSlave_0_PREADY),
.io_apbSlave_0_PWRITE(io_apbSlave_0_PWRITE),
.io_apbSlave_0_PWDATA(io_apbSlave_0_PWDATA),
.io_apbSlave_0_PRDATA(io_apbSlave_0_PRDATA),
.io_apbSlave_0_PSLVERROR(io_apbSlave_0_PSLVERROR),
.jtagCtrl_tck(jtag_inst1_TCK),
.jtagCtrl_tdi(jtag_inst1_TDI),
.jtagCtrl_tdo(jtag_inst1_TDO),
.jtagCtrl_enable(jtag_inst1_SEL),
.jtagCtrl_capture(jtag_inst1_CAPTURE),
.jtagCtrl_shift(jtag_inst1_SHIFT),
.jtagCtrl_update(jtag_inst1_UPDATE),
.jtagCtrl_reset(jtag_inst1_RESET),

.io_systemClk(io_systemClk),
.io_asyncReset(reset),
.io_systemReset(io_systemReset)		
);

endmodule

//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2013-2021 Efinix Inc. All rights reserved.
//
// This   document  contains  proprietary information  which   is
// protected by  copyright. All rights  are reserved.  This notice
// refers to original work by Efinix, Inc. which may be derivitive
// of other work distributed under license of the authors.  In the
// case of derivative work, nothing in this notice overrides the
// original author's license agreement.  Where applicable, the 
// original license agreement is included in it's original 
// unmodified form immediately below this header.
//
// WARRANTY DISCLAIMER.  
//     THE  DESIGN, CODE, OR INFORMATION ARE PROVIDED “AS IS” AND 
//     EFINIX MAKES NO WARRANTIES, EXPRESS OR IMPLIED WITH 
//     RESPECT THERETO, AND EXPRESSLY DISCLAIMS ANY IMPLIED WARRANTIES, 
//     INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF 
//     MERCHANTABILITY, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR 
//     PURPOSE.  SOME STATES DO NOT ALLOW EXCLUSIONS OF AN IMPLIED 
//     WARRANTY, SO THIS DISCLAIMER MAY NOT APPLY TO LICENSEE.
//
// LIMITATION OF LIABILITY.  
//     NOTWITHSTANDING ANYTHING TO THE CONTRARY, EXCEPT FOR BODILY 
//     INJURY, EFINIX SHALL NOT BE LIABLE WITH RESPECT TO ANY SUBJECT 
//     MATTER OF THIS AGREEMENT UNDER TORT, CONTRACT, STRICT LIABILITY 
//     OR ANY OTHER LEGAL OR EQUITABLE THEORY (I) FOR ANY INDIRECT, 
//     SPECIAL, INCIDENTAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES OF ANY 
//     CHARACTER INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF 
//     GOODWILL, DATA OR PROFIT, WORK STOPPAGE, OR COMPUTER FAILURE OR 
//     MALFUNCTION, OR IN ANY EVENT (II) FOR ANY AMOUNT IN EXCESS, IN 
//     THE AGGREGATE, OF THE FEE PAID BY LICENSEE TO EFINIX HEREUNDER 
//     (OR, IF THE FEE HAS BEEN WAIVED, $100), EVEN IF EFINIX SHALL HAVE 
//     BEEN INFORMED OF THE POSSIBILITY OF SUCH DAMAGES.  SOME STATES DO 
//     NOT ALLOW THE EXCLUSION OR LIMITATION OF INCIDENTAL OR 
//     CONSEQUENTIAL DAMAGES, SO THIS LIMITATION AND EXCLUSION MAY NOT 
//     APPLY TO LICENSEE.
//
/////////////////////////////////////////////////////////////////////////////
