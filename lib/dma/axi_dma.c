#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "axi_dma.h"
#include "board.h"

int dma_descriptor_init(int memFD, uint32_t dma_desc_add, uint8_t n_desc, uint32_t *buff_add, uint32_t *buff_size)
{
	uint32_t *descriptor_p = mmap(0, DESC_BUFF_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memFD, (off_t)dma_desc_add);
	if (descriptor_p == MAP_FAILED) {
		printf("Failed to mmap desc @0x%8X\n", dma_desc_add);
		exit(EXIT_FAILURE);
	}

	for(int i=0; i<n_desc;i++)
	{
		descriptor_p[i*XIL_DMA_DESC_SIZE/4+XIL_DMA_DESC_NXTDESC] = dma_desc_add+XIL_DMA_DESC_SIZE*((i+1)%n_desc);
		descriptor_p[i*XIL_DMA_DESC_SIZE/4+XIL_DMA_DESC_NXTDESC_MSB] = 0;
		descriptor_p[i*XIL_DMA_DESC_SIZE/4+XIL_DMA_DESC_BUFFER_ADD] = buff_add[i];
		descriptor_p[i*XIL_DMA_DESC_SIZE/4+XIL_DMA_DESC_BUFFER_ADD_MSB] = 0;
		descriptor_p[i*XIL_DMA_DESC_SIZE/4+XIL_DMA_DESC_CONTROL] = buff_size[i];
		descriptor_p[i*XIL_DMA_DESC_SIZE/4+XIL_DMA_DESC_STATUS] = 0;
	}
	printf("\n");
	printf("Descriptor update:  @ 0x%8X\n", dma_desc_add);
	munmap((void*)descriptor_p, DESC_BUFF_SIZE);
	return 1;
}

int dma_reset(uint32_t *dma_reg_p)
{
	dma_reg_p[XIL_DMA_REG_CTRL] = 0x04;

	return 1;
}

int dma_init(uint32_t *dma_reg_p, uint32_t desc_add, int cyclic_mode_enable)
{
	dma_reg_p[XIL_DMA_REG_CURDESC] = desc_add;
	dma_reg_p[XIL_DMA_REG_CTRL] = 0x01+(cyclic_mode_enable?0x10:0);

	return 1;
}

int dma_retrig(uint32_t *dma_reg_p, uint32_t desc_add, int cyclic_mode_enable)
{
	dma_reg_p[XIL_DMA_REG_TAILDESC] = desc_add+(cyclic_mode_enable?0xF000:0);	// write a temp address in tail (out of descriptor range) for cyclic mode

	return 1;
}

int dma_status(uint32_t *dma_reg_p)
{
	if(dma_reg_p[XIL_DMA_REG_STATUS]&0xf000)
	{
		dma_reg_p[XIL_DMA_REG_STATUS] = 0xf000; // to clear interrupt
		return 1;
	}
	else
		return 0;
}