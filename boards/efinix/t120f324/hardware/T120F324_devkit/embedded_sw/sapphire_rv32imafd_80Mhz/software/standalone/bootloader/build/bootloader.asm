
build/bootloader.elf:     file format elf32-littleriscv


Disassembly of section .init:

f9000000 <_start>:

_start:
#ifdef USE_GP
.option push
.option norelax
	la gp, __global_pointer$
f9000000:	00001197          	auipc	gp,0x1
f9000004:	e4018193          	addi	gp,gp,-448 # f9000e40 <__global_pointer$>

f9000008 <init>:
	sw a0, smp_lottery_lock, a1
    ret
#endif

init:
	la sp, _sp
f9000008:	91c18113          	addi	sp,gp,-1764 # f900075c <_sp>

	/* Load data section */
	la a0, _data_lma
f900000c:	00000517          	auipc	a0,0x0
f9000010:	61050513          	addi	a0,a0,1552 # f900061c <__init_array_end>
	la a1, _data
f9000014:	00000597          	auipc	a1,0x0
f9000018:	60858593          	addi	a1,a1,1544 # f900061c <__init_array_end>
	la a2, _edata
f900001c:	81c18613          	addi	a2,gp,-2020 # f900065c <__bss_start>
	bgeu a1, a2, 2f
f9000020:	00c5fc63          	bgeu	a1,a2,f9000038 <init+0x30>
1:
	lw t0, (a0)
f9000024:	00052283          	lw	t0,0(a0)
	sw t0, (a1)
f9000028:	0055a023          	sw	t0,0(a1)
	addi a0, a0, 4
f900002c:	00450513          	addi	a0,a0,4
	addi a1, a1, 4
f9000030:	00458593          	addi	a1,a1,4
	bltu a1, a2, 1b
f9000034:	fec5e8e3          	bltu	a1,a2,f9000024 <init+0x1c>
2:

	/* Clear bss section */
	la a0, __bss_start
f9000038:	81c18513          	addi	a0,gp,-2020 # f900065c <__bss_start>
	la a1, _end
f900003c:	82018593          	addi	a1,gp,-2016 # f9000660 <_end>
	bgeu a0, a1, 2f
f9000040:	00b57863          	bgeu	a0,a1,f9000050 <init+0x48>
1:
	sw zero, (a0)
f9000044:	00052023          	sw	zero,0(a0)
	addi a0, a0, 4
f9000048:	00450513          	addi	a0,a0,4
	bltu a0, a1, 1b
f900004c:	feb56ce3          	bltu	a0,a1,f9000044 <init+0x3c>
2:

#ifndef NO_LIBC_INIT_ARRAY
	call __libc_init_array
f9000050:	538000ef          	jal	ra,f9000588 <__libc_init_array>
#endif

	call main
f9000054:	51c000ef          	jal	ra,f9000570 <main>

f9000058 <mainDone>:
mainDone:
    j mainDone
f9000058:	0000006f          	j	f9000058 <mainDone>

f900005c <_init>:


	.globl _init
_init:
    ret
f900005c:	00008067          	ret

Disassembly of section .text:

f9000060 <uart_writeAvailability>:
#include "type.h"
#include "soc.h"


    static inline u32 read_u32(u32 address){
        return *((volatile u32*) address);
f9000060:	00452503          	lw	a0,4(a0)
        enum UartStop stop;
        u32 clockDivider;
    } Uart_Config;
    
    static u32 uart_writeAvailability(u32 reg){
        return (read_u32(reg + UART_STATUS) >> 16) & 0xFF;
f9000064:	01055513          	srli	a0,a0,0x10
    }
f9000068:	0ff57513          	andi	a0,a0,255
f900006c:	00008067          	ret

f9000070 <uart_write>:
    static u32 uart_readOccupancy(u32 reg){
        return read_u32(reg + UART_STATUS) >> 24;
    }
    
    static void uart_write(u32 reg, char data){
f9000070:	ff010113          	addi	sp,sp,-16
f9000074:	00112623          	sw	ra,12(sp)
f9000078:	00812423          	sw	s0,8(sp)
f900007c:	00912223          	sw	s1,4(sp)
f9000080:	00050413          	mv	s0,a0
f9000084:	00058493          	mv	s1,a1
        while(uart_writeAvailability(reg) == 0);
f9000088:	00040513          	mv	a0,s0
f900008c:	fd5ff0ef          	jal	ra,f9000060 <uart_writeAvailability>
f9000090:	fe050ce3          	beqz	a0,f9000088 <uart_write+0x18>
    }
    
    static inline void write_u32(u32 data, u32 address){
        *((volatile u32*) address) = data;
f9000094:	00942023          	sw	s1,0(s0)
        write_u32(data, reg + UART_DATA);
    }
