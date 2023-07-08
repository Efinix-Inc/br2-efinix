// SPDX-License-Identifier: GPL-2.0-only 
/*
 * Efinix I2C bus driver
 *
 * Sapphire SoC Datasheet: https://www.efinixinc.com/docs/riscv-sapphire-ds-v3.0.pdf
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/i2c.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/slab.h>
#include <linux/moduleparam.h>
#include <linux/kdev_t.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>

/* EFX I2C offset registers */
#define EFX_I2C_TX_DATA			0x00
#define EFX_I2C_TX_ACK			0x04
#define EFX_I2C_RX_DATA			0x08
#define EFX_I2C_RX_ACK			0x0c
#define EFX_I2C_IRQ			0x20
#define EFX_I2C_IRQ_CLR			0x24
#define EFX_I2C_SAMPLING_CLOCK_DIV	0x28
#define EFX_I2C_TIMEOUT			0x2c
#define EFX_I2C_TSU_DATA		0x30
#define EFX_I2C_MASTER_STATUS		0x40
#define EFX_I2C_TLOW			0x50
#define EFX_I2C_THIGH			0x54
#define EFX_I2C_TBUF			0x58
#define EFX_I2C_HIT_CONTEXT		0x80
#define EFX_I2C_FILTERING_STATUS	0x84
#define EFX_I2C_FILTERING_CONFIG0	0x88
#define EFX_I2C_FILTERING_CONFIG1	0x8c

/* EFX I2C TxDATA Register */
#define EFX_I2C_TX_DATA_VALUE			BIT(7)
#define EFX_I2C_TX_DATA_VALID			BIT(8)
#define EFX_I2C_TX_DATA_ENABLE			BIT(9)
#define EFX_I2C_TX_DATA_REPEAT			BIT(10)
#define EFX_I2C_TX_DATA_DISABLE_DATA_CONFLICT	BIT(11)

/* EFX I2C Tx ACK Register */
#define EFX_I2C_TX_ACK_VALUE			BIT(0)
#define EFX_I2C_TX_ACK_VALID			BIT(8)
#define EFX_I2C_TX_ACK_ENABLE			BIT(9)
#define EFX_I2C_TX_ACK_REPEAT			BIT(10)
#define EFX_I2C_TX_ACK_DISABLE_DATA_CONFLICT	BIT(11)

/* EFX I2C Rx Data register */
#define EFX_I2C_RX_DATA_VALUE			BIT(7)
#define EFX_I2C_RX_DATA_VALID			BIT(8)
#define EFX_I2C_RX_DATA_LISTEN			BIT(9)

/* EFX I2C Rx Ack Register */
#define EFX_I2C_RX_ACK_VALUE			BIT(0)
#define EFX_I2C_RX_ACK_VALID			BIT(8)
#define EFX_I2C_RX_ACK_LISTEN			BIT(9)

/* EFX I2C Interrupt Register */
#define EFX_I2C_IRQ_RX_DATA_ENABLE		BIT(0)
#define EFX_I2C_IRQ_RX_ACK_ENABLE		BIT(1)
#define EFX_I2C_IRQ_TX_DATA_ENABLE		BIT(2)
#define EFX_I2C_IRQ_TX_ACK_ENABLE		BIT(3)
#define EFX_I2C_IRQ_START_IRQ_ENABLE		BIT(4)
#define EFX_I2C_IRQ_RESTART_IRQ_ENABLE		BIT(5)
#define EFX_I2C_IRQ_END_IRQ_ENABLE		BIT(6)
#define EFX_I2C_IRQ_DROP_ENABLE			BIT(7)
#define EFX_I2C_IRQ_CLK_GEN_BUSY_ENABLE		BIT(16)
#define EFX_I2C_IRQ_FILTER_ENABLE		BIT(17)

