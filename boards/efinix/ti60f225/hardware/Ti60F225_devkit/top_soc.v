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
input		jtag_inst1_TCK,
input		jtag_inst1_TDI,
output		jtag_inst1_TDO,
input		jtag_inst1_SEL,
input		jtag_inst1_CAPTURE,
input		jtag_inst1_SHIFT,
input		jtag_inst1_UPDATE,
input		jtag_inst1_RESET,
output		system_i2c_0_io_sda_writeEnable,
output		system_i2c_0_io_sda_write,
input		system_i2c_0_io_sda_read,
output		system_i2c_0_io_scl_writeEnable,
output		system_i2c_0_io_scl_write,
input		system_i2c_0_io_scl_read,
input		io_memoryClk,
input		hbramClk,
input		hbramClk_cal,
output		hbc_rst_n,
output		hbc_cs_n,
output		hbc_ck_p_HI,
output		hbc_ck_p_LO,
output		hbc_ck_n_HI,
output		hbc_ck_n_LO,
output [1:0] hbc_rwds_OUT_HI,
output [1:0] hbc_rwds_OUT_LO,
input  [1:0] hbc_rwds_IN_HI,
input  [1:0] hbc_rwds_IN_LO,
output [1:0] hbc_rwds_OE,
input  [15:0] hbc_dq_IN_LO,
input  [15:0] hbc_dq_IN_HI,
output [15:0] hbc_dq_OUT_HI,
output [15:0] hbc_dq_OUT_LO,
output [15:0] hbc_dq_OE,
output [2:0] hbc_cal_SHIFT,
output [4:0] hbc_cal_SHIFT_SEL,
output		hbc_cal_SHIFT_ENA,
output		hbc_cal_pass,
output		system_uart_0_io_txd,
input		system_uart_0_io_rxd,
input [3:0] system_gpio_0_io_read,
output [3:0] system_gpio_0_io_write,
output [3:0] system_gpio_0_io_writeEnable,
input		io_peripheralClk,
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
output      memoryCheckerPass,
output      systemClk_rstn,
input       systemClk_locked,
output      baseClk_pll_rstn,
input       baseClk_pll_locked,
input       io_systemClk,
input       io_asyncResetn

);
/////////////////////////////////////////////////////////////////////////////
//Reset and PLL
wire 		reset;
wire		io_systemReset;
wire 	    io_memoryReset;				
wire [1:0]  io_ddrA_b_payload_resp=2'b00;
wire		io_ddrA_arw_valid;
wire		io_ddrA_arw_ready;
wire [31:0] io_ddrA_arw_payload_addr;
wire [7:0] io_ddrA_arw_payload_id;
wire [7:0] io_ddrA_arw_payload_len;
wire [2:0] io_ddrA_arw_payload_size;
wire [1:0] io_ddrA_arw_payload_burst;
wire [1:0] io_ddrA_arw_payload_lock;
wire		io_ddrA_arw_payload_write;
wire [7:0] io_ddrA_w_payload_id;
wire		io_ddrA_w_valid;
wire		io_ddrA_w_ready;
wire [127:0] io_ddrA_w_payload_data;
wire [15:0] io_ddrA_w_payload_strb;
wire		io_ddrA_w_payload_last;
wire		io_ddrA_b_valid;
wire		io_ddrA_b_ready;
wire [7:0] io_ddrA_b_payload_id;
wire		io_ddrA_r_valid;
wire		io_ddrA_r_ready;
wire [127:0] io_ddrA_r_payload_data;
wire [7:0] io_ddrA_r_payload_id;
wire [1:0] io_ddrA_r_payload_resp;
wire		io_ddrA_r_payload_last;
wire		dyn_pll_phase_en;
wire [2:0] dyn_pll_phase_sel;
wire [15:0] io_apbSlave_0_PADDR;
wire		io_apbSlave_0_PSEL;
wire		io_apbSlave_0_PENABLE;
wire		io_apbSlave_0_PREADY;
wire		io_apbSlave_0_PWRITE;
wire [31:0] io_apbSlave_0_PWDATA;
wire [31:0] io_apbSlave_0_PRDATA;
wire		io_apbSlave_0_PSLVERROR;
wire io_peripheralReset;