f9000098:	00c12083          	lw	ra,12(sp)
f900009c:	00812403          	lw	s0,8(sp)
f90000a0:	00412483          	lw	s1,4(sp)
f90000a4:	01010113          	addi	sp,sp,16
f90000a8:	00008067          	ret

f90000ac <uart_writeStr>:
    
    static void uart_writeStr(u32 reg, const char* str){
f90000ac:	ff010113          	addi	sp,sp,-16
f90000b0:	00112623          	sw	ra,12(sp)
f90000b4:	00812423          	sw	s0,8(sp)
f90000b8:	00912223          	sw	s1,4(sp)
f90000bc:	00050493          	mv	s1,a0
f90000c0:	00058413          	mv	s0,a1
        while(*str) uart_write(reg, *str++);
f90000c4:	00044583          	lbu	a1,0(s0)
f90000c8:	00058a63          	beqz	a1,f90000dc <uart_writeStr+0x30>
f90000cc:	00140413          	addi	s0,s0,1
f90000d0:	00048513          	mv	a0,s1
f90000d4:	f9dff0ef          	jal	ra,f9000070 <uart_write>
f90000d8:	fedff06f          	j	f90000c4 <uart_writeStr+0x18>
    }
f90000dc:	00c12083          	lw	ra,12(sp)
f90000e0:	00812403          	lw	s0,8(sp)
f90000e4:	00412483          	lw	s1,4(sp)
f90000e8:	01010113          	addi	sp,sp,16
f90000ec:	00008067          	ret

f90000f0 <clint_uDelay>:
    
        return (((u64)hi) << 32) | lo;
    }
    
    static void clint_uDelay(u32 usec, u32 hz, u32 reg){
        u32 mTimePerUsec = hz/1000000;
f90000f0:	000f47b7          	lui	a5,0xf4
f90000f4:	24078793          	addi	a5,a5,576 # f4240 <__stack_size+0xf4140>
f90000f8:	02f5d5b3          	divu	a1,a1,a5
    readReg_u32 (clint_getTimeLow , CLINT_TIME_ADDR)
f90000fc:	0000c7b7          	lui	a5,0xc
f9000100:	ff878793          	addi	a5,a5,-8 # bff8 <__stack_size+0xbef8>
f9000104:	00f60633          	add	a2,a2,a5
        return *((volatile u32*) address);
f9000108:	00062783          	lw	a5,0(a2)
        u32 limit = clint_getTimeLow(reg) + usec*mTimePerUsec;
f900010c:	02a58533          	mul	a0,a1,a0
f9000110:	00f50533          	add	a0,a0,a5
f9000114:	00062783          	lw	a5,0(a2)
        while((int32_t)(limit-(clint_getTimeLow(reg))) >= 0);
f9000118:	40f507b3          	sub	a5,a0,a5
f900011c:	fe07dce3          	bgez	a5,f9000114 <clint_uDelay+0x24>
    }
f9000120:	00008067          	ret

f9000124 <spi_cmdAvailability>:
f9000124:	00452503          	lw	a0,4(a0)
        u32 ssDisable;
    } Spi_Config;
    
    static u32 spi_cmdAvailability(u32 reg){
        return read_u32(reg + SPI_BUFFER) & 0xFFFF;
    }
f9000128:	01051513          	slli	a0,a0,0x10
f900012c:	01055513          	srli	a0,a0,0x10
f9000130:	00008067          	ret

f9000134 <spi_rspOccupancy>:
f9000134:	00452503          	lw	a0,4(a0)
    static u32 spi_rspOccupancy(u32 reg){
        return read_u32(reg + SPI_BUFFER) >> 16;
    }
f9000138:	01055513          	srli	a0,a0,0x10
f900013c:	00008067          	ret