/* EFX I2C Interrupt Clears Register */
#define EFX_I2C_IRQ_CLR_START			BIT(4)
#define EFX_I2C_IRQ_CLR_RESTART			BIT(5)
#define EFX_I2C_IRQ_CLR_END			BIT(6)
#define EFX_I2C_IRQ_CLR_DROP			BIT(7)
#define EFX_I2C_IRQ_CLR_CLK_GEN_BUSY		BIT(16)
#define EFX_I2C_IRQ_CLR_FILTER			BIT(17)

/* EFX I2C Master Status Register */
#define EFX_I2C_MASTER_STATUS_BUSY		BIT(0)
#define EFX_I2C_MASTER_STATUS_START		BIT(4)
#define EFX_I2C_MASTER_STATUS_STOP		BIT(5)
#define EFX_I2C_MASTER_STATUS_DROP		BIT(6)

/* EFX I2C Filtering Configuration 0 Register */
#define EFX_I2C_FILTERING_CONFIG0_ENABLE	BIT(15)

#define EFX_I2C_READ_MASK			0xff

#undef pr_fmt
#define pr_fmt(fmt) "%s: " fmt, __func__

#define DRIVER_NAME	"efx-i2c"
#define NACK	1
#define ACK	0

/**
 * struct efx_i2c_msg - client specific data
 */
struct efx_i2c_msg {
	u8 addr;
	u16 count;
	u8 *buf;
	unsigned short flags;
};

/**
 * struct efx_i2c_dev - private data of the controller
 */
struct efx_i2c_dev {
	void __iomem *base;
	struct i2c_adapter adap;
	struct device *dev;
	struct efx_i2c_msg msg;
	u32 clk_rate;
	u32 peripheral_clk;
	struct clk *clk;
	u8 mode;
	int irq;
	struct i2c_client *slave;
	spinlock_t lock;
	u32 sampling_clk_div;
};

enum {
        I2C_MODE_STANDARD,
        I2C_MODE_FAST,
        I2C_MODE_FAST_PLUS,
};

/**
 * struct efx_i2c_spec - private i2c specification timing
 * @speed: I2C bus speed (Hz)
 * @tsu_data: Min data setup time (Hz)
 * @tbuf: Min Min delay between start and stop (Hz)
 * @thigh: Min SCL in high state (Hz)
 * @tlow: Min SCL in low state (Hz)
 * @timeout: Timeout of I2C transaction (Hz)
 */
struct efx_i2c_spec {
	u32 speed;
	u32 tsu_data;
	u32 tbuf;
	u32 thigh;
	u32 tlow;
	u32 timeout;
};

enum state {
	IDLE,
	WRITE_ADDR,
	READ_DATA0,
	WRITE_DATA0,
};

static enum state state = IDLE;

static inline int efx_i2c_rx_ack(struct efx_i2c_dev *i2c_dev);
static inline int efx_i2c_rx_nack(struct efx_i2c_dev *i2c_dev);

static inline void efx_i2c_set_bits(void __iomem *reg, u32 mask)
{
	writel(readl(reg) | mask, reg);
}

static inline void efx_i2c_clr_bits(void __iomem *reg, u32 mask)
{
	writel(readl(reg) & ~mask, reg);
}

static inline void efx_i2c_disable_interrupt(struct efx_i2c_dev *i2c_dev, u32 value)
{
	efx_i2c_clr_bits(i2c_dev->base + EFX_I2C_IRQ, value);
}

static inline void efx_i2c_enable_interrupt(struct efx_i2c_dev *i2c_dev, u32 value)
{
	writel(value | readl(i2c_dev->base + EFX_I2C_IRQ), i2c_dev->base + EFX_I2C_IRQ);
}

static inline void efx_i2c_clear_irq_flag(struct efx_i2c_dev *i2c_dev, u32 value)
{
	writel(value, i2c_dev->base + EFX_I2C_IRQ_CLR);
}

/**
 * efx_i2c_write_byte - Write a byte in the data register
 */
