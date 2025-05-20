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

void usb_s2mm_thread(struct thread_info * info)
{
	enum usb_rxdata_state{init_state, start_dma_command, wait_for_command, wait_for_sample_data} state = init_state;
	
	uint32_t *dma_reg_p						= info->dma_p;
	struct message_info *usb_mm2s_msg_p		= info->usb_mm2s_message;
	struct message_info *data_mm2s_msg_p	= info->data_mm2s_message;
	struct message_info *data_s2mm_msg_p	= info->data_s2mm_message;
	uint32_t buff_add[1];
	uint32_t buff_size[1];
	
	uint32_t *ctrl_rx_buff = mmap(0, CTRL_BUFF_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, info->memoryFD, (off_t)CTRL_BUFF_RX_ADD);
	uint32_t *ctrl_tx_buff = mmap(0, CTRL_BUFF_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, info->memoryFD, (off_t)CTRL_BUFF_TX_ADD);
	uint32_t *fpga_reg_p = mmap(0, 0x40000000, PROT_READ | PROT_WRITE, MAP_SHARED, info->memoryFD, (off_t)AXI_FPGA_REGISTER_ADDRESS);
	
	while(1)
	{
		switch(state)
		{
			case init_state:
				dma_init(dma_reg_p, DESC_BUFF_USB_S2MM_ADD, 0);
				state = start_dma_command;
				break;
			
			case start_dma_command:
				buff_add[0] = CTRL_BUFF_RX_ADD;
				buff_size[0] = CTRL_BUFF_SIZE;
				dma_descriptor_init(info->memoryFD, DESC_BUFF_USB_S2MM_ADD, 1, buff_add, buff_size);
				fpga_reg_p[AXI_FPGA_REG_DMA_USB_LEN/4] = 32; //CTRL_BUFF_SIZE;
				dma_retrig(dma_reg_p, DESC_BUFF_USB_S2MM_ADD, 0);
				// reset rx buffer 
				ctrl_rx_buff[0] = 0;
				state = wait_for_command;
				break;
			
			case wait_for_command:
				while(!dma_status(dma_reg_p));
				usleep(1000);
				printf("USB_S2MM: command is recieved = 0x%X, params = 0x%X, 0x%X, 0x%X\n", ctrl_rx_buff[0], ctrl_rx_buff[1], ctrl_rx_buff[2], ctrl_rx_buff[3]);
				switch(ctrl_rx_buff[0])
				{
					case write_reg:
						fpga_reg_p[(ctrl_rx_buff[1]-AXI_FPGA_REGISTER_ADDRESS)/4] = ctrl_rx_buff[2];
						state = start_dma_command;
						break;
					
					case read_reg:
						// update txBuf with read regiter info
						ctrl_tx_buff[0] = ctrl_rx_buff[0];
						ctrl_tx_buff[1] = ctrl_rx_buff[1];
						ctrl_tx_buff[2] = fpga_reg_p[(ctrl_rx_buff[1]-AXI_FPGA_REGISTER_ADDRESS)/4];
						// send a message to dma_usb_mm2s
						usb_mm2s_msg_p->buffer_add = CTRL_BUFF_TX_ADD;
						usb_mm2s_msg_p->buffer_size = 8*1024; // minimum transfer to usb is 2Kbytes. minimum to auto flashing data in fx3 is 8Kbytes
						while(!usb_mm2s_msg_p->cmd_done);
						usb_mm2s_msg_p->start_dma = 1;
						usb_mm2s_msg_p->cmd_done = 0;
						state = start_dma_command;
						break;
					
					case write_sample_data:
						if(ctrl_rx_buff[1] != 0) // len should be non-zero
						{
							// configure DB to accept sample data
							buff_add[0] = TX_BUFF_SLICE_3_START;
							buff_size[0] = TX_BUFF_SLICE_SIZE-1;
							dma_descriptor_init(info->memoryFD, DESC_BUFF_USB_S2MM_ADD, 1, buff_add, buff_size);
							//dma_init(dma_reg_p, DESC_BUFF_USB_S2MM_ADD, 0);
							fpga_reg_p[AXI_FPGA_REG_DMA_USB_LEN/4] = ctrl_rx_buff[1];	// len is comping in first param in command
							dma_retrig(dma_reg_p, DESC_BUFF_USB_S2MM_ADD, 0);
							printf("USB_S2MM: start dma to read sample data. len = 0x%x\n", ctrl_rx_buff[1]);
						}
						state = wait_for_sample_data;
						break;
					
					case stop_trans_sample:
						// message to stop dma_data_mm2s
						while(!data_mm2s_msg_p->cmd_done);
						data_mm2s_msg_p->cmd_done = 0;
						data_mm2s_msg_p->stop_dma = 1;
						state = start_dma_command;
						break;
						
					case start_trans_sample:
						if(ctrl_rx_buff[1] != 0) // len should be non-zero
						{
							// message to start dma_data_mm2s with params
							data_mm2s_msg_p->buffer_add = TX_BUFF_SLICE_3_START;
							data_mm2s_msg_p->buffer_size = ctrl_rx_buff[1];
							data_mm2s_msg_p->cyclic_mode = (uint8_t)ctrl_rx_buff[3];
							while(!data_mm2s_msg_p->cmd_done);
							data_mm2s_msg_p->cmd_done = 0;
							data_mm2s_msg_p->start_dma = 1;
						}
						state = start_dma_command;
						break;
					
					case stop_receive_sample:
						// message to stop dma_data_s2mm
						while(!data_s2mm_msg_p->cmd_done);
						data_s2mm_msg_p->cmd_done = 0;
						data_s2mm_msg_p->stop_dma = 1;
						state = start_dma_command;
						break;
						
					case start_receive_sample:
						while(!data_s2mm_msg_p->cmd_done);
						data_s2mm_msg_p->cmd_done = 0;
						data_s2mm_msg_p->start_dma = 1;
						state = start_dma_command;
						break;
						
					default:
						printf("Error: Unknown command  = 0x%x\n", ctrl_rx_buff[0]);
						state = start_dma_command;
						break;
				}
				break;
			
			case wait_for_sample_data:
				while(!dma_status(dma_reg_p));
				printf("USB_S2MM: sample data is recieved\n");
				
				if(ctrl_rx_buff[2] != 0) // start transmit after received sample data
				{
					printf("USB_S2MM: Send a message to start transmitting\n");
					data_mm2s_msg_p->buffer_add = TX_BUFF_SLICE_3_START;
					data_mm2s_msg_p->buffer_size = ctrl_rx_buff[1];
					data_mm2s_msg_p->cyclic_mode = (uint8_t)ctrl_rx_buff[3];
					while(!data_mm2s_msg_p->cmd_done);
					data_mm2s_msg_p->cmd_done = 0;
					data_mm2s_msg_p->start_dma = 1;
				}
				printf("USB_S2MM: Send is done.\n");
				
				state = start_dma_command;
				break;
		}
	}
	
	// return 0;
}
