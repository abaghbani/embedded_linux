

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <signal.h>
#include <sched.h>
#include <time.h>
#include <errno.h>
#include <sys/param.h>


#define BUFFER_SIZE (128 * 1024)	 	/* must match driver exactly */
#define BUFFER_COUNT 32					/* driver only */

#define TX_BUFFER_COUNT 	1				/* app only, must be <= to the number in the driver */
#define RX_BUFFER_COUNT 	32				/* app only, must be <= to the number in the driver */
#define BUFFER_INCREMENT	1				/* normally 1, but skipping buffers (2) defeats prefetching in the CPU */

#define FINISH_XFER 	_IOW('a','a',int32_t*)
#define START_XFER 		_IOW('a','b',int32_t*)
#define XFER 			_IOR('a','c',int32_t*)


struct channel_buffer {
	unsigned int buffer[BUFFER_SIZE / sizeof(unsigned int)];
	enum proxy_status { PROXY_NO_ERROR = 0, PROXY_BUSY = 1, PROXY_TIMEOUT = 2, PROXY_ERROR = 3 } status;
	unsigned int length;
} __attribute__ ((aligned (1024)));		/* 64 byte alignment required for DMA, but 1024 handy for viewing memory */


#define TX_CHANNEL_COUNT 1
#define RX_CHANNEL_COUNT 1

const char *tx_channel_names[] = { "dma_proxy_tx", /* add unique channel names here */ };
const char *rx_channel_names[] = { "dma_proxy_rx", /* add unique channel names here */ };

/* Internal data which should work without tuning */

struct channel {
	struct channel_buffer *buf_ptr;
	int fd;
	pthread_t tid;
};

static int verify = 1;
static int test_size;
static volatile int stop = 0;
int num_transfers;

struct channel tx_channels[TX_CHANNEL_COUNT], rx_channels[RX_CHANNEL_COUNT];

/*******************************************************************************************************************/
/* Handle a control C or kill, maybe the actual signal number coming in has to be more filtered?
 * The stop should cause a graceful shutdown of all the transfers so that the application can
 * be started again afterwards.
 */
void sigint(int a)
{
	stop = 1;
}

/*******************************************************************************************************************/
/* Get the clock time in usecs to allow performance testing
 */
static uint64_t get_posix_clock_time_usec ()
{
    struct timespec ts;

    if (clock_gettime (CLOCK_MONOTONIC, &ts) == 0)
        return (uint64_t) (ts.tv_sec * 1000000 + ts.tv_nsec / 1000);
    else
        return 0;
}

/*******************************************************************************************************************/
/*
 * The following function is the transmit thread to allow the transmit and the receive channels to be
 * operating simultaneously. Some of the ioctl calls are blocking so that multiple threads are required.
 */