static inline void efx_i2c_write_byte(struct efx_i2c_dev *i2c_dev, u8 byte)
{
        u32 mask;

        mask = byte | EFX_I2C_TX_DATA_ENABLE | EFX_I2C_TX_DATA_VALID
		| EFX_I2C_TX_DATA_DISABLE_DATA_CONFLICT;
        writel(mask, i2c_dev->base + EFX_I2C_TX_DATA);
}

static inline void efx_i2c_tx_byte_repeat(struct efx_i2c_dev *i2c_dev, u8 byte)
{
	byte |=  EFX_I2C_TX_DATA_REPEAT | EFX_I2C_TX_DATA_DISABLE_DATA_CONFLICT;
	efx_i2c_write_byte(i2c_dev, byte);
}

static inline void efx_i2c_tx_ack(struct efx_i2c_dev *i2c_dev)
{
	writel(EFX_I2C_TX_ACK_VALID | EFX_I2C_TX_ACK_ENABLE,
		i2c_dev->base + EFX_I2C_TX_ACK);
}

static inline void efx_i2c_tx_nack(struct efx_i2c_dev *i2c_dev)
{
	writel(0x1 | EFX_I2C_TX_ACK_VALID | EFX_I2C_TX_ACK_ENABLE,
		i2c_dev->base + EFX_I2C_TX_ACK);
}

static inline void efx_i2c_tx_nack_repeat(struct efx_i2c_dev *i2c_dev)
{
	writel(0x1 | EFX_I2C_TX_ACK_VALID | EFX_I2C_TX_ACK_ENABLE
		| EFX_I2C_TX_DATA_DISABLE_DATA_CONFLICT
		| EFX_I2C_TX_DATA_REPEAT,
		i2c_dev->base + EFX_I2C_TX_ACK);
}

static irqreturn_t efx_i2c_interrupt_slave(int irq, void *dev_id)
{
        struct efx_i2c_dev *i2c_dev = (struct efx_i2c_dev *)dev_id;
	unsigned long flags;
        u32 irq_status;
        u8 value = 0x0;
        u8 release = 0xFF;

	spin_lock_irqsave(&i2c_dev->lock, flags);
	irq_status = readl(i2c_dev->base + EFX_I2C_IRQ_CLR);

	if (irq_status & EFX_I2C_IRQ_CLR_FILTER) {
		state = IDLE;
		efx_i2c_disable_interrupt(i2c_dev, EFX_I2C_IRQ_TX_DATA_ENABLE);
	}

	switch (state) {
	case IDLE:
		if (readl(i2c_dev->base + EFX_I2C_HIT_CONTEXT) == 0x1) {
			if (readl(i2c_dev->base + EFX_I2C_FILTERING_STATUS) == 0x0) {
				// master write
				i2c_slave_event(i2c_dev->slave,
						I2C_SLAVE_WRITE_REQUESTED, &value);
				efx_i2c_tx_ack(i2c_dev);
				efx_i2c_write_byte(i2c_dev, release);
				efx_i2c_enable_interrupt(i2c_dev,
						EFX_I2C_IRQ_TX_DATA_ENABLE);
				state = WRITE_ADDR;
			} else {
				// master read
				i2c_slave_event(i2c_dev->slave,
						I2C_SLAVE_READ_REQUESTED, &value);
				efx_i2c_tx_ack(i2c_dev);
				// send first read value to master
				efx_i2c_write_byte(i2c_dev, value);
				efx_i2c_enable_interrupt(i2c_dev,
						EFX_I2C_IRQ_TX_DATA_ENABLE);
				state = READ_DATA0;
			}

		} else {
			efx_i2c_disable_interrupt(i2c_dev, EFX_I2C_IRQ_TX_DATA_ENABLE);
			state = IDLE;

		}
		efx_i2c_clear_irq_flag(i2c_dev, EFX_I2C_IRQ_CLR_FILTER);
		break;

	case WRITE_ADDR:
		efx_i2c_tx_ack(i2c_dev);
		efx_i2c_write_byte(i2c_dev, release);
		value = readl(i2c_dev->base + EFX_I2C_RX_DATA) & 0xff;
		i2c_slave_event(i2c_dev->slave,
				I2C_SLAVE_WRITE_RECEIVED, &value);
		state = WRITE_DATA0;
		break;

	case READ_DATA0:
		i2c_slave_event(i2c_dev->slave,
				I2C_SLAVE_READ_PROCESSED, &value);
		efx_i2c_tx_nack(i2c_dev);
		efx_i2c_write_byte(i2c_dev, value);
		state = READ_DATA0;
		break;

	case WRITE_DATA0:
		efx_i2c_tx_ack(i2c_dev);
		efx_i2c_write_byte(i2c_dev, release);
		value = readl(i2c_dev->base + EFX_I2C_RX_DATA) & 0xff;
		i2c_slave_event(i2c_dev->slave,
				I2C_SLAVE_WRITE_RECEIVED, &value);

		state = WRITE_DATA0;
		break;
	}

	spin_unlock_irqrestore(&i2c_dev->lock, flags);

	return IRQ_HANDLED;
}

