///////////////////////////////////////////////////////////////////////////////////
//  MIT License
//  
//  Copyright (c) 2023 SaxonSoc contributors
//  
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.
///////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "type.h"
#pragma GCC diagnostic ignored "-Wunused-function"
// #include "io.h"
#define I2C_TX_DATA                     0x00
#define I2C_TX_ACK                      0x04
#define I2C_RX_DATA                     0x08
#define I2C_RX_ACK                      0x0C
#define I2C_INTERRUPT_ENABLE            0x20
#define I2C_INTERRUPT_FLAG              0x24
#define I2C_SAMPLING_CLOCK_DIVIDER      0x28
#define I2C_TIMEOUT                     0x2C
#define I2C_TSUDAT                      0x30
#define I2C_MASTER_STATUS               0x40
#define I2C_SLAVE_STATUS                0x44
#define I2C_SLAVE_OVERRIDE              0x48
#define I2C_TLOW                        0x50
#define I2C_THIGH                       0x54
#define I2C_TBUF                        0x58
#define I2C_FILTERING_HIT               0x80
#define I2C_FILTERING_STATUS            0x84
#define I2C_FILTERING_CONFIG            0x88
#define I2C_MODE_CPOL                   (1 << 0)
#define I2C_MODE_CPHA                   (1 << 1)
#define I2C_TX_VALUE                    (0xFF)
#define I2C_TX_VALID                    (1 << 8)
#define I2C_TX_ENABLE                   (1 << 9)
#define I2C_TX_REPEAT                   (1 << 10)
#define I2C_TX_DISABLE_ON_DATA_CONFLICT (1 << 11)
#define I2C_RX_VALUE                    (0xFF)
#define I2C_RX_VALID                    (1 << 8)
#define I2C_RX_LISTEN                   (1 << 9)
#define I2C_MASTER_BUSY                 (1 << 0)
#define I2C_MASTER_START                (1 << 4)
#define I2C_MASTER_STOP                 (1 << 5)
#define I2C_MASTER_DROP                 (1 << 6)
#define I2C_MASTER_RECOVER              (1 << 7)
#define I2C_MASTER_START_DROPPED        (1 << 9)
#define I2C_MASTER_STOP_DROPPED         (1 << 10)
#define I2C_MASTER_RECOVER_DROPPED      (1 << 11)
#define I2C_SLAVE_STATUS_IN_FRAME       (1 << 0)
#define I2C_SLAVE_STATUS_SDA            (1 << 1)
#define I2C_SLAVE_STATUS_SCL            (1 << 2)
#define I2C_SLAVE_OVERRIDE_SDA          (1 << 1)
#define I2C_SLAVE_OVERRIDE_SCL          (1 << 2)
#define I2C_FILTER_7_BITS               (0)
#define I2C_FILTER_10_BITS              (1 << 14)
#define I2C_FILTER_ENABLE               (1 << 15)
#define I2C_INTERRUPT_TX_DATA           (1 << 2)
#define I2C_INTERRUPT_TX_ACK            (1 << 3)
#define I2C_INTERRUPT_DROP              (1 << 7)
#define I2C_INTERRUPT_CLOCK_GEN_EXIT    (1 << 15)
#define I2C_INTERRUPT_CLOCK_GEN_ENTER   (1 << 16)
#define I2C_INTERRUPT_CLOCK_GEN_BUSY    (1 << 16) //Renamed into I2C_INTERRUPT_CLOCK_GEN_ENTER
#define I2C_INTERRUPT_FILTER            (1 << 17)
#define I2C_READ                        0x01
#define I2C_WRITE                       0x00


/* Backward compatibility
* Please use i2c_get* functions for new designs
*/
#define gpio_getInterruptFlag(reg) i2c_getInterruptFlag(reg)
#define gpio_getMasterStatus(reg) i2c_getMasterStatus(reg)
#define gpio_getFilteringHit(reg) i2c_getFilteringHit(reg)
#define gpio_getFilteringStatus(reg) i2c_getFilteringStatus(reg)
//end
    readReg_u32 (i2c_getInterruptFlag   , I2C_INTERRUPT_FLAG)
    readReg_u32 (i2c_getMasterStatus    , I2C_MASTER_STATUS)
    readReg_u32 (i2c_getFilteringHit    , I2C_FILTERING_HIT)
    readReg_u32 (i2c_getFilteringStatus , I2C_FILTERING_STATUS)
    readReg_u32 (i2c_getSlaveStatus , I2C_SLAVE_STATUS)
    writeReg_u32 (i2c_getSlaveOverride , I2C_SLAVE_OVERRIDE)