f9000140 <spi_write>:
    
    static void spi_write(u32 reg, u8 data){
f9000140:	ff010113          	addi	sp,sp,-16
f9000144:	00112623          	sw	ra,12(sp)
f9000148:	00812423          	sw	s0,8(sp)
f900014c:	00912223          	sw	s1,4(sp)
f9000150:	00050413          	mv	s0,a0
f9000154:	00058493          	mv	s1,a1
        while(spi_cmdAvailability(reg) == 0);
f9000158:	00040513          	mv	a0,s0
f900015c:	fc9ff0ef          	jal	ra,f9000124 <spi_cmdAvailability>
f9000160:	fe050ce3          	beqz	a0,f9000158 <spi_write+0x18>
        write_u32(data | SPI_CMD_WRITE, reg + SPI_DATA);
f9000164:	1004e493          	ori	s1,s1,256
        *((volatile u32*) address) = data;
f9000168:	00942023          	sw	s1,0(s0)
    }
f900016c:	00c12083          	lw	ra,12(sp)
f9000170:	00812403          	lw	s0,8(sp)
f9000174:	00412483          	lw	s1,4(sp)
f9000178:	01010113          	addi	sp,sp,16
f900017c:	00008067          	ret

f9000180 <spi_read>:
    
    static u8 spi_read(u32 reg){
f9000180:	ff010113          	addi	sp,sp,-16
f9000184:	00112623          	sw	ra,12(sp)
f9000188:	00812423          	sw	s0,8(sp)
f900018c:	00050413          	mv	s0,a0
        while(spi_cmdAvailability(reg) == 0);
f9000190:	00040513          	mv	a0,s0
f9000194:	f91ff0ef          	jal	ra,f9000124 <spi_cmdAvailability>
f9000198:	fe050ce3          	beqz	a0,f9000190 <spi_read+0x10>
f900019c:	20000793          	li	a5,512
f90001a0:	00f42023          	sw	a5,0(s0)
        write_u32(SPI_CMD_READ, reg + SPI_DATA);
        while(spi_rspOccupancy(reg) == 0);
f90001a4:	00040513          	mv	a0,s0
f90001a8:	f8dff0ef          	jal	ra,f9000134 <spi_rspOccupancy>
f90001ac:	fe050ce3          	beqz	a0,f90001a4 <spi_read+0x24>
        return *((volatile u32*) address);
f90001b0:	00042503          	lw	a0,0(s0)
        return read_u32(reg + SPI_DATA);
    }
f90001b4:	0ff57513          	andi	a0,a0,255
f90001b8:	00c12083          	lw	ra,12(sp)
f90001bc:	00812403          	lw	s0,8(sp)
f90001c0:	01010113          	addi	sp,sp,16
f90001c4:	00008067          	ret

f90001c8 <spi_select>:
        write_u32(data | SPI_CMD_READ | SPI_CMD_WRITE, reg + SPI_DATA);
        while(spi_rspOccupancy(reg) == 0);
        return read_u32(reg + SPI_DATA);
    }
    
    static void spi_select(u32 reg, u32 slaveId){
f90001c8:	ff010113          	addi	sp,sp,-16
f90001cc:	00112623          	sw	ra,12(sp)
f90001d0:	00812423          	sw	s0,8(sp)
f90001d4:	00912223          	sw	s1,4(sp)
f90001d8:	00050413          	mv	s0,a0
f90001dc:	00058493          	mv	s1,a1
        while(spi_cmdAvailability(reg) == 0);
f90001e0:	00040513          	mv	a0,s0
f90001e4:	f41ff0ef          	jal	ra,f9000124 <spi_cmdAvailability>
f90001e8:	fe050ce3          	beqz	a0,f90001e0 <spi_select+0x18>
        write_u32(slaveId | 0x80 | SPI_CMD_SS, reg + SPI_DATA);
f90001ec:	000017b7          	lui	a5,0x1
f90001f0:	88078793          	addi	a5,a5,-1920 # 880 <__stack_size+0x780>
f90001f4:	00f4e4b3          	or	s1,s1,a5
        *((volatile u32*) address) = data;
f90001f8:	00942023          	sw	s1,0(s0)
    }
f90001fc:	00c12083          	lw	ra,12(sp)
f9000200:	00812403          	lw	s0,8(sp)
f9000204:	00412483          	lw	s1,4(sp)
f9000208:	01010113          	addi	sp,sp,16
f900020c:	00008067          	ret