void tx_thread(struct channel *channel_ptr)
{
	int i, counter = 0, buffer_id, in_progress_count = 0;
	int stop_in_progress = 0;

	// Start all buffers being sent
	buffer_id = 0;
	for (i = 0; i < test_size / sizeof(unsigned int); i++)
		channel_ptr->buf_ptr[0].buffer[i] = i;
	channel_ptr->buf_ptr[0].length = test_size;
	ioctl(channel_ptr->fd, START_XFER, &buffer_id);
	channel_ptr->buf_ptr[0].length = test_size;
	ioctl(channel_ptr->fd, START_XFER, &buffer_id);

	ioctl(channel_ptr->fd, FINISH_XFER, &buffer_id);
	ioctl(channel_ptr->fd, FINISH_XFER, &buffer_id);

	/*
	for (buffer_id = 0; buffer_id < TX_BUFFER_COUNT; buffer_id += BUFFER_INCREMENT) {

		channel_ptr->buf_ptr[buffer_id].length = test_size;

		if (verify)
			for (i = 0; i < 1; i++) // test_size / sizeof(unsigned int); i++)
				channel_ptr->buf_ptr[buffer_id].buffer[i] = i + in_progress_count;

		ioctl(channel_ptr->fd, (unsigned long int)START_XFER, &buffer_id);

		if (++in_progress_count >= num_transfers)
			break;
	}

	buffer_id = 0;

	while (1) {

		ioctl(channel_ptr->fd, FINISH_XFER, &buffer_id);
		if (channel_ptr->buf_ptr[buffer_id].status != PROXY_NO_ERROR)
			printf("Proxy tx transfer error\n");

		in_progress_count--;
		counter++;


		if (counter >= num_transfers)
			break;


		if (stop & !stop_in_progress) {
			stop_in_progress = 1;
			num_transfers = counter + RX_BUFFER_COUNT;
		}


		if ((counter + in_progress_count) >= num_transfers)
			goto end_tx_loop0;


		if (verify) {
			unsigned int *buffer = (unsigned int *)&channel_ptr->buf_ptr[buffer_id].buffer;
			for (i = 0; i < test_size / sizeof(unsigned int); i++)
				buffer[i] = i + ((TX_BUFFER_COUNT / BUFFER_INCREMENT) - 1) + counter;
		}


		ioctl(channel_ptr->fd, START_XFER, &buffer_id);

		in_progress_count++;

end_tx_loop0:


		buffer_id += BUFFER_INCREMENT;
		buffer_id %= TX_BUFFER_COUNT;
	}*/
}

void rx_thread(struct channel *channel_ptr)
{
	int in_progress_count = 0, buffer_id = 0;
	int rx_counter = 0;

	// Start all buffers being received

	channel_ptr->buf_ptr[0].length = test_size;
	ioctl(channel_ptr->fd, START_XFER, &buffer_id);
	channel_ptr->buf_ptr[0].length = test_size;
	ioctl(channel_ptr->fd, START_XFER, &buffer_id);

	ioctl(channel_ptr->fd, FINISH_XFER, &buffer_id);
	ioctl(channel_ptr->fd, FINISH_XFER, &buffer_id);


/*
	for (buffer_id = 0; buffer_id < RX_BUFFER_COUNT; buffer_id += BUFFER_INCREMENT) {

		// Don't worry about initializing the receive buffers as the pattern used in the transmit buffers is unique across every transfer so it should catch errors.

		channel_ptr->buf_ptr[buffer_id].length = test_size;

		ioctl(channel_ptr->fd, START_XFER, &buffer_id);

		// Handle the case of a specified number of transfers that is less than the number of buffers
		if (++in_progress_count >= num_transfers)
			break;
	}

	buffer_id = 0;

	// Finish each queued up receive buffer and keep starting the buffer over again until all the transfers are done

	while (1) {

		ioctl(channel_ptr->fd, FINISH_XFER, &buffer_id);

		if (channel_ptr->buf_ptr[buffer_id].status != PROXY_NO_ERROR) {
			printf("Proxy rx transfer error, # transfers %d, # completed %d, # in progress %d\n",
						num_transfers, rx_counter, in_progress_count);
			exit(1);
		}

		// Verify the data received matches what was sent (tx is looped back to tx) A unique value in the buffers is used across all transfers

		if (verify) {
			unsigned int *buffer = &channel_ptr->buf_ptr[buffer_id].buffer;
			int i;
			for (i = 0; i < 1; i++) // test_size / sizeof(unsigned int); i++) this is slow
				if (buffer[i] != i + rx_counter) {
					printf("buffer not equal, index = %d, data = %d expected data = %d\n", i,
						buffer[i], i + rx_counter);
					break;
				}
		}

		// Keep track how many transfers are in progress so that only the specified number of transfers are attempted

		in_progress_count--;

		// If all the transfers are done then exit

		if (++rx_counter >= num_transfers)
			break;

		// If the ones in progress will complete the number of transfers then don't start more but finish the ones that are already started

		if ((rx_counter + in_progress_count) >= num_transfers)
			goto end_rx_loop0;

		// Start the next buffer again with another transfer keeping track of the number in progress but not finished

		ioctl(channel_ptr->fd, START_XFER, &buffer_id);

		in_progress_count++;

	end_rx_loop0:

		// Flip to next buffer treating them as a circular list, and possibly skipping some to show the results when prefetching is not happening

		buffer_id += BUFFER_INCREMENT;
		buffer_id %= RX_BUFFER_COUNT;

	}*/
}