/// This is my try struct
/// This is the detailed explanation
    typedef struct {
        //Master/Slave mode
        //Number of cycle - 1 between each SDA/SCL sample
        u32 samplingClockDivider; 
        //Number of cycle - 1 after which an inactive frame is considered dropped.
        u32 timeout;              
        //Number of cycle - 1 SCL should be keept low (clock stretching) after having feed the data to
        //the SDA to ensure a correct propagation to other devices
        u32 tsuDat;               
        //Master mode
        //SCL low (cycle count -1)
        u32 tLow;  
        //SCL high (cycle count -1)
        u32 tHigh; 
        //Minimum time between the Stop/Drop -> Start transition
        u32 tBuf;  
    } I2c_Config;

    static void i2c_applyConfig(u32 reg, I2c_Config *config){
        write_u32(config->samplingClockDivider, reg + I2C_SAMPLING_CLOCK_DIVIDER);
        write_u32(config->timeout, reg + I2C_TIMEOUT);
        write_u32(config->tsuDat, reg + I2C_TSUDAT);
        write_u32(config->tLow, reg + I2C_TLOW);
        write_u32(config->tHigh, reg + I2C_THIGH);
        write_u32(config->tBuf, reg + I2C_TBUF);
    }
    static inline void i2c_filterEnable(u32 reg, u32 filterId, u32 config){
        write_u32(config, reg + I2C_FILTERING_CONFIG + 4*filterId);
    }
    static inline void i2c_masterStart(u32 reg){
        write_u32(I2C_MASTER_START | I2C_MASTER_START_DROPPED, reg + I2C_MASTER_STATUS);
    }
    static inline void i2c_masterRestart(u32 reg){
        i2c_masterStart(reg);
    }
    static inline void i2c_masterRecover(u32 reg){
        write_u32(I2C_MASTER_RECOVER | I2C_MASTER_RECOVER_DROPPED, reg + I2C_MASTER_STATUS);
    }
    static int i2c_masterBusy(u32 reg){
        return (read_u32(reg + I2C_MASTER_STATUS) & I2C_MASTER_BUSY) != 0;
    }
    static int i2c_masterStatus(u32 reg){
        return (read_u32(reg + I2C_MASTER_STATUS));
    }
    static void i2c_masterStartBlocking(u32 reg){
        i2c_masterStart(reg);
        while(i2c_getMasterStatus(reg) & I2C_MASTER_START);
    }
    static void i2c_masterRestartBlocking(u32 reg){
        i2c_masterStartBlocking(reg);
    }
    static inline void i2c_masterStop(u32 reg){
        write_u32(I2C_MASTER_STOP | I2C_MASTER_STOP_DROPPED, reg + I2C_MASTER_STATUS);
    }
    static void i2c_masterRecoverBlocking(u32 reg){
		int i;
        for(i = 0;i < 3;i++){
            i2c_masterRecover(reg);
            while(i2c_getMasterStatus(reg) & I2C_MASTER_RECOVER);
            if((i2c_getMasterStatus(reg) & I2C_MASTER_RECOVER_DROPPED) == 0){
                break;
            }
        }
    }
    static void i2c_masterStopWait(u32 reg){
        while(i2c_masterBusy(reg));
    }
    static inline void i2c_masterDrop(u32 reg){
        write_u32(I2C_MASTER_DROP, reg + I2C_MASTER_STATUS);
    }
    static void i2c_masterStopBlocking(u32 reg){
        i2c_masterStop(reg);
        i2c_masterStopWait(reg);
    }
    static inline void i2c_listenAck(u32 reg){
        write_u32(I2C_RX_LISTEN ,reg + I2C_RX_ACK);
    }
    static inline void i2c_txByte(u32 reg,u8 byte){
        write_u32(byte | I2C_TX_VALID | I2C_TX_ENABLE | I2C_TX_DISABLE_ON_DATA_CONFLICT, reg + I2C_TX_DATA);
    }
    static inline void i2c_txAck(u32 reg){
        write_u32(I2C_TX_VALID | I2C_TX_ENABLE, reg + I2C_TX_ACK);
    }
    static inline void i2c_txNack(u32 reg){
        write_u32(1 | I2C_TX_VALID | I2C_TX_ENABLE, reg + I2C_TX_ACK);
    }
    static void i2c_txAckWait(u32 reg){
        // int tmpI = 0;
        // while ((read_u32(reg + I2C_TX_ACK) & I2C_TX_VALID) && (tmpI++ < 500));
        while (read_u32(reg + I2C_TX_ACK) & I2C_TX_VALID);
    }
    static void i2c_txAckBlocking(u32 reg){
        i2c_txAck(reg);
        i2c_txAckWait(reg);
    }
    static void i2c_txNackBlocking(u32 reg){
        i2c_txNack(reg);
        i2c_txAckWait(reg);
    }
    static u32 i2c_rxData(u32 reg){
        return read_u32(reg + I2C_RX_DATA) & I2C_RX_VALUE;
    }
    static int i2c_rxNack(u32 reg){
        return (read_u32(reg + I2C_RX_ACK) & I2C_RX_VALUE) != 0;
    }
    static int i2c_rxAck(u32 reg){
        return (read_u32(reg + I2C_RX_ACK) & I2C_RX_VALUE) == 0;
    }
    static void i2c_txByteRepeat(u32 reg,u8 byte){
        write_u32(byte | I2C_TX_VALID | I2C_TX_ENABLE | I2C_TX_DISABLE_ON_DATA_CONFLICT | I2C_TX_REPEAT, reg + I2C_TX_DATA);
    }
    static void i2c_txNackRepeat(u32 reg){
        write_u32(1 | I2C_TX_VALID | I2C_TX_ENABLE | I2C_TX_REPEAT, reg + I2C_TX_ACK);
    }
    static inline void i2c_setFilterConfig(u32 reg, u32 filterId, u32 value){
        write_u32(value, reg + I2C_FILTERING_CONFIG + 4*filterId);
    }

