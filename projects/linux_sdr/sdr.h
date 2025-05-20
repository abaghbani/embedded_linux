#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "axi_switch.h"
#include "axi_dma.h"
#include "board.h"

struct message_info {
	uint8_t				start_dma;
	uint8_t				stop_dma;
	uint8_t				cmd_done;
	uint32_t			buffer_add;
	uint32_t			buffer_size;
	uint8_t				cyclic_mode;
};

struct thread_info {
	int							memoryFD;
	uint32_t					*dma_p;
	
	struct message_info			*usb_mm2s_message;
	struct message_info			*data_mm2s_message;
	struct message_info			*data_s2mm_message;
};


#define write_reg				0xf0100030
#define read_reg				0xf0100060
#define write_sample_data		0xf01000C0
#define start_trans_sample		0xf01030C0
#define stop_trans_sample		0xf01040C0
#define start_receive_sample	0xf01050C0
#define stop_receive_sample		0xf01060C0

// void * usb_s2mm_thread(void * data);
void usb_s2mm_thread(struct thread_info * info);
void * usb_mm2s_thread(void * data);
void * data_mm2s_thread(void * data);
void * data_s2mm_thread(void * data);