/////////////////////////////////////////////////////////////////////////////
`include "hbram_top.vh"
//Reset and PLL
assign reset 	= ~( io_asyncResetn & systemClk_locked);
assign systemClk_rstn 	= 1'b1;
assign baseClk_pll_rstn = 1'b1;
assign dyn_pll_phase_en=1'b1;
assign dyn_pll_phase_sel=3'b010;
assign system_i2c_0_io_sda_writeEnable = !system_i2c_0_io_sda_write;
assign system_i2c_0_io_scl_writeEnable = !system_i2c_0_io_scl_write;
assign memoryCheckerPass=1'b0;


/////////////////////////////////////////////////////////////////////////////
apb3_slave #(
.ADDR_WIDTH(16)) apb_slave_0 (
.clk(io_peripheralClk),
.resetn(~io_peripheralReset),
.PADDR(io_apbSlave_0_PADDR),
.PSEL(io_apbSlave_0_PSEL),
.PENABLE(io_apbSlave_0_PENABLE),
.PREADY(io_apbSlave_0_PREADY),
.PWRITE(io_apbSlave_0_PWRITE),
.PWDATA(io_apbSlave_0_PWDATA),
.PRDATA(io_apbSlave_0_PRDATA),
.PSLVERROR(io_apbSlave_0_PSLVERROR));

hbram_top#(
.AXI_IF(AXI_IF),
.AXI_DBW(128),
.DDIN_MODE(DDIN_MODE),
.AXI_AWR_DEPTH(AXI_AWR_DEPTH),
.AXI_R_DEPTH(AXI_R_DEPTH),
.AXI_W_DEPTH(AXI_W_DEPTH),
.CAL_CLK_CH(CAL_CLK_CH),
.CAL_DQ_STEPS(CAL_DQ_STEPS),
.CAL_MODE(CAL_MODE),
.CAL_RWDS_STEPS(CAL_RWDS_STEPS),
.CR0_DPD(CR0_DPD),
.CR0_FLE(CR0_FLE),
.CR0_ILC(CR0_ILC),
.CR0_HBE(CR0_HBE),
.CR0_ODS(CR0_ODS),
.CR1_HSE(CR1_HSE),
.CR0_WBL(CR0_WBL),
.CR1_PAR(CR1_PAR),
.CR1_MCT(CR1_MCT),
.RAM_ABW(RAM_ABW),
.RAM_DBW(RAM_DBW),
.RDO_DELAY(RDO_DELAY),
.TRH(TRH),
.TRTR(TRTR),
.TVCS(TVCS),
.CAL_BYTES(CAL_BYTES),
.MHZ(MHZ),
.TCSM(TCSM),
.PLL_MANUAL(PLL_MANUAL)
) hbram_top_inst (
.rst(io_memoryReset),
.ram_clk(hbramClk),
.ram_clk_cal(hbramClk_cal),
.io_axi_clk(io_memoryClk),
.io_arw_valid(io_ddrA_arw_valid),
.io_arw_ready(io_ddrA_arw_ready),
.io_arw_payload_addr(io_ddrA_arw_payload_addr),
.io_arw_payload_id(io_ddrA_arw_payload_id),
.io_arw_payload_len(io_ddrA_arw_payload_len),
.io_arw_payload_size(io_ddrA_arw_payload_size),
.io_arw_payload_burst(io_ddrA_arw_payload_burst),
.io_arw_payload_lock(io_ddrA_arw_payload_lock),
.io_arw_payload_write(io_ddrA_arw_payload_write),
.io_w_payload_id(io_ddrA_w_payload_id),
.io_w_valid(io_ddrA_w_valid),
.io_w_ready(io_ddrA_w_ready),
.io_w_payload_data(io_ddrA_w_payload_data),
.io_w_payload_strb(io_ddrA_w_payload_strb),
.io_w_payload_last(io_ddrA_w_payload_last),
.io_b_valid(io_ddrA_b_valid),
.io_b_ready(io_ddrA_b_ready),
.io_b_payload_id(io_ddrA_b_payload_id),
.io_r_valid(io_ddrA_r_valid),
.io_r_ready(io_ddrA_r_ready),
.io_r_payload_data(io_ddrA_r_payload_data),
.io_r_payload_id(io_ddrA_r_payload_id),
.io_r_payload_resp(io_ddrA_r_payload_resp),
.io_r_payload_last(io_ddrA_r_payload_last),
.dyn_pll_phase_en(dyn_pll_phase_en),
.dyn_pll_phase_sel(dyn_pll_phase_sel),
.hbc_cal_SHIFT_ENA(hbc_cal_SHIFT_ENA),
.hbc_cal_SHIFT(hbc_cal_SHIFT),
.hbc_cal_SHIFT_SEL(hbc_cal_SHIFT_SEL),
.hbc_cal_pass(hbc_cal_pass),
.hbc_cal_debug_info(),
.hbc_rst_n(hbc_rst_n),
.hbc_cs_n(hbc_cs_n),
.hbc_ck_p_HI(hbc_ck_p_HI),
.hbc_ck_p_LO(hbc_ck_p_LO),
.hbc_ck_n_HI(hbc_ck_n_HI),
.hbc_ck_n_LO(hbc_ck_n_LO),
.hbc_rwds_OUT_HI(hbc_rwds_OUT_HI),
.hbc_rwds_OUT_LO(hbc_rwds_OUT_LO),
.hbc_rwds_IN_HI(hbc_rwds_IN_HI),
.hbc_rwds_IN_LO(hbc_rwds_IN_LO),
.hbc_rwds_OE(hbc_rwds_OE),
.hbc_dq_OUT_HI(hbc_dq_OUT_HI),
.hbc_dq_OUT_LO(hbc_dq_OUT_LO),
.hbc_dq_IN_HI(hbc_dq_IN_HI),
.hbc_dq_IN_LO(hbc_dq_IN_LO),
.hbc_dq_OE(hbc_dq_OE));


/////////////////////////////////////////////////////////////////////////////

sapphire_rv32imafd_250Mhz soc_inst
(
.system_uart_0_io_txd(system_uart_0_io_txd),
.system_uart_0_io_rxd(system_uart_0_io_rxd),
.io_peripheralClk(io_peripheralClk),
.io_peripheralReset(io_peripheralReset),
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
.jtagCtrl_tck(jtag_inst1_TCK),
.jtagCtrl_tdi(jtag_inst1_TDI),
.jtagCtrl_tdo(jtag_inst1_TDO),
.jtagCtrl_enable(jtag_inst1_SEL),
.jtagCtrl_capture(jtag_inst1_CAPTURE),
.jtagCtrl_shift(jtag_inst1_SHIFT),
.jtagCtrl_update(jtag_inst1_UPDATE),
.jtagCtrl_reset(jtag_inst1_RESET),
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
.system_i2c_0_io_sda_write(system_i2c_0_io_sda_write),
.system_i2c_0_io_sda_read(system_i2c_0_io_sda_read),
.system_i2c_0_io_scl_write(system_i2c_0_io_scl_write),
.system_i2c_0_io_scl_read(system_i2c_0_io_scl_read),
.system_gpio_0_io_read(system_gpio_0_io_read),
.system_gpio_0_io_write(system_gpio_0_io_write),
.system_gpio_0_io_writeEnable(system_gpio_0_io_writeEnable),
.io_apbSlave_0_PADDR(io_apbSlave_0_PADDR),
.io_apbSlave_0_PSEL(io_apbSlave_0_PSEL),
.io_apbSlave_0_PENABLE(io_apbSlave_0_PENABLE),
.io_apbSlave_0_PREADY(io_apbSlave_0_PREADY),
.io_apbSlave_0_PWRITE(io_apbSlave_0_PWRITE),
.io_apbSlave_0_PWDATA(io_apbSlave_0_PWDATA),
.io_apbSlave_0_PRDATA(io_apbSlave_0_PRDATA),
.io_apbSlave_0_PSLVERROR(io_apbSlave_0_PSLVERROR),

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