#ifdef I2C_FULL_FT
    static void i2c_enableInterrupt(u32 reg, u32 value){
        write_u32(value | read_u32(reg + I2C_INTERRUPT_ENABLE), reg + I2C_INTERRUPT_ENABLE);
    }
    static void i2c_disableInterrupt(u32 reg, u32 value){
        write_u32(~value & read_u32(reg + I2C_INTERRUPT_ENABLE), reg + I2C_INTERRUPT_ENABLE);
    }
    static inline void i2c_clearInterruptFlag(u32 reg, u32 value){
        write_u32(value, reg + I2C_INTERRUPT_FLAG);
    }

    /*
     *  I2C Write data with 8-bit register address
     */
    static void i2c_writeData_b(u32 reg, u8 slaveAddr, u8 regAddr, u8 *data, u32 length){
        i2c_masterStartBlocking(reg);               // Send start sequence
        i2c_txByte(reg, slaveAddr | I2C_WRITE);     // write device address byte with write bit
        i2c_txNackBlocking(reg);                    // send nack bit
        i2c_txByte(reg, (regAddr & 0xFF));          // write a byte of register address to access
        i2c_txNackBlocking(reg);                    // send nack bit
        if(length > 1){
			int i;
            for(i = 0; i < length - 1; i++){
                    i2c_txByte(reg, data[i]);       // send 8-bit data to slave
                    i2c_txNackBlocking(reg);        // send nack bit
                }
        }
        i2c_txByte(reg, data[length-1]);            // send last 8-bit data to slave
        i2c_txNackBlocking(reg);                    // send nack bit
        i2c_masterStopBlocking(reg);                // send stop sequence
    }

    /*
     *  I2C Write data with 16-bit register address
     */
    static void i2c_writeData_w(u32 reg, u8 slaveAddr, u16 regAddr, u8 *data, u32 length){
        i2c_masterStartBlocking(reg);               // Send start sequence
        i2c_txByte(reg, slaveAddr | I2C_WRITE);     // write device address byte with write bit
        i2c_txNackBlocking(reg);                    // send nack bit
        i2c_txByte(reg, ((regAddr >>8) & 0xFF));    // send MSB of register address to access
        i2c_txNackBlocking(reg);                    // send nack bit
        i2c_txByte(reg, (regAddr & 0xFF));          // send LSB of register address to access
        i2c_txNackBlocking(reg);                    // send nack bit
        if(length > 1){
			int i;
            for(i = 0; i < length - 1; i++){
                    i2c_txByte(reg, data[i]);       // send 8-bit data to slave
                    i2c_txNackBlocking(reg);        // send nack bit
                }
        }
        i2c_txByte(reg, data[length-1]);            // send last 8-bit data to slave
        i2c_txNackBlocking(reg);                    // send nack bit
        i2c_masterStopBlocking(reg);                // send stop sequence
    }

    /*
     *  I2C Read data with 8-bit register address
     */
    static void i2c_readData_b(u32 reg, u8 slaveAddr, u8 regAddr, u8 *data , u32 length){
        i2c_masterStartBlocking(reg);               // Send start sequence
        i2c_txByte(reg, slaveAddr|I2C_WRITE);       // write device address byte with write bit
        i2c_txNackBlocking(reg);                    // send nack bit
        i2c_txByte(reg, (regAddr & 0xFF));          // write second byte address
        i2c_txNackBlocking(reg);                    // send nack bit
        i2c_masterRestartBlocking(reg);             // send restart sequence and wait for it to complete
        i2c_txByte(reg, slaveAddr|I2C_READ);        // write device address byt ewith read bit
        i2c_txNackBlocking(reg);                    // send nack bit
        if(length > 1){
			int i;
            for(i = 0; i < length - 1; i++){
                i2c_txByte(reg, 0xFF);              // send 0xFF (Release SDA line) to the slave while generate 8-bit SCL pulses
                i2c_txAckBlocking(reg);             // send ack bit to ask slave to continue send the next byte
                data[i] = i2c_rxData(reg);          // read the data from rx data register and place it into data array
            }
        }
        i2c_txByte(reg, 0xFF);                      // send 0xFF (Release SDA line) to the slave while generate 8-bit SCL pulses
        i2c_txNackBlocking(reg);                    // send nack bit
        data[length-1] = i2c_rxData(reg);           // read the data from rx data register and place it into last data array
        i2c_masterStopBlocking(reg);                // send stop sequence
    }

    /*
     *  I2C Read data with 16-bit register address
     */
    static void i2c_readData_w(u32 reg, u8 slaveAddr, u16 regAddr, u8 *data , u32 length){
        i2c_masterStartBlocking(reg);               // Send start sequence
        i2c_txByte(reg, slaveAddr|I2C_WRITE);       // write device address byte with write bit
        i2c_txNackBlocking(reg);                    // send nack bit
        i2c_txByte(reg, ((regAddr >>8) & 0xFF));    // write first byte address
        i2c_txNackBlocking(reg);                    // send nack bit
        i2c_txByte(reg, (regAddr & 0xFF));          // write second byte address
        i2c_txNackBlocking(reg);                    // send nack bit
        i2c_masterRestartBlocking(reg);             // send restart sequence and wait for it to complete
        i2c_txByte(reg, slaveAddr|I2C_READ);        // write device address byt ewith read bit
        i2c_txNackBlocking(reg);                    // send nack bit
        if(length > 1){
			int i;
            for(i = 0; i < length - 1; i++){
                i2c_txByte(reg, 0xFF);              // send 0xFF (Release SDA line) to the slave while generate 8-bit SCL pulses
                i2c_txAckBlocking(reg);             // send ack bit to ask slave to continue send the next byte
                data[i] = i2c_rxData(reg);          // read the data from rx data register and place it into data array
            }
        }
        i2c_txByte(reg, 0xFF);                      // send 0xFF (Release SDA line) to the slave while generate 8-bit SCL pulses
        i2c_txNackBlocking(reg);                    // send nack bit
        data[length-1] = i2c_rxData(reg);           // read the data from rx data register and place it into last data array
        i2c_masterStopBlocking(reg);                // send stop sequence
    }

#endif