static void efx_i2c_terminate_xfer(struct efx_i2c_dev *i2c_dev)
{
        void __iomem *reg;

        reg = i2c_dev->base + EFX_I2C_MASTER_STATUS;
	efx_i2c_set_bits(reg, EFX_I2C_MASTER_STATUS_STOP);
}

/*
 * efx_i2c_rx_ack: check for slave ack
 *
 * return: 0 if slave ack. Else -EIO.
 */
static inline int efx_i2c_rx_ack(struct efx_i2c_dev *i2c_dev)
{
	if ((readl(i2c_dev->base + EFX_I2C_RX_ACK) & 0xFF) == 0)
		return 0;

	else
		return -EIO;
}

static inline int efx_i2c_rx_nack(struct efx_i2c_dev *i2c_dev)
{
	if ((readl(i2c_dev->base + EFX_I2C_RX_ACK) & 0xFF) != 0)
		return 0;

	else
		return -EIO;
}


/**
 * efx_i2c_tx_ack_wait - send tx ack/nack and wait for slave device to respond
 * @i2c_dev: pointer to current i2c device
 * @flag: can be ACK or NACK
 *
 * return: 0 on success or -ETIMEDOUT on failure
 */
static inline int efx_i2c_tx_ack_wait(struct efx_i2c_dev *i2c_dev, u32 flag)
{
        u32 status;

        efx_i2c_set_bits(i2c_dev->base + EFX_I2C_TX_ACK,
                        EFX_I2C_TX_ACK_ENABLE | EFX_I2C_TX_ACK_VALID | flag);

	return readl_relaxed_poll_timeout(i2c_dev->base + EFX_I2C_TX_ACK,
			status,
			((status & EFX_I2C_TX_ACK_VALID) == 0x0),
			0, 1000);

}

static inline void __efx_i2c_start(struct efx_i2c_dev *i2c_dev)
{
	efx_i2c_set_bits(i2c_dev->base + EFX_I2C_MASTER_STATUS,
			 EFX_I2C_MASTER_STATUS_START);
}

static inline int efx_i2c_master_busy(struct efx_i2c_dev *i2c_dev)
{
	void __iomem *reg;
	u32 status;

	reg = i2c_dev->base + EFX_I2C_MASTER_STATUS;
	status = readl(reg) & EFX_I2C_MASTER_STATUS_BUSY;

	return status;
}

static struct efx_i2c_spec efx_i2c_specs[] = {
	{
		.speed = I2C_MAX_STANDARD_MODE_FREQ,
		.tsu_data = 2000000,
		.tbuf = 200000,
		.thigh = 200000,
		.tlow = 200000,
		.timeout = 1000,
	},
	{
		.speed = I2C_MAX_FAST_MODE_FREQ,
		.tsu_data = 2000000,
		.tbuf = 666667,
		.thigh = 1000000,
		.tlow = 666667,
		.timeout = 1000,
	},
	{
		.speed = I2C_MAX_FAST_MODE_PLUS_FREQ,
		.tsu_data = 20000000,
		.tbuf = 1666667,
		.thigh = 2500000,
		.tlow = 1666667,
		.timeout = 1000,
	}
};

