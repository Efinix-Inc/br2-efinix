#include <stdint.h>

#ifndef SPI_H
#define SPI_H

#define WRITE_STATUS_REG	0x01
#define READ_CMD		0x03
#define WRITE_CMD		0x02
#define READ_STATUS_REG		0x05
#define WRITE_ENABLE_CMD	0x06
#define GLOBAL_LOCK		0x7E
#define GLOBAL_UNLOCK		0x98
#define SECTOR_ERASE		0x20
#define SPI_ID			0xAB
#define READ_SFDP		0x5A

/* status register bits field */
#define SRP1	0x80 /* Status Register Protect bit */
#define WEL	0x02 /* Write Enable Latch bit */
#define WIP	0x01 /* Not currently writing bit */

/*
 * Configuration for the SPI device.
 */
struct spi_device {
	char *filename; /**< Path of the SPI bus, eg: /dev/spidev1.0 */
	uint8_t mode; /**< Mode of the SPI bus */
	uint8_t bpw; /**< Bits-per-word of the SPI bus */
	uint32_t speed; /**< Speed of the SPI bus */
	uint8_t cs_change;

	int fd; /**< File descriptor for the SPI bus */
};

int spi_start(struct spi_device *dev);
int spi_transfer(struct spi_device *dev, uint8_t *write_buf, uint8_t *read_buf, uint32_t buf_len);
void spi_stop(struct spi_device *dev);
#endif // SPI_H
