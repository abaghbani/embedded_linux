
struct SPIDevice
{
	char* filename;		// Path of the SPI bus, eg: /dev/spidev1.1
	int fd;				// File descriptor for the SPI bus
	uint32_t mode;
	uint8_t bits;
	uint32_t speed;
};

int spi_open(struct SPIDevice* dev);
ssize_t spi_transfer(struct SPIDevice* dev, void *out, void *in, size_t len);
int spi_close(struct SPIDevice* dev);