f9000210 <spi_diselect>:
    
    static void spi_diselect(u32 reg, u32 slaveId){
f9000210:	ff010113          	addi	sp,sp,-16
f9000214:	00112623          	sw	ra,12(sp)
f9000218:	00812423          	sw	s0,8(sp)
f900021c:	00912223          	sw	s1,4(sp)
f9000220:	00050413          	mv	s0,a0
f9000224:	00058493          	mv	s1,a1
        while(spi_cmdAvailability(reg) == 0);
f9000228:	00040513          	mv	a0,s0
f900022c:	ef9ff0ef          	jal	ra,f9000124 <spi_cmdAvailability>
f9000230:	fe050ce3          	beqz	a0,f9000228 <spi_diselect+0x18>
        write_u32(slaveId | 0x00 | SPI_CMD_SS, reg + SPI_DATA);
f9000234:	000017b7          	lui	a5,0x1
f9000238:	80078793          	addi	a5,a5,-2048 # 800 <__stack_size+0x700>
f900023c:	00f4e4b3          	or	s1,s1,a5
f9000240:	00942023          	sw	s1,0(s0)
    }
f9000244:	00c12083          	lw	ra,12(sp)
f9000248:	00812403          	lw	s0,8(sp)
f900024c:	00412483          	lw	s1,4(sp)
f9000250:	01010113          	addi	sp,sp,16
f9000254:	00008067          	ret

f9000258 <spi_applyConfig>:
    
    static void spi_applyConfig(u32 reg, Spi_Config *config){
        write_u32((config->cpol << 0) | (config->cpha << 1) | (config->mode << 4), reg + SPI_CONFIG);
f9000258:	0005a783          	lw	a5,0(a1)
f900025c:	0045a703          	lw	a4,4(a1)
f9000260:	00171713          	slli	a4,a4,0x1
f9000264:	00e7e7b3          	or	a5,a5,a4
f9000268:	0085a703          	lw	a4,8(a1)
f900026c:	00471713          	slli	a4,a4,0x4
f9000270:	00e7e7b3          	or	a5,a5,a4
f9000274:	00f52423          	sw	a5,8(a0)
        write_u32(config->clkDivider, reg + SPI_CLK_DIVIDER);
f9000278:	00c5a783          	lw	a5,12(a1)
f900027c:	02f52023          	sw	a5,32(a0)
        write_u32(config->ssSetup, reg + SPI_SS_SETUP);
f9000280:	0105a783          	lw	a5,16(a1)
f9000284:	02f52223          	sw	a5,36(a0)
        write_u32(config->ssHold, reg + SPI_SS_HOLD);
f9000288:	0145a783          	lw	a5,20(a1)
f900028c:	02f52423          	sw	a5,40(a0)
        write_u32(config->ssDisable, reg + SPI_SS_DISABLE);
f9000290:	0185a783          	lw	a5,24(a1)
f9000294:	02f52623          	sw	a5,44(a0)
    }
f9000298:	00008067          	ret

f900029c <spiFlash_select>:
    static void spiFlash_diselect_withGpioCs(u32 gpio, u32 cs){
        gpio_setOutput(gpio, gpio_getOutput(gpio) | (1 << cs));
        bsp_uDelay(1);
    }
    
    static void spiFlash_select(u32 spi, u32 cs){
f900029c:	ff010113          	addi	sp,sp,-16
f90002a0:	00112623          	sw	ra,12(sp)
        spi_select(spi, cs);
f90002a4:	f25ff0ef          	jal	ra,f90001c8 <spi_select>
    }
f90002a8:	00c12083          	lw	ra,12(sp)
f90002ac:	01010113          	addi	sp,sp,16
f90002b0:	00008067          	ret

f90002b4 <spiFlash_diselect>:
    
    static void spiFlash_diselect(u32 spi, u32 cs){
f90002b4:	ff010113          	addi	sp,sp,-16
f90002b8:	00112623          	sw	ra,12(sp)
        spi_diselect(spi, cs);
f90002bc:	f55ff0ef          	jal	ra,f9000210 <spi_diselect>
    }
f90002c0:	00c12083          	lw	ra,12(sp)
f90002c4:	01010113          	addi	sp,sp,16
f90002c8:	00008067          	ret

