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

void * data_s2mm_thread(void * data)
{
	struct thread_info * info = data;
	enum data_state{init_state, wait_for_command, start_capture, wait_for_transfer_done, wait_for_usb_transfer_done} state = init_state;
	uint8_t m_current_desc;
	
	uint32_t buff_add[8], buff_size[8];
	uint32_t *dma_reg_p = info->dma_p;
	struct message_info *msg_p = info->data_s2mm_message;
	struct message_info *usb_msg_p = info->usb_mm2s_message;
	
	uint32_t *dma_usb_mm2s_p = mmap(0, XIL_DMA_REG_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, info->memoryFD, (off_t)AXI_DMA_USB_ADDRESS);
	uint32_t *fpga_reg_p = mmap(0, AXI_FPGA_REGISTER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, info->memoryFD, (off_t)AXI_FPGA_REGISTER_ADDRESS);
	uint32_t *descriptor_p = mmap(0, DESC_BUFF_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, info->memoryFD, (off_t)DESC_BUFF_DATA_S2MM_ADD);
	uint32_t *usb_descriptor_p = mmap(0, DESC_BUFF_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, info->memoryFD, (off_t)DESC_BUFF_USB_MM2S_ADD);
	
	while(1)
	{
		switch(state)
		{
			case init_state:
				//dma_init(dma_reg_p, DESC_BUFF_data_S2MM_ADD, 0);
				dma_init(dma_usb_mm2s_p, DESC_BUFF_USB_MM2S_ADD, 0);
				state = wait_for_command;
				break;
			
			case wait_for_command:
				if(msg_p->start_dma && !msg_p->cmd_done)
				{
					printf("data_S2MM: message is recieved to start capture\n");
					dma_init(dma_reg_p, DESC_BUFF_DATA_S2MM_ADD, 0);
					msg_p->start_dma = 0;
					msg_p->cmd_done = 1;
					state = start_capture;
				}
				else if(msg_p->stop_dma && !msg_p->cmd_done)
				{
					// reset dma to clear all regs
					fpga_reg_p[AXI_FPGA_REG_CTRL/4] = 0x80;		// reset dma counter in fpga
					dma_reset(dma_reg_p);
					printf("data_S2MM: dma is stopped.\n");
					msg_p->stop_dma = 0;
					msg_p->cmd_done = 1;
					state = wait_for_command;
				}
				else if(usb_msg_p->start_dma && !usb_msg_p->cmd_done)
				{
					printf("USB_MM2S: message is recieved\n");
					buff_add[0] = usb_msg_p->buffer_add;
					buff_size[0] = XIL_DMA_DESC_TXSOF+XIL_DMA_DESC_TXEOF+usb_msg_p->buffer_size;	// mm2s dma need to be set sof,eof
					dma_descriptor_init(info->memoryFD, DESC_BUFF_USB_MM2S_ADD, 1, buff_add, buff_size);
					dma_retrig(dma_usb_mm2s_p, DESC_BUFF_USB_MM2S_ADD, 0);
					state = wait_for_usb_transfer_done;
				}
				break;
			
			case start_capture:
				buff_add[0] = RX_BUFF_SLICE_0_START;
				buff_add[1] = RX_BUFF_SLICE_1_START;
				buff_add[2] = RX_BUFF_SLICE_2_START;
				buff_add[3] = RX_BUFF_SLICE_3_START;
				buff_add[4] = RX_BUFF_SLICE_4_START;
				buff_add[5] = RX_BUFF_SLICE_5_START;
				buff_add[6] = RX_BUFF_SLICE_6_START;
				buff_add[7] = RX_BUFF_SLICE_7_START;
				buff_size[0] = RX_BUFF_SLICE_SIZE-1;
				buff_size[1] = RX_BUFF_SLICE_SIZE-1;
				buff_size[2] = RX_BUFF_SLICE_SIZE-1;
				buff_size[3] = RX_BUFF_SLICE_SIZE-1;
				buff_size[4] = RX_BUFF_SLICE_SIZE-1;
				buff_size[5] = RX_BUFF_SLICE_SIZE-1;
				buff_size[6] = RX_BUFF_SLICE_SIZE-1;
				buff_size[7] = RX_BUFF_SLICE_SIZE-1;
				dma_descriptor_init(info->memoryFD, DESC_BUFF_DATA_S2MM_ADD, 8, buff_add, buff_size);
				fpga_reg_p[AXI_FPGA_REG_DMA_DATA_LEN/4] = RX_BUFF_SLICE_SIZE-16;
				dma_retrig(dma_reg_p, DESC_BUFF_DATA_S2MM_ADD+7*XIL_DMA_DESC_SIZE, 0);
				printf("data_S2MM: capturing...\n");
				m_current_desc = 0;
				usb_descriptor_p[XIL_DMA_DESC_STATUS] = 0x80000000;		// to show this dma is not busy for first transaction
				state = wait_for_transfer_done;
				break;

			case wait_for_transfer_done:
				if((msg_p->start_dma || msg_p->stop_dma) && !msg_p->cmd_done)
				{
					state = wait_for_command;
				}
				else if( (descriptor_p[XIL_DMA_DESC_STATUS+m_current_desc*XIL_DMA_DESC_SIZE/4] & 0x80000000) && (usb_descriptor_p[XIL_DMA_DESC_STATUS] & 0x80000000) )
				{
					buff_add[0] = RX_BUFF_SLICE_0_START+m_current_desc*RX_BUFF_SLICE_SIZE;
					buff_size[0] = XIL_DMA_DESC_TXSOF+XIL_DMA_DESC_TXEOF+(RX_BUFF_SLICE_SIZE-16);	// mm2s dma need to be set sof,eof
					dma_descriptor_init(info->memoryFD, DESC_BUFF_USB_MM2S_ADD, 1, buff_add, buff_size);
					dma_retrig(dma_usb_mm2s_p, DESC_BUFF_USB_MM2S_ADD, 0);
					printf("data_S2MM: desc # %d is completed.\n", m_current_desc);
					if(++m_current_desc >= 8)
						state = wait_for_command;
				}
				break;
			
			case wait_for_usb_transfer_done:
				while(!dma_status(dma_usb_mm2s_p));
				printf("USB_MM2S: message is done.\n");
				usb_msg_p->start_dma = 0;
				usb_msg_p->cmd_done = 1;
				state = wait_for_command;
				break;
		}
	}
	
	return 0;
}