/**
 * exf_i2c_hw_config - Initialize I2C device
 */
static int efx_i2c_hw_config(struct efx_i2c_dev *i2c_dev)
{
        u32 clk_rate = i2c_dev->clk_rate;
	u32 peripheral_clk = i2c_dev->peripheral_clk;
        u32 tsu_data;
        u32 thigh;
        u32 tlow;
        u32 tbuf;
        u32 timeout;
	struct efx_i2c_spec spec;

        if (clk_rate <= 0)
                return -ENODEV;

	switch(i2c_dev->mode) {
	case I2C_MODE_STANDARD:
		spec = efx_i2c_specs[I2C_MODE_STANDARD];
		break;
	case I2C_MODE_FAST:
		spec = efx_i2c_specs[I2C_MODE_FAST];
		break;
	case I2C_MODE_FAST_PLUS:
		spec = efx_i2c_specs[I2C_MODE_FAST_PLUS];
		break;
	default:
		spec = efx_i2c_specs[I2C_MODE_STANDARD];
	}

	tsu_data = peripheral_clk / spec.tsu_data;
	thigh = peripheral_clk / spec.thigh;
	tlow = peripheral_clk / spec.tlow;
	tbuf = peripheral_clk / spec.tbuf;
	timeout = peripheral_clk / spec.timeout;

	writel(i2c_dev->sampling_clk_div, i2c_dev->base + EFX_I2C_SAMPLING_CLOCK_DIV);
	writel(timeout, i2c_dev->base + EFX_I2C_TIMEOUT);
	writel(tsu_data, i2c_dev->base + EFX_I2C_TSU_DATA);
	writel(tlow, i2c_dev->base + EFX_I2C_TLOW);
	writel(thigh, i2c_dev->base + EFX_I2C_THIGH);
	writel(tbuf, i2c_dev->base + EFX_I2C_TBUF);

	return 0;
}

static int efx_i2c_master_write(struct efx_i2c_dev *i2c_dev, u8 *buf)
{
	int ret;

	efx_i2c_write_byte(i2c_dev, *buf);
	ret = efx_i2c_tx_ack_wait(i2c_dev, NACK);
	if (ret)
		return ret;

	return 0;
}

static int efx_i2c_start(struct efx_i2c_dev *i2c_dev, struct efx_i2c_msg *i2c_msg)
{
	int ret;

	__efx_i2c_start(i2c_dev);
	ret = efx_i2c_master_write(i2c_dev, &i2c_msg->addr);
	if (ret)
		return ret;

	ret = efx_i2c_rx_ack(i2c_dev);
	if (ret)
		return ret;

	return 0;
}

/** efx_i2c_master_read - i2c master read operation
 * @i2c_dev: pointer to i2c device
 * @i2c_msg: i2c metadata for the transaction
 * @buf: pointer of data buffer to be written to after i2c read ops
 *
 * return: number of bytes has been read
 */
static int efx_i2c_master_read(struct efx_i2c_dev *i2c_dev, struct efx_i2c_msg *i2c_msg)
{
	int ret;

        // need to send 0xff to get the data from i2c slave
        efx_i2c_write_byte(i2c_dev, EFX_I2C_READ_MASK);

        if (i2c_msg->count == 1) // last msg
                ret = efx_i2c_tx_ack_wait(i2c_dev, NACK);
        else
                ret = efx_i2c_tx_ack_wait(i2c_dev, ACK);

	if (ret < 0)
		return ret;

        *(i2c_msg->buf) = readl(i2c_dev->base + EFX_I2C_RX_DATA) & 0xff;

	return 0;
}

/*
 * efx_i2c_xfer_msg: i2c master start the transfer
 *
 * return: 0 successfully transfer or negative number on failure
 */
