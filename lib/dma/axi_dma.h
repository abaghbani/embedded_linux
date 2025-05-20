
#define XIL_DMA_DESC_NXTDESC			0x00/4
#define XIL_DMA_DESC_NXTDESC_MSB		0x04/4
#define XIL_DMA_DESC_BUFFER_ADD			0x08/4
#define XIL_DMA_DESC_BUFFER_ADD_MSB		0x0C/4
#define XIL_DMA_DESC_CONTROL			0x18/4
#define XIL_DMA_DESC_STATUS				0x1C/4
#define XIL_DMA_DESC_SIZE				0x40
#define XIL_DMA_DESC_TXSOF				0x08000000
#define XIL_DMA_DESC_TXEOF				0x04000000

#define XIL_DMA_REG_CTRL				0x00/4
#define XIL_DMA_REG_STATUS				0x04/4
#define XIL_DMA_REG_CURDESC				0x08/4
#define XIL_DMA_REG_CURDESC_MSB			0x0C/4
#define XIL_DMA_REG_TAILDESC			0x10/4
#define XIL_DMA_REG_TAILDESC_MSB		0x14/4
#define XIL_DMA_REG_SIZE				0x30

int dma_descriptor_init(int memFD, uint32_t dma_desc_add, uint8_t n_desc, uint32_t *buff_add, uint32_t *buff_size);
int dma_reset(uint32_t *dma_reg_p);
int dma_init(uint32_t *dma_reg_p, uint32_t desc_add, int cyclic_mode_enable);
int dma_retrig(uint32_t *dma_reg_p, uint32_t desc_add, int cyclic_mode_enable);
int dma_status(uint32_t *dma_reg_p);

