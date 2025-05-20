/*
 * dma_proxy_test.c
 *
 *  Created on: Sep 30, 2021
 *      Author: Akbar
 */



#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>		// open function
#include <unistd.h>		// close function
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sched.h>
#include <errno.h>
#include <sys/param.h>
#include <pthread.h>


#define TEST_SIZE (3 * 1024 * 1024)


struct dma_proxy_channel_interface {
	unsigned char buffer[TEST_SIZE];
	enum proxy_status { PROXY_NO_ERROR = 0, PROXY_BUSY = 1, PROXY_TIMEOUT = 2, PROXY_ERROR = 3 } status;
	unsigned int length;
};

struct tx_thread_info {
	struct dma_proxy_channel_interface *tx_proxy_interface_p;
	int tx_proxy_fd;
    int dma_count;
    int dma_size;
};

void * tx_thread(void * data)
{
	int dummy, i, counter;
	struct tx_thread_info * info = data;

	info->tx_proxy_interface_p->length = info->dma_size;

	for (counter = 0; counter < info->dma_count; counter++)
	{
		for (i = 0; i < info->dma_size; i++)
			info->tx_proxy_interface_p->buffer[i] = counter + i;

		// Perform the DMA transfer and the check the status after it completes as the call blocks til the transfer is done.
		ioctl(info->tx_proxy_fd, 0, &dummy);
		if (info->tx_proxy_interface_p->status != PROXY_NO_ERROR)
			printf("Proxy tx transfer error\n");
	}

	return 0;
}

int dma_proxy_test()
{
    printf("Hello World - Akbar\n");

	int dummy;
	pthread_t tid;

	int tx_proxy_fd = open("/dev/dma_proxy_tx", O_RDWR | O_SYNC);
	if (tx_proxy_fd < 1) {
		printf("Unable to open DMA proxy device file");
		return -1;
	}

	int rx_proxy_fd = open("/dev/dma_proxy_rx", O_RDWR | O_SYNC);
	if (tx_proxy_fd < 1) {
		printf("Unable to open DMA proxy device file");
		return -1;
	}

	struct dma_proxy_channel_interface * tx_proxy_interface_p = (struct dma_proxy_channel_interface *)mmap(NULL, sizeof(struct dma_proxy_channel_interface), PROT_READ | PROT_WRITE, MAP_SHARED, tx_proxy_fd, 0);
	struct dma_proxy_channel_interface * rx_proxy_interface_p = (struct dma_proxy_channel_interface *)mmap(NULL, sizeof(struct dma_proxy_channel_interface), PROT_READ | PROT_WRITE, MAP_SHARED, rx_proxy_fd, 0);
	if ((rx_proxy_interface_p == MAP_FAILED) || (tx_proxy_interface_p == MAP_FAILED)) {
		printf("Failed to mmap\n");
		return -1;
	}

	int test_size = 100*1024;
	struct tx_thread_info info = {.dma_count = 10, .dma_size = test_size, .tx_proxy_fd = tx_proxy_fd, .tx_proxy_interface_p = tx_proxy_interface_p};

	// Create the thread for the transmit processing passing the number of transactions to it
	pthread_create(&tid, NULL, tx_thread, (void *)&info);

	for (int counter = 0; counter < info.dma_count; counter++)
	{
		rx_proxy_interface_p->length = test_size;
		ioctl(rx_proxy_fd, 0, &dummy);

		if(rx_proxy_interface_p->status != PROXY_NO_ERROR)
			printf("Proxy rx transfer error\n");

		// Verify the data recieved matchs what was sent (tx is looped back to tx)
		for (int i = 0; i < test_size; i++)
			if (rx_proxy_interface_p->buffer[i] != (unsigned char)(counter + i))
				printf("buffer not equal, index = %d, data = %d expected data = %d\n", i, rx_proxy_interface_p->buffer[i], (unsigned char)(counter + i));

		printf("=====>>>> run %d is passed.\n", counter);
	}

	pthread_exit(NULL);
	munmap(tx_proxy_interface_p, sizeof(struct dma_proxy_channel_interface));
	munmap(rx_proxy_interface_p, sizeof(struct dma_proxy_channel_interface));
	close(tx_proxy_fd);
	close(rx_proxy_fd);

	printf("DMA proxy test complete\n");

    return 0;
}