static int efx_i2c_xfer_msg(struct efx_i2c_dev *i2c_dev, struct i2c_msg *msgs,
			    int num, bool is_first, bool is_last)
{
	struct efx_i2c_msg *i2c_msg = &i2c_dev->msg;
	int ret;

	i2c_msg->addr = i2c_8bit_addr_from_msg(msgs);
	i2c_msg->buf = msgs->buf;
	i2c_msg->count = msgs->len;
	i2c_msg->flags = msgs->flags;

	if (is_first) {
		// start
		if (!efx_i2c_master_busy(i2c_dev)) {
			ret = efx_i2c_start(i2c_dev, i2c_msg);
			if (ret)
				goto i2c_terminate;
		} else
			return -EBUSY;
	}

	/*
	 * SMBus protocol require repeated start for read operation while
	 * I2C protocol only perform stop and start
	 */
	if ((num > 1) && (i2c_msg->flags & I2C_M_RD)) {
		ret = efx_i2c_start(i2c_dev, i2c_msg);
		if (ret)
			goto i2c_terminate;
	}

	while (i2c_msg->count) {
		if (i2c_msg->flags & I2C_M_RD) {
			ret = efx_i2c_master_read(i2c_dev, i2c_msg);
			if (ret)
				goto i2c_terminate;
		} else {
			ret = efx_i2c_master_write(i2c_dev, i2c_msg->buf);
			if (ret)
				goto i2c_terminate;
                }

		if (i2c_msg->count == 1) {
			if (msgs->len > 1)
				efx_i2c_rx_nack(i2c_dev);
			else
				efx_i2c_rx_ack(i2c_dev);
		} else
			efx_i2c_rx_ack(i2c_dev);

		// increase the pointer to the next buffer
		i2c_msg->buf = ++msgs->buf;
		i2c_msg->count--;
	}

	if (is_last) {
		// stop
		if (efx_i2c_master_busy(i2c_dev))
			goto i2c_terminate;
		else
			return -EIO;
	}

	return 0;

i2c_terminate:
	efx_i2c_terminate_xfer(i2c_dev);

	return ret;
}

static int efx_i2c_xfer(struct i2c_adapter *adap, struct i2c_msg msgs[],
			int num)
{
	struct efx_i2c_dev *i2c_dev = i2c_get_adapdata(adap);
	int i;
	int ret;

	for (i = 0; i < num; i++) {
		ret = efx_i2c_xfer_msg(i2c_dev, &msgs[i], num,
				i == 0, i == (num - 1));
	}

	return (ret < 0) ? ret : num;
}

static u32 efx_i2c_func(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL | I2C_FUNC_SLAVE;
}

static int efx_i2c_reg_slave(struct i2c_client *slave)
{
	struct efx_i2c_dev *i2c_dev = i2c_get_adapdata(slave->adapter);

	if (i2c_dev->slave)
		return -EBUSY;

	/* I2C 10-bit client address not support for now */
	if (slave->flags & I2C_CLIENT_TEN)
		return -EAFNOSUPPORT;

	i2c_dev->slave = slave;

	pr_info("Entered I2C slave mode (slave addr = 0x%x\n", slave->addr);

	/* Set filter configuration */
	writel(slave->addr | EFX_I2C_FILTERING_CONFIG0_ENABLE,
		i2c_dev->base + EFX_I2C_FILTERING_CONFIG0);

	/* Enable slave interrupt register */
	efx_i2c_enable_interrupt(i2c_dev, EFX_I2C_IRQ_FILTER_ENABLE);

	return 0;
}

static int efx_i2c_unreg_slave(struct i2c_client *slave)
{
	struct efx_i2c_dev *i2c_dev = i2c_get_adapdata(slave->adapter);

	WARN_ON(!i2c_dev->slave);
	i2c_dev->slave = NULL;
	pr_info("leaving i2c slave mode\n");

	return 0;
}

