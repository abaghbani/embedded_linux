#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <pthread.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <sched.h>
#include <time.h>
#include <errno.h>
#include <sys/param.h>

#include "sdr.h"
#include "peripheral.h"

int main(int argc, char *argv[])
{
	printf("SDR embedded is running -- input arg# = %d\n", argc);
	
	printf("peripheral initialization...\n");
	static struct peripheral_info m_peripheral;
	peripheral_init(&m_peripheral);
	
	m_peripheral.led_activity = blue;
	m_peripheral.led_operating = green;
	m_peripheral.attenuator = att_value_0db;
	m_peripheral.led_update = true;
	m_peripheral.attenuator_update = true;
	peripheral(&m_peripheral);
	
	printf("DMA process is started...\n");
	int memoryFD = open("/dev/mem", O_RDWR);
	uint32_t *fpga_reg_p = mmap(0, AXI_FPGA_REGISTER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memoryFD, (off_t)AXI_FPGA_REGISTER_ADDRESS);
	uint32_t *dma_data_mm2s_p = mmap(0, XIL_DMA_REG_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memoryFD, (off_t)AXI_DMA_DATA_ADDRESS);
	uint32_t *dma_data_s2mm_p = dma_data_mm2s_p+XIL_DMA_REG_SIZE/4;
	uint32_t *dma_usb_mm2s_p = mmap(0, XIL_DMA_REG_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memoryFD, (off_t)AXI_DMA_USB_ADDRESS);
	uint32_t *dma_usb_s2mm_p = dma_usb_mm2s_p+XIL_DMA_REG_SIZE/4;
	uint32_t *switch_p = mmap(0, AXI_SWITCH_REGISTER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memoryFD, (off_t)AXI_SWITCH_ADDRESS);
	
	// reset procedure
	// reset all fpga regs
	fpga_reg_p[AXI_FPGA_REG_DMA_DATA_LEN/4] = 0;
	fpga_reg_p[AXI_FPGA_REG_DMA_USB_LEN/4] = 0;
	fpga_reg_p[AXI_FPGA_REG_CTRL/4] = 0xc0+0x2a;
	fpga_reg_p[AXI_FPGA_REG_CAPTURE_CONFIG/4] = 0x8+0x2;	// select both rx and tx with dual channel (channel 0 and 1)
	
	// reset all dma
	dma_reset(dma_data_mm2s_p);
	dma_reset(dma_data_s2mm_p);
	dma_reset(dma_usb_mm2s_p);
	dma_reset(dma_usb_s2mm_p);
	
	// config switch
	switch_config(switch_p);
	
	// reset fpga fifos (again)
	fpga_reg_p[AXI_FPGA_REG_CTRL/4] = 0x40;
	
	printf("Start dma.\n");
	pthread_t data_mm2s_tid;
	pthread_t data_s2mm_tid;
	
	struct message_info usb_mm2s_message = {.start_dma = 0, .stop_dma = 0, .cmd_done = 1};
	struct message_info data_mm2s_message = {.start_dma = 0, .stop_dma = 0, .cmd_done = 1};
	struct message_info data_s2mm_message = {.start_dma = 0, .stop_dma = 0, .cmd_done = 1};
	
	struct thread_info usb_s2mm_info = {.memoryFD = memoryFD, .dma_p = dma_usb_s2mm_p, .usb_mm2s_message = &usb_mm2s_message, .data_mm2s_message = &data_mm2s_message, .data_s2mm_message = &data_s2mm_message};
	struct thread_info data_mm2s_info = {.memoryFD = memoryFD, .dma_p = dma_data_mm2s_p, .usb_mm2s_message = &usb_mm2s_message, .data_mm2s_message = &data_mm2s_message, .data_s2mm_message = &data_s2mm_message};
	struct thread_info data_s2mm_info = {.memoryFD = memoryFD, .dma_p = dma_data_s2mm_p, .usb_mm2s_message = &usb_mm2s_message, .data_mm2s_message = &data_mm2s_message, .data_s2mm_message = &data_s2mm_message};
	
	pthread_create(&data_mm2s_tid, NULL, data_mm2s_thread, &data_mm2s_info);
	printf("creat thread to control dma_data_mm2s...\n");
	pthread_create(&data_s2mm_tid, NULL, data_s2mm_thread, &data_s2mm_info);
	printf("creat thread to control dma_data_s2mm...\n");
	usb_s2mm_thread(&usb_s2mm_info);
	
	// in cae of exit
	printf("cancel all thread and exit...\n");
	pthread_cancel(data_mm2s_tid);
	pthread_cancel(data_s2mm_tid);
	printf("cancelation of all threads is done.\n");
	
	munmap((void*)fpga_reg_p, AXI_FPGA_REGISTER_SIZE);
	munmap((void*)dma_data_mm2s_p, XIL_DMA_REG_SIZE);
	munmap((void*)dma_usb_mm2s_p, XIL_DMA_REG_SIZE);
	munmap((void*)switch_p, AXI_SWITCH_REGISTER_SIZE);
	close(memoryFD);
	peripheral_stop(&m_peripheral);
	
	return 1;
}