f90002cc <spiFlash_init_>:
    
    static void spiFlash_init_(u32 spi){
f90002cc:	fd010113          	addi	sp,sp,-48
f90002d0:	02112623          	sw	ra,44(sp)
        Spi_Config spiCfg;
        spiCfg.cpol = 0;
f90002d4:	00012223          	sw	zero,4(sp)
        spiCfg.cpha = 0;
f90002d8:	00012423          	sw	zero,8(sp)
        spiCfg.mode = 0;
f90002dc:	00012623          	sw	zero,12(sp)
        spiCfg.clkDivider = 2;
f90002e0:	00200793          	li	a5,2
f90002e4:	00f12823          	sw	a5,16(sp)
        spiCfg.ssSetup = 2;
f90002e8:	00f12a23          	sw	a5,20(sp)
        spiCfg.ssHold = 2;
f90002ec:	00f12c23          	sw	a5,24(sp)
        spiCfg.ssDisable = 2;
f90002f0:	00f12e23          	sw	a5,28(sp)
        spi_applyConfig(spi, &spiCfg);
f90002f4:	00410593          	addi	a1,sp,4
f90002f8:	f61ff0ef          	jal	ra,f9000258 <spi_applyConfig>
    }
f90002fc:	02c12083          	lw	ra,44(sp)
f9000300:	03010113          	addi	sp,sp,48
f9000304:	00008067          	ret

f9000308 <spiFlash_init>:
        spiFlash_init_(spi);
        gpio_setOutputEnable(gpio, gpio_getOutputEnable(gpio) | (1 << cs));
        spiFlash_diselect_withGpioCs(gpio,cs);
    }
    
    static void spiFlash_init(u32 spi, u32 cs){
f9000308:	ff010113          	addi	sp,sp,-16
f900030c:	00112623          	sw	ra,12(sp)
f9000310:	00812423          	sw	s0,8(sp)
f9000314:	00912223          	sw	s1,4(sp)
f9000318:	00050413          	mv	s0,a0
f900031c:	00058493          	mv	s1,a1
        spiFlash_init_(spi);
f9000320:	fadff0ef          	jal	ra,f90002cc <spiFlash_init_>
        spiFlash_diselect(spi, cs);
f9000324:	00048593          	mv	a1,s1
f9000328:	00040513          	mv	a0,s0
f900032c:	f89ff0ef          	jal	ra,f90002b4 <spiFlash_diselect>
    }
f9000330:	00c12083          	lw	ra,12(sp)
f9000334:	00812403          	lw	s0,8(sp)
f9000338:	00412483          	lw	s1,4(sp)
f900033c:	01010113          	addi	sp,sp,16
f9000340:	00008067          	ret

f9000344 <spiFlash_wake_>:
    
    static void spiFlash_wake_(u32 spi){
f9000344:	ff010113          	addi	sp,sp,-16
f9000348:	00112623          	sw	ra,12(sp)
        spi_write(spi, 0xAB);
f900034c:	0ab00593          	li	a1,171
f9000350:	df1ff0ef          	jal	ra,f9000140 <spi_write>
    }
f9000354:	00c12083          	lw	ra,12(sp)
f9000358:	01010113          	addi	sp,sp,16
f900035c:	00008067          	ret

f9000360 <spiFlash_wake>:
        spiFlash_wake_(spi);
        spiFlash_diselect_withGpioCs(gpio,cs);
        bsp_uDelay(200);
    }
    
    static void spiFlash_wake(u32 spi, u32 cs){
f9000360:	ff010113          	addi	sp,sp,-16
f9000364:	00112623          	sw	ra,12(sp)
f9000368:	00812423          	sw	s0,8(sp)
f900036c:	00912223          	sw	s1,4(sp)
f9000370:	00050413          	mv	s0,a0
f9000374:	00058493          	mv	s1,a1
        spiFlash_select(spi,cs);
f9000378:	f25ff0ef          	jal	ra,f900029c <spiFlash_select>
        spiFlash_wake_(spi);
f900037c:	00040513          	mv	a0,s0
f9000380:	fc5ff0ef          	jal	ra,f9000344 <spiFlash_wake_>
        spiFlash_diselect(spi,cs);
f9000384:	00048593          	mv	a1,s1
f9000388:	00040513          	mv	a0,s0
f900038c:	f29ff0ef          	jal	ra,f90002b4 <spiFlash_diselect>
        bsp_uDelay(200);
f9000390:	f8b00637          	lui	a2,0xf8b00
f9000394:	04c4b5b7          	lui	a1,0x4c4b
f9000398:	40058593          	addi	a1,a1,1024 # 4c4b400 <__stack_size+0x4c4b300>
f900039c:	0c800513          	li	a0,200
f90003a0:	d51ff0ef          	jal	ra,f90000f0 <clint_uDelay>
    }