static struct i2c_algorithm efx_i2c_algo = {
	.master_xfer = efx_i2c_xfer,
	.functionality = efx_i2c_func,
	.reg_slave = efx_i2c_reg_slave,
	.unreg_slave = efx_i2c_unreg_slave,
};

static int efx_i2c_probe(struct platform_device *pdev)
{
	struct efx_i2c_dev *i2c_dev;
	struct resource *res;
	struct i2c_adapter *adap;
	struct device_node *np = pdev->dev.of_node;
	struct device *dev = &pdev->dev;
	int ret;

	i2c_dev = devm_kzalloc(dev, sizeof(*i2c_dev), GFP_KERNEL);
	if (!i2c_dev)
		return -ENOMEM;

	i2c_dev->dev = dev;
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	i2c_dev->base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(i2c_dev->base))
		return PTR_ERR(i2c_dev->base);

	i2c_dev->mode = I2C_MODE_STANDARD;
	ret = of_property_read_u32(np, "clock-frequency", &i2c_dev->clk_rate);
	if (ret) {
		dev_err(dev, "Property clock-frequency is missing from i2c device tree");
		return ret;
	} else {
		if (i2c_dev->clk_rate == 400000)
			i2c_dev->mode = I2C_MODE_FAST;
		else if (i2c_dev->clk_rate == 1000000)
			i2c_dev->mode = I2C_MODE_FAST_PLUS;
	}

	ret = of_property_read_u32(np, "efx,i2c-sampling-clock-divider",
			&i2c_dev->sampling_clk_div);
	if (ret)
		i2c_dev->sampling_clk_div = 3;

	i2c_dev->clk = devm_clk_get(dev, NULL);
	if (IS_ERR(i2c_dev->clk)) {
		dev_err(dev, "Failed to get peripheral clock");
		return PTR_ERR(i2c_dev->clk);
	}
	i2c_dev->peripheral_clk = clk_get_rate(i2c_dev->clk);
	i2c_dev->irq = platform_get_irq(pdev, 0);
	ret = devm_request_irq(dev, i2c_dev->irq, efx_i2c_interrupt_slave,
			       0, DRIVER_NAME, i2c_dev);
	if (ret) {
		dev_err(dev, "Cannot get I2C irq %d", i2c_dev->irq);
		return ret;
	}

	ret = efx_i2c_hw_config(i2c_dev);
	if (ret) {
		dev_err(dev, "Failed to configure I2C controller\n");
		return ret;
	}

	adap = &i2c_dev->adap;
	i2c_set_adapdata(adap, i2c_dev);
	snprintf(adap->name, sizeof(adap->name), "EFX I2C (%pa)", &res->start);
	adap->owner = THIS_MODULE;
	adap->timeout = 2 *HZ;
	adap->retries = 3;
	adap->algo = &efx_i2c_algo;
	adap->dev.parent = &pdev->dev;
	adap->dev.of_node = pdev->dev.of_node;
	i2c_detect_slave_mode(dev);

	ret = i2c_add_adapter(adap);
	if (ret)
		return ret;

	platform_set_drvdata(pdev, i2c_dev);

        return 0;
}

static int efx_i2c_remove(struct platform_device *pdev)
{
	struct efx_i2c_dev *i2c_dev = platform_get_drvdata(pdev);

	i2c_del_adapter(&i2c_dev->adap);
	return 0;
}

static struct of_device_id efx_i2c_match[] = {
	{.compatible = "efinix,efx-i2c"},
	{}
};

static struct platform_driver efx_i2c_driver = {
	.probe = efx_i2c_probe,
	.remove = efx_i2c_remove,
	.driver = {
		.name = DRIVER_NAME,
		.of_match_table = of_match_ptr(efx_i2c_match),
	},
};

module_platform_driver(efx_i2c_driver);

MODULE_AUTHOR("Mohamad Noor Alim Hussin <mnalim@efinixinc.com>");
MODULE_DESCRIPTION("Efinix I2C bus driver");
MODULE_LICENSE("GPL v2");
