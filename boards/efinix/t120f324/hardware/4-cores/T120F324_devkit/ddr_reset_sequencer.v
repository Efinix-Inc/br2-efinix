/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2013-2022 Efinix Inc. All rights reserved.
//
// Description:
//
// Efinix soft logic DDR system reset controller
//
// The Trion DDR controller block, (instantiated in the Interface Designer),
// has three input pins for reset control.  (when I2C calibration not enabled)
//
//     Master Reset (active low)
//     Sequencer Reset (active high)
//     Sequencer Start (active high)
//
// This Verilog module generates outputs that can directly
// connect to these pins, given a single reset signal and a clock.
//
// The module also generates a "done" status signal, to inform user system
// when reset + DDR-reinitialization is completed, and read/write operations
// to the DDR AXI interfaces may resume.  The user should define
// FREQ parameter to correspond to frequency of their clock signal.
//
// NOTE #1:  This reset sequencer resets and re-initializes both the DDR
//           interface of the Trion device, as well as the DDR module(s)
//           themselves.
//
// NOTE #2:  The user is not expected to pulse reset upon device configuration
//           and initial entry to user mode.  During the configuration process
//           DDR reset and initialization will be triggered automatically.  This
//           soft logic reset is only required if the user needs to reset
//           the DDR system again while maintaining the Trion device in user mode.
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
`resetall
`timescale 1ns / 1ps

module ddr_reset_sequencer 
(
ddr_rstn_i,
clk,
ddr_rstn,
ddr_cfg_seq_rst,
ddr_cfg_seq_start,
ddr_init_done
);
parameter FREQ = 100;			// default is 100 MHz.  Redefine as needed.

input ddr_rstn_i;				// main user DDR reset, active low
input clk;					// user clock

/* Connect these three signals to DDR reset interface */
output ddr_rstn;				// Master Reset
output ddr_cfg_seq_rst;		// Sequencer Reset
output reg ddr_cfg_seq_start;	// Sequencer Start

/* optional status monitor for user logic */
output reg ddr_init_done;		// Done status




localparam CNT_INIT = 1.5*FREQ*1000;

reg [1:0] rstn_dly;
always @(posedge clk or negedge ddr_rstn_i) begin
	if (!ddr_rstn_i) begin
		rstn_dly <= 3'd0;
	end else begin
		rstn_dly[0] <= 1'b1;
		rstn_dly[1] <= rstn_dly[0];
	end
end

assign ddr_rstn = ddr_rstn_i;

assign ddr_cfg_seq_rst = ~rstn_dly[1];

reg [19:0] cnt;
reg [1:0] cnt_start;

always @(posedge clk or negedge ddr_rstn_i) begin
	if (!ddr_rstn_i) begin
		ddr_init_done <= 1'b0;
		cnt <= CNT_INIT;
	end else begin
		if (cnt != 20'd0) begin
		cnt <= cnt - 20'd1;
		end else begin
		cnt <= cnt;
		ddr_init_done <= 1'b1;
		end
	end
end


always @(posedge clk or negedge rstn_dly[1]) begin
	if (!rstn_dly[1]) begin
		ddr_cfg_seq_start <= 1'b0;
		cnt_start <= 2'd0;
	end else begin
		if (cnt_start == 2'b11) begin
		ddr_cfg_seq_start <= 1'b1;
		cnt_start <= cnt_start;
		end else begin
		cnt_start <= cnt_start + 1'b1;
		end
	end

end


endmodule

//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2013-2019 Efinix Inc. All rights reserved.
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
