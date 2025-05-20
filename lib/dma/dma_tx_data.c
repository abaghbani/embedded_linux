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

void * data_mm2s_thread(void * data)
{
	struct thread_info * info = data;
	enum data_state{init_state, wait_for_command, wait_for_transfer_done} state = init_state;
	
	uint32_t buff_add[1], buff_size[1];
	uint32_t *dma_reg_p = info->dma_p;
	struct message_info *msg_p = info->data_mm2s_message;
	
	while(1)
	{
		switch(state)
		{
			case init_state:
				// do nothing
				state = wait_for_command;
				break;
			
			case wait_for_command:
				if(msg_p->start_dma && !msg_p->cmd_done)
				{
					printf("data_MM2S: message is recieved\n");
					printf("data_MM2S: received paras: 0x%x 0x%x 0x%x \n", msg_p->buffer_add, msg_p->buffer_size, msg_p->cyclic_mode);
					buff_add[0] = msg_p->buffer_add;
					buff_size[0] = XIL_DMA_DESC_TXSOF+XIL_DMA_DESC_TXEOF+msg_p->buffer_size;	// mm2s dma need to be set sof,eof
					dma_descriptor_init(info->memoryFD, DESC_BUFF_DATA_MM2S_ADD, 1, buff_add, buff_size);
					dma_init(dma_reg_p, DESC_BUFF_DATA_MM2S_ADD, msg_p->cyclic_mode);
					dma_retrig(dma_reg_p, DESC_BUFF_DATA_MM2S_ADD, msg_p->cyclic_mode);
					if(msg_p->cyclic_mode)
					{
						msg_p->start_dma = 0;
						msg_p->cmd_done = 1;
						printf("data_MM2S: dma is started in cyclic_mode.\n");
						state = wait_for_command;
					}
					else
					{
						printf("data_MM2S: dma is started in non cyclic_mode.\n");
						state = wait_for_transfer_done;
					}
				}
				if(msg_p->stop_dma && !msg_p->cmd_done)
				{
					// reset dma to clear all regs
					dma_reset(dma_reg_p);
					printf("data_MM2S: dma is stopped.\n");
					msg_p->stop_dma = 0;
					msg_p->cmd_done = 1;
					state = wait_for_command;
				}
				break;
			
			case wait_for_transfer_done:
				while(!dma_status(dma_reg_p));
				printf("data_MM2S: transmit data is done.\n");
				msg_p->start_dma = 0;
				msg_p->cmd_done = 1;
				state = wait_for_command;
				break;
		}
	}
	
	return 0;
}
