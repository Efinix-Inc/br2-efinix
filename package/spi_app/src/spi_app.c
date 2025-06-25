#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "spi.h"

int verbose = 0;

void help()
{
	printf("help\n");
	printf("-a	Address to read/write of SPI flash\n");
	printf("-d	Set SPI device name. Example, /dev/spidev0.0\n");
	printf("-l	Length of data to read in byte\n");
	printf("-s	Set SPI clock frequency in hertz\n");
	printf("-r	Read mode\n");
	printf("-w	Write mode\n");
	printf("-m	Message to write to SPI flash\n");
	printf("-v	Show verbose debug message\n");
	printf("-h	show this message\n");
	printf("\nExample usage\n");
	printf("Reading SPI flash\n");
	printf("spi_app -r -a 0x800000 -l 4\n\n");
	printf("Writing to SPI flash\n");
	printf("spi_app -w -a 0x800000 -m \"hello world\"\n");
	printf("\n");
}

void wait_busy(struct spi_device *dev)
{
        uint16_t timeout = 0;
        uint8_t out;
        uint8_t cmd = READ_STATUS_REG;

        while(1) {
                sleep(0.5);
                spi_transfer(dev, &cmd, &out, 1);
                if ((out & WIP) == 0x0)
                        break;
                timeout++;
                if (timeout >= 400) {
                        printf("Timeout ...\n");
                        break;
                }
        }
}

int spi_tx_cmd(struct spi_device *dev, uint8_t reg)
{
	uint8_t cs = dev->cs_change;
	int rc;

	dev->cs_change = 0;
	rc = spi_transfer(dev, &reg, NULL, sizeof(reg));
	dev->cs_change = cs;
	
	return rc;
}

int ncmd_reg(struct spi_device *dev, uint8_t reg, uint8_t *buf, int buf_len)
{
        uint8_t *full_buf;
        int full_buf_len;
        int rc;

        /*
         * Allocate a buffer that contains the command and
         * the register address as the first element.
         */
        full_buf_len = buf_len + 3;
        full_buf = malloc(full_buf_len * sizeof(*full_buf));
	if (full_buf == NULL) {
		printf("Failed to allocated memory\n");
		return -1;
	}
        full_buf[0] = reg;

        rc = spi_transfer(dev, full_buf, full_buf, full_buf_len);

        memcpy(buf, full_buf, buf_len);

	free(full_buf);

        return rc;
}

int cmd_reg(struct spi_device *dev, uint8_t reg)
{
        uint8_t data = 0;
        ncmd_reg(dev, reg, &data, 1);
	
        return data;
}

int enable_write_latch(struct spi_device *dev)
{
	return spi_tx_cmd(dev, WRITE_ENABLE_CMD);
}

int global_lock(struct spi_device *dev)
{
	int rc;

	rc = enable_write_latch(dev);
	if (rc < 0)
		return rc;

	return spi_tx_cmd(dev, GLOBAL_LOCK);
}

int global_unlock(struct spi_device *dev)
{
	int rc;

	rc = enable_write_latch(dev);
	if (rc < 0)
		return rc;

	return spi_tx_cmd(dev, GLOBAL_UNLOCK);
}

int sector_erase(struct spi_device *dev, uint32_t addr)
{
	uint8_t tx_buf[4] = {0};
	int rc = 0;
	
	rc = enable_write_latch(dev);
	if (rc < 0)
		return rc;
	
	tx_buf[0] = SECTOR_ERASE;
	tx_buf[1] = (addr >> 16) & 0xFF;
	tx_buf[2] = (addr >> 8) & 0xFF;
	tx_buf[3] = addr & 0xFF;
	
	if (verbose) {
		printf("%s: debug tx_buf\n", __func__);
		for (int i = 0; i < 4; i++) {
			printf("%02x ", tx_buf[i]);
		}
		printf("\n");
	}

	rc = spi_transfer(dev, tx_buf, NULL, sizeof(tx_buf));
	if (rc < 0) {
		printf("Error: Failed to erase sector\n");
		return rc;
	}
	
	wait_busy(dev);
	
	return rc;
}