f90003a4:	00c12083          	lw	ra,12(sp)
f90003a8:	00812403          	lw	s0,8(sp)
f90003ac:	00412483          	lw	s1,4(sp)
f90003b0:	01010113          	addi	sp,sp,16
f90003b4:	00008067          	ret

f90003b8 <spiFlash_f2m_>:
        id = spiFlash_read_id_(spi);
        spiFlash_diselect(spi,cs);
        return id;
    }
    
    static void spiFlash_f2m_(u32 spi, u32 flashAddress, u32 memoryAddress, u32 size){
f90003b8:	fe010113          	addi	sp,sp,-32
f90003bc:	00112e23          	sw	ra,28(sp)
f90003c0:	00812c23          	sw	s0,24(sp)
f90003c4:	00912a23          	sw	s1,20(sp)
f90003c8:	01212823          	sw	s2,16(sp)
f90003cc:	01312623          	sw	s3,12(sp)
f90003d0:	00050913          	mv	s2,a0
f90003d4:	00058493          	mv	s1,a1
f90003d8:	00060413          	mv	s0,a2
f90003dc:	00068993          	mv	s3,a3
        spi_write(spi, 0x0B);
f90003e0:	00b00593          	li	a1,11
f90003e4:	d5dff0ef          	jal	ra,f9000140 <spi_write>
        spi_write(spi, flashAddress >> 16);
f90003e8:	0104d593          	srli	a1,s1,0x10
f90003ec:	0ff5f593          	andi	a1,a1,255
f90003f0:	00090513          	mv	a0,s2
f90003f4:	d4dff0ef          	jal	ra,f9000140 <spi_write>
        spi_write(spi, flashAddress >>  8);
f90003f8:	0084d593          	srli	a1,s1,0x8
f90003fc:	0ff5f593          	andi	a1,a1,255
f9000400:	00090513          	mv	a0,s2
f9000404:	d3dff0ef          	jal	ra,f9000140 <spi_write>
        spi_write(spi, flashAddress >>  0);
f9000408:	0ff4f593          	andi	a1,s1,255
f900040c:	00090513          	mv	a0,s2
f9000410:	d31ff0ef          	jal	ra,f9000140 <spi_write>
        spi_write(spi, 0);
f9000414:	00000593          	li	a1,0
f9000418:	00090513          	mv	a0,s2
f900041c:	d25ff0ef          	jal	ra,f9000140 <spi_write>
        uint8_t *ram = (uint8_t *) memoryAddress;
        for(u32 idx = 0;idx < size;idx++){
f9000420:	00000493          	li	s1,0
f9000424:	0134fe63          	bgeu	s1,s3,f9000440 <spiFlash_f2m_+0x88>
            u8 value = spi_read(spi);
f9000428:	00090513          	mv	a0,s2
f900042c:	d55ff0ef          	jal	ra,f9000180 <spi_read>
            *ram++ = value;
f9000430:	00a40023          	sb	a0,0(s0)
        for(u32 idx = 0;idx < size;idx++){
f9000434:	00148493          	addi	s1,s1,1
            *ram++ = value;
f9000438:	00140413          	addi	s0,s0,1
f900043c:	fe9ff06f          	j	f9000424 <spiFlash_f2m_+0x6c>
        }
    }
f9000440:	01c12083          	lw	ra,28(sp)
f9000444:	01812403          	lw	s0,24(sp)
f9000448:	01412483          	lw	s1,20(sp)
f900044c:	01012903          	lw	s2,16(sp)
f9000450:	00c12983          	lw	s3,12(sp)
f9000454:	02010113          	addi	sp,sp,32
f9000458:	00008067          	ret

f900045c <spiFlash_f2m>:
        spiFlash_select_withGpioCs(gpio,cs);
        spiFlash_f2m_(spi, flashAddress, memoryAddress, size);
        spiFlash_diselect_withGpioCs(gpio,cs);
    }
    
    static void spiFlash_f2m(u32 spi, u32 cs, u32 flashAddress, u32 memoryAddress, u32 size){
f900045c:	fe010113          	addi	sp,sp,-32
f9000460:	00112e23          	sw	ra,28(sp)
f9000464:	00812c23          	sw	s0,24(sp)
f9000468:	00912a23          	sw	s1,20(sp)
f900046c:	01212823          	sw	s2,16(sp)
f9000470:	01312623          	sw	s3,12(sp)
f9000474:	01412423          	sw	s4,8(sp)
f9000478:	00050413          	mv	s0,a0
f900047c:	00058493          	mv	s1,a1
f9000480:	00060913          	mv	s2,a2
f9000484:	00068993          	mv	s3,a3
f9000488:	00070a13          	mv	s4,a4
        spiFlash_select(spi,cs);
f900048c:	e11ff0ef          	jal	ra,f900029c <spiFlash_select>
        spiFlash_f2m_(spi, flashAddress, memoryAddress, size);
f9000490:	000a0693          	mv	a3,s4
f9000494:	00098613          	mv	a2,s3
f9000498:	00090593          	mv	a1,s2
f900049c:	00040513          	mv	a0,s0
f90004a0:	f19ff0ef          	jal	ra,f90003b8 <spiFlash_f2m_>
        spiFlash_diselect(spi,cs);
f90004a4:	00048593          	mv	a1,s1
f90004a8:	00040513          	mv	a0,s0
f90004ac:	e09ff0ef          	jal	ra,f90002b4 <spiFlash_diselect>
    }
