#include "stdio.h"
#include "fifo.h"
#include "stm32l1xx_hal.h"
#include "stm32l1xx_hal_usart.h"



#define DBG_LOG (printf("%s:%u [%ld]\t", __FILE__, __LINE__, xTaskGetTickCount()), printf)

#if 0
#define LOG_BUFF_SIZE  1024
static uint8_t log_buff[LOG_BUFF_SIZE];
static struct fifo log_fifo;
void log_module_init()
{
	
	int ret = fifo_init(&log_fifo,log_buff,sizeof(log_buff));	
	if(ret != 0){
		DBG_LOG("log fifo init fail %d\n",ret);
	}
}
#endif

extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif
/** 
* @brief  Retargets the C library printf function to the USART. 
* @param  None 
* @retval None 
*/  
PUTCHAR_PROTOTYPE  
{  
	
	//while(__HAL_USART_GET_FLAG(&huart2, USART_FLAG_TC) == RESET);
	//huart2.Instance->DR = ch;
	HAL_UART_Transmit(&huart2, (uint8_t *)&ch,1,0xFF);
	return ch;
} 