/*******************************************************************************************************************/
/*
 * Setup the transmit and receive threads so that the transmit thread is low priority to help prevent it from
 * overrunning the receive since most testing is done without any backpressure to the transmit channel.
 */
void setup_threads(int *num_transfers)
{
	pthread_attr_t tattr_tx;
	int newprio = 20, i;
	struct sched_param param;

	/* The transmit thread should be lower priority than the receive
	 * Get the default attributes and scheduling param
	 */
	pthread_attr_init (&tattr_tx);
	pthread_attr_getschedparam (&tattr_tx, &param);

	/* Set the transmit priority to the lowest
	 */
	param.sched_priority = newprio;
	pthread_attr_setschedparam (&tattr_tx, &param);

	for (i = 0; i < RX_CHANNEL_COUNT; i++)
		pthread_create(&rx_channels[i].tid, NULL, rx_thread, (void *)&rx_channels[i]);

	for (i = 0; i < TX_CHANNEL_COUNT; i++)
		pthread_create(&tx_channels[i].tid, &tattr_tx, tx_thread, (void *)&tx_channels[i]);
}


//int main(int argc, char *argv[])
int main()
{
    printf("Hello World - Akbar\n");

	int i;
	uint64_t start_time, end_time, time_diff;
	int mb_sec;
	int buffer_id = 0;
	int max_channel_count = MAX(TX_CHANNEL_COUNT, RX_CHANNEL_COUNT);

	printf("DMA proxy test\n");

	signal(SIGINT, sigint);

	/*
	if ((argc != 3) && (argc != 4)) {
		printf("Usage: dma-proxy-test <# of DMA transfers to perform> <# of bytes in each transfer in KB (< 1MB)> <optional verify, 0 or 1>\n");
		exit(EXIT_FAILURE);
	}
*/
	/* Get the number of transfers to perform */

	//num_transfers = 20; //atoi(argv[1]);

	/* Get the size of the test to run, making sure it's not bigger than the size of the buffers and
	 * convert it from KB to bytes
	 */
	//test_size = 128*1024; //atoi(argv[2]);
	/*
	if (test_size > BUFFER_SIZE)
		test_size = BUFFER_SIZE;
	test_size *= 1024;*/

	/* Verify is off by default to get pure performance of the DMA transfers without the CPU accessing all the data
	 * to slow it down.
	 */
	/*
	if (argc == 4)
		verify = atoi(argv[3]);
	printf("Verify = %d\n", verify);
*/
	/* Open the file descriptors for each tx channel and map the kernel driver memory into user space */

	for (i = 0; i < TX_CHANNEL_COUNT; i++) {
		char channel_name[64] = "/dev/";
		strcat(channel_name, tx_channel_names[i]);
		tx_channels[i].fd = open(channel_name, O_RDWR);
		if (tx_channels[i].fd < 1) {
			printf("Unable to open DMA proxy device file: %s\r", channel_name);
			exit(EXIT_FAILURE);
		}
		tx_channels[i].buf_ptr = (struct channel_buffer *)mmap(NULL, sizeof(struct channel_buffer) * TX_BUFFER_COUNT,
										PROT_READ | PROT_WRITE, MAP_SHARED, tx_channels[i].fd, 0);
		if (tx_channels[i].buf_ptr == MAP_FAILED) {
			printf("Failed to mmap tx channel\n");
			exit(EXIT_FAILURE);
		}
	}

	/* Open the file descriptors for each rx channel and map the kernel driver memory into user space */

	for (i = 0; i < RX_CHANNEL_COUNT; i++) {
		char channel_name[64] = "/dev/";
		strcat(channel_name, rx_channel_names[i]);
		rx_channels[i].fd = open(channel_name, O_RDWR);
		if (rx_channels[i].fd < 1) {
			printf("Unable to open DMA proxy device file: %s\r", channel_name);
			exit(EXIT_FAILURE);
		}
		rx_channels[i].buf_ptr = (struct channel_buffer *)mmap(NULL, sizeof(struct channel_buffer) * RX_BUFFER_COUNT,
										PROT_READ | PROT_WRITE, MAP_SHARED, rx_channels[i].fd, 0);
		if (rx_channels[i].buf_ptr == MAP_FAILED) {
			printf("Failed to mmap rx channel\n");
			exit(EXIT_FAILURE);
		}
	}

	/*
    FILE * write_ptr;
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char output_filename[64];
    sprintf(output_filename, "sdr_output_%d%02d%02d_%02d%02d%02d.bin", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    write_ptr = fopen(output_filename,"wb");
*/
	num_transfers = 2;
	test_size = 128*1024;


    int memoryFD = open("/dev/mem", O_RDWR);
    uint32_t *u32P = mmap(0, 0x20, PROT_READ | PROT_WRITE, MAP_SHARED, memoryFD, (off_t)0x40000000);
    u32P[0x04/4] = test_size;
    printf("reading from axi port: %x\n", u32P[0x04/4]);

    /*
    u32P[0x04/4] = test_size;
    buffer_id = 0;
    rx_channels[0].buf_ptr[buffer_id].length = test_size;
    ioctl(rx_channels[0].fd, START_XFER, &buffer_id);
    //buffer_id = 1;
    //ioctl(rx_channels[0].fd, START_XFER, &buffer_id);

    ioctl(rx_channels[0].fd, FINISH_XFER, &buffer_id);

   	//ioctl(rx_channels[0].fd, FINISH_XFER, &buffer_id);

	if (rx_channels[0].buf_ptr[buffer_id].status != PROXY_NO_ERROR) {
		printf("Proxy rx transfer error, # transfers %d, # completed %d, # in progress %d\n", num_transfers, 0, 0);
		exit(1);
	}

	fwrite(rx_channels[0].buf_ptr[0].buffer, test_size, 1, write_ptr);
	fwrite(rx_channels[0].buf_ptr[1].buffer, test_size, 1, write_ptr);

	munmap(tx_channels[0].buf_ptr, sizeof(struct channel_buffer));
	munmap(rx_channels[0].buf_ptr, sizeof(struct channel_buffer));
	close(tx_channels[0].fd);
	close(rx_channels[0].fd);
	fclose(write_ptr);
    munmap((void*)u32P, 0x20);
    close(memoryFD);

	return 0;
	*/

	/* Grab the start time to calculate performance then start the threads & transfers on all channels */

	start_time = get_posix_clock_time_usec();
	setup_threads(&num_transfers);

	// Do the minimum to know the transfers are done before getting the time for performance

	for (i = 0; i < RX_CHANNEL_COUNT; i++)
		pthread_join(rx_channels[i].tid, NULL);

	// Grab the end time and calculate the performance

	end_time = get_posix_clock_time_usec();
	time_diff = end_time - start_time;
	mb_sec = ((1000000 / (double)time_diff) * (num_transfers * max_channel_count * (double)test_size)) / 1000000;

	printf("Time: %d microseconds\n", time_diff);
	printf("Transfer size: %d KB\n", (long long)(num_transfers) * (test_size / 1024) * max_channel_count);
	printf("Throughput: %d MB / sec \n", mb_sec);

	// Clean up all the channels before leaving

	for (i = 0; i < TX_CHANNEL_COUNT; i++) {
		pthread_join(tx_channels[i].tid, NULL);
		munmap(tx_channels[i].buf_ptr, sizeof(struct channel_buffer));
		close(tx_channels[i].fd);
	}
	for (i = 0; i < RX_CHANNEL_COUNT; i++) {
		munmap(rx_channels[i].buf_ptr, sizeof(struct channel_buffer));
		close(rx_channels[i].fd);
	}

	printf("DMA proxy test complete\n");

    return 0;
}