f90004b0:	01c12083          	lw	ra,28(sp)
f90004b4:	01812403          	lw	s0,24(sp)
f90004b8:	01412483          	lw	s1,20(sp)
f90004bc:	01012903          	lw	s2,16(sp)
f90004c0:	00c12983          	lw	s3,12(sp)
f90004c4:	00812a03          	lw	s4,8(sp)
f90004c8:	02010113          	addi	sp,sp,32
f90004cc:	00008067          	ret

f90004d0 <bspMain>:

#define UBOOT_MEMORY		0x01040000
#define UBOOT_FLASH		0x480000
#define UBOOT_SIZE		0x0C0000

void bspMain() {
f90004d0:	ff010113          	addi	sp,sp,-16
f90004d4:	00112623          	sw	ra,12(sp)
#ifndef SIM
	spiFlash_init(SPI, SPI_CS);
f90004d8:	00000593          	li	a1,0
f90004dc:	f8014537          	lui	a0,0xf8014
f90004e0:	e29ff0ef          	jal	ra,f9000308 <spiFlash_init>
	spiFlash_wake(SPI, SPI_CS);
f90004e4:	00000593          	li	a1,0
f90004e8:	f8014537          	lui	a0,0xf8014
f90004ec:	e75ff0ef          	jal	ra,f9000360 <spiFlash_wake>
	bsp_putString("OpenSBI copy\n");
f90004f0:	f90005b7          	lui	a1,0xf9000
f90004f4:	61c58593          	addi	a1,a1,1564 # f900061c <__global_pointer$+0xfffff7dc>
f90004f8:	f8010537          	lui	a0,0xf8010
f90004fc:	bb1ff0ef          	jal	ra,f90000ac <uart_writeStr>
	spiFlash_f2m(SPI, SPI_CS, OPENSBI_FLASH, OPENSBI_MEMORY, OPENSBI_SIZE);
f9000500:	00040737          	lui	a4,0x40
f9000504:	010006b7          	lui	a3,0x1000
f9000508:	00400637          	lui	a2,0x400
f900050c:	00000593          	li	a1,0
f9000510:	f8014537          	lui	a0,0xf8014
f9000514:	f49ff0ef          	jal	ra,f900045c <spiFlash_f2m>
	bsp_putString("U-Boot copy\n");
f9000518:	f90005b7          	lui	a1,0xf9000
f900051c:	62c58593          	addi	a1,a1,1580 # f900062c <__global_pointer$+0xfffff7ec>
f9000520:	f8010537          	lui	a0,0xf8010
f9000524:	b89ff0ef          	jal	ra,f90000ac <uart_writeStr>
        spiFlash_f2m(SPI, SPI_CS, UBOOT_FLASH, UBOOT_MEMORY, UBOOT_SIZE);
f9000528:	000c0737          	lui	a4,0xc0
f900052c:	010406b7          	lui	a3,0x1040
f9000530:	00480637          	lui	a2,0x480
f9000534:	00000593          	li	a1,0
f9000538:	f8014537          	lui	a0,0xf8014
f900053c:	f21ff0ef          	jal	ra,f900045c <spiFlash_f2m>
#endif

	bsp_putString("Payload boot\n");
f9000540:	f90005b7          	lui	a1,0xf9000
f9000544:	63c58593          	addi	a1,a1,1596 # f900063c <__global_pointer$+0xfffff7fc>
f9000548:	f8010537          	lui	a0,0xf8010
f900054c:	b61ff0ef          	jal	ra,f90000ac <uart_writeStr>
	void (*userMain)(u32, u32, u32) = (void (*)(u32, u32, u32))OPENSBI_MEMORY;
	userMain(0,0,0);
f9000550:	00000613          	li	a2,0
f9000554:	00000593          	li	a1,0
f9000558:	00000513          	li	a0,0
f900055c:	010007b7          	lui	a5,0x1000
f9000560:	000780e7          	jalr	a5 # 1000000 <__stack_size+0xffff00>
}
f9000564:	00c12083          	lw	ra,12(sp)
f9000568:	01010113          	addi	sp,sp,16
f900056c:	00008067          	ret

