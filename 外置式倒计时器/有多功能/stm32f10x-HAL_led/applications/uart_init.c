#include <rtthread.h>
#include <rtdevice.h>
#include "board.h"
#include "stm32f1xx_hal_uart.h"
#include "config.h"

struct stm32_usart
{
    UART_HandleTypeDef huart;
    IRQn_Type irq;
};
struct stm32_usart usart2 =
{
    {USART2},
    USART2_IRQn
};

void rt_hw_uart_init(void)
	{
    //GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
		
     //USART1_TX   PA.2
    GPIO_InitStructure.Pin = GPIO_PIN_2;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
   
    //USART1_RX	  PA.3
    GPIO_InitStructure.Pin = GPIO_PIN_3;
    GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);  
  
   //USART 初始化设置
		usart2.huart.Init.BaudRate = BAUD_RATE;//一般设置为9600;
		usart2.huart.Init.WordLength = UART_WORDLENGTH_8B;
		usart2.huart.Init.StopBits = UART_STOPBITS_1;
		usart2.huart.Init.Parity = UART_PARITY_NONE;
		usart2.huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
		usart2.huart.Init.Mode = UART_MODE_TX_RX;	 
		HAL_UART_Init(&usart2.huart);
		__HAL_UART_ENABLE_IT(&usart2.huart, USART_IT_RXNE);
//		__HAL_UART_ENABLE_IT(&usart2.huart, USART_IT_IDLE);
		HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
		//HAL_NVIC_EnableIRQ(USART1_IRQn);
		HAL_UART_Init(&usart2.huart);                    //使能串口 
}


#if defined (RT_SAMPLES_AUTORUN) && defined(RT_USING_COMPONENTS_INIT)
    INIT_APP_EXPORT(rt_hw_uart_init);
#endif
MSH_CMD_EXPORT(rt_hw_uart_init, uart sample);