int spi_flash_write(struct spi_device *dev, uint32_t addr, uint8_t *data, int len)
{
	int rc = 0;
	uint8_t *tx_buf;
	int tx_len;
	
	rc = global_unlock(dev);
	if (rc < 0) {
		printf("Error: Failed to unlock the SPI flash\n");
		return rc;
	}
	
	rc = sector_erase(dev, addr);
	if (rc < 0) {
		printf("Error: Failed to erase SPI flash\n");
		return rc;
	}
	
	rc = enable_write_latch(dev);
	if (rc < 0) {
		printf("Error: Failed to enable write latch\n");
		return rc;
	}
	
	tx_len = len + 4; // 4 means 1 byte command + 3 bytes address
	tx_buf = malloc(tx_len * sizeof(*tx_buf));
	if (tx_buf == NULL) {
		printf("Error: Failed to allocate memory\n");
		return -1;
	}
	
	tx_buf[0] = WRITE_CMD;
	tx_buf[1] = (addr >> 16) & 0xFF;
	tx_buf[2] = (addr >> 8) & 0xFF;
	tx_buf[3] = addr & 0xFF;
	
	memcpy(tx_buf + 4, data, len);
	
	if (verbose) {
		printf("%s: debug tx_buf\n", __func__);
		for (int i = 0; i < tx_len; i++) {
			printf("%02x ", tx_buf[i]);
		}
		printf("\n");
	}
	
	for (int i = 0; i < len; i++) {
		printf("%02x ", data[i]);
	}
	printf("\n\n");

	rc = spi_transfer(dev, tx_buf, NULL, tx_len);
	if (rc < 0) {
		printf("Error: Failed to write to SPI flash\n");
	}
	
	wait_busy(dev);
	rc = global_lock(dev);
	if (rc < 0) {
		printf("Error: Failed to lock SPI flash\n");
	}
	free(tx_buf);
	
	return rc;
}

void spi_flash_read(struct spi_device *dev, uint32_t addr, uint32_t len)
{
	int j = 0;
	uint32_t naddr;
        uint8_t tx[4] = {0};
        uint8_t rx = 0;

        printf("Reading SPI Flash\n\n");
        printf(" Address:  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
        for (naddr = addr; naddr < addr + len; naddr++) {
                tx[0] = READ_CMD;
                tx[1] = (naddr >> 16) & 0xFF;
                tx[2] = (naddr >> 8) & 0xFF;
                tx[3] = naddr & 0xFF;

                spi_transfer(dev, tx, &rx, sizeof(tx));

		if (j == 0) {
			printf("0x%x: ", naddr);
		}
		printf("%02x ", rx);
		if (++j == 16) {
			printf("\n");
			j = 0;
		}

	}
	printf("\n\n");
}

int main(int argc, char **argv)
{
	uint8_t value;
	struct spi_device dev;
	int rc;
	int cmd_opt = 0;
	uint32_t len = 16;
	uint8_t rd = 0;
	uint8_t wr = 0;
	uint32_t addr = 0x00600000;
	int id = 0;
	char msg[256] = {0};
	char device[20] = {0};
	int msg_len = 0;
	int force = 0;

	dev.filename = "/dev/spidev0.0";
	dev.mode = 0;
	dev.bpw = 8;
	dev.speed = 5000000;
	dev.cs_change = 0;

	while (cmd_opt != -1) {
		cmd_opt = getopt(argc, argv, "ifrwhva:d:s:l:p:m:");

		switch (cmd_opt) {
			case 'a':
				addr = strtol(optarg, NULL, 16);
				break;

			case 'd':
				strncpy(device, optarg, sizeof(device) - 1);
				device[sizeof(device) -1] = '\0';
				dev.filename = device;
				break;

			case 'i':
				id = 1;
				break;

			case 'f':
				force = 1;
				break;

			case 'm':
				msg_len = strlen(optarg);
				strncpy(msg, optarg, sizeof(msg) - 1);
				msg[sizeof(msg) - 1] = '\0';
				break;

			case 's':
				dev.speed = (uint32_t)atoi(optarg);
				break;

			case 'l':
				len = (uint32_t)atoi(optarg);
				break;

			case 'r':
				rd = 1;
				break;

			case 'w':
				wr = 1;
				break;
				
			case 'v':
				verbose = 1;
				break;

			case 'h':
				help();
				return 0;

			case -1:
				break;

			default:
				help();
				return -1;
		}
	}

	if (verbose) {
		printf("device = %s\n", dev.filename);
		printf("speed = %d HZ\n", dev.speed);
		printf("mode = %d\n", dev.mode);
		printf("address = 0x%x\n", addr);
	}

	rc = spi_start(&dev);
	if (rc) {
		printf("Failed to start SPI device\n");
		return rc;
	}

	if (id) {
		/* Read ID of SPI Flash */
		value = cmd_reg(&dev, SPI_ID);
		printf("SPI Flash ID = 0x%X\n", value);
	}

	if (wr) {
		if (addr < 0x00800000) {
			if (force != 1) {
				addr = 0x00800000;
				printf("Warning: Avoid writing to adddress less than 0x%x\n", addr);
				printf("as it will overwrite Linux bootloader\n");
				printf("Use -f if you know what you are doing\n");
			}
		}

		if (msg == NULL) {
			printf("Message is empty. Use -m to write the data\n");
		} else {
			printf("Writing to SPI flash at 0x%x\n", addr);
			spi_flash_write(&dev, addr, (uint8_t *)msg, msg_len);
		}
	}
	
	if (rd) {
		spi_flash_read(&dev, addr, len);
	}

	spi_stop(&dev);

	return 0;
}