f9000570 <main>:
///////////////////////////////////////////////////////////////////////////////////
#include "type.h"
#include "bsp.h"
#include "bootloaderConfig.h"

void main() {
f9000570:	ff010113          	addi	sp,sp,-16
f9000574:	00112623          	sw	ra,12(sp)
    bsp_init();
    bspMain();
f9000578:	f59ff0ef          	jal	ra,f90004d0 <bspMain>
}
f900057c:	00c12083          	lw	ra,12(sp)
f9000580:	01010113          	addi	sp,sp,16
f9000584:	00008067          	ret

f9000588 <__libc_init_array>:
f9000588:	ff010113          	addi	sp,sp,-16
f900058c:	00812423          	sw	s0,8(sp)
f9000590:	01212023          	sw	s2,0(sp)
f9000594:	00000417          	auipc	s0,0x0
f9000598:	08840413          	addi	s0,s0,136 # f900061c <__init_array_end>
f900059c:	00000917          	auipc	s2,0x0
f90005a0:	08090913          	addi	s2,s2,128 # f900061c <__init_array_end>
f90005a4:	40890933          	sub	s2,s2,s0
f90005a8:	00112623          	sw	ra,12(sp)
f90005ac:	00912223          	sw	s1,4(sp)
f90005b0:	40295913          	srai	s2,s2,0x2
f90005b4:	00090e63          	beqz	s2,f90005d0 <__libc_init_array+0x48>
f90005b8:	00000493          	li	s1,0
f90005bc:	00042783          	lw	a5,0(s0)
f90005c0:	00148493          	addi	s1,s1,1
f90005c4:	00440413          	addi	s0,s0,4
f90005c8:	000780e7          	jalr	a5
f90005cc:	fe9918e3          	bne	s2,s1,f90005bc <__libc_init_array+0x34>
f90005d0:	00000417          	auipc	s0,0x0
f90005d4:	04c40413          	addi	s0,s0,76 # f900061c <__init_array_end>
f90005d8:	00000917          	auipc	s2,0x0
f90005dc:	04490913          	addi	s2,s2,68 # f900061c <__init_array_end>
f90005e0:	40890933          	sub	s2,s2,s0
f90005e4:	40295913          	srai	s2,s2,0x2
f90005e8:	00090e63          	beqz	s2,f9000604 <__libc_init_array+0x7c>
f90005ec:	00000493          	li	s1,0
f90005f0:	00042783          	lw	a5,0(s0)
f90005f4:	00148493          	addi	s1,s1,1
f90005f8:	00440413          	addi	s0,s0,4
f90005fc:	000780e7          	jalr	a5
f9000600:	fe9918e3          	bne	s2,s1,f90005f0 <__libc_init_array+0x68>
f9000604:	00c12083          	lw	ra,12(sp)
f9000608:	00812403          	lw	s0,8(sp)
f900060c:	00412483          	lw	s1,4(sp)
f9000610:	00012903          	lw	s2,0(sp)
f9000614:	01010113          	addi	sp,sp,16
f9000618:	00008067          	ret
