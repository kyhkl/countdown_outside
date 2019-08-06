#include "board.h"
#include <rtdevice.h>
#include <drv_dma.h>

/* STM32 dma driver */
struct stm32_dma
{
    DMA_HandleTypeDef hdma;
    IRQn_Type irq;
};


int rt_hw_dma_init(void)
{
	 struct stm32_dma *dma;
	
	 HAL_DMA_Start (&dma->hdma, uint32_t SrcAddress, uint32_t DstAddress, uint32_t DataLength);
}