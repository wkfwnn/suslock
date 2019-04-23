/*
	This file is about uart function,it will start a task about uart read
	only support receive  max bytes  defined in file:MAX_UART_QUEUE_SIZE,if
	exceeed MAX_UART_QUEUE_SIZE,we only receive MAX_UART_QUEUE_SIZE,mean bytes 
	exceed MAX_UART_QUEUE_SIZE will lost some ,so if want receive exceed MAX_UART_QUEUE_SIZE,
	please define more size than MAX_UART_QUEUE_SIZE
*/

#include "uart-core.h"
#include "user_define.h"
#include "stdio.h"
#include "stm32l1xx_hal.h"
#include "cmsis_os.h"
#include "string.h"
#include "fifo.h"

/*this define in main.c*/
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;

static EventGroupHandle_t uart_core_write_event_group;

#define UART_RRAME_TIME_GAP_MS 10
#define MAX_READ_CALL_BACK 3

typedef enum {
	UART_RX_OEPN,
	UART_RX_CLOSE,
} uart_core_rx_t;

#define UART_RX_RECEIVE_FRAME_NOT_DEAL 1
#define UART_RX_RECEIVE_NO_FRAME 0

#define UART_NAME_MAX_LEN 6
typedef struct
{
	char uart_name[UART_NAME_MAX_LEN];
	uart_handle_t uart_handle;
	UART_HandleTypeDef *uart;
	struct fifo *pfifo;
	uint8_t uart_rx_tmp_data;
	uint8_t uart_rx_flag;
	/*call back function is in isr routing,pls as short as possible,fist copy data,and relese sem or other signal*/
	uart_read_call_back uart_read_call_back_array[MAX_READ_CALL_BACK];
	uint8_t uart_call_back_count;
	TimerHandle_t xTimers;
	TimerCallbackFunction_t xtimerCallbackFunction;
	SemaphoreHandle_t xSemaphore;
	uint16_t write_event_bits;

} uart_map_t;

static void uart1TimerCallback(TimerHandle_t xTimer);
static void uart2TimerCallback(TimerHandle_t xTimer);
static void uart3TimerCallback(TimerHandle_t xTimer);

#define UART1_WRITE_CORE_BIT_3 (1 << 0)
#define UART2_WRITE_CORE_BIT_4 (1 << 1)
#define UART3_WRITE_CORE_BIT_5 (1 << 2)
#define MAX_UART_QUEUE_SIZE 512

uint8_t uart1_queue[MAX_UART_QUEUE_SIZE] = {
	0x0,
};
uint8_t uart2_queue[MAX_UART_QUEUE_SIZE] = {
	0x0,
};
uint8_t uart3_queue[MAX_UART_QUEUE_SIZE] = {
	0x0,
};
struct fifo uart1_fifo;
struct fifo uart2_fifo;
struct fifo uart3_fifo;
uart_map_t uart_map_array[] = {
	{"uart1", 1, &huart1, &uart1_fifo, 0, UART_RX_RECEIVE_NO_FRAME, {NULL, NULL, NULL}, 0, NULL, uart1TimerCallback, NULL, UART1_WRITE_CORE_BIT_3},
	{"uart2", 2, &huart2, &uart2_fifo, 0, UART_RX_RECEIVE_NO_FRAME, {NULL, NULL, NULL}, 0, NULL, uart2TimerCallback, NULL, UART2_WRITE_CORE_BIT_4},
	{"uart3", 3, &huart3, &uart3_fifo, 0, UART_RX_RECEIVE_NO_FRAME, {NULL, NULL, NULL}, 0, NULL, uart3TimerCallback, NULL, UART3_WRITE_CORE_BIT_5}
	//add other uart here
};

/*
@name:can be uart1,uart2,uart3,uart4...
@handle: return a handle,common uart_name return common handle 
*/
user_error_t uart_get_handle(const char *uart_name, uint8_t uart_name_len, uart_handle_t *handle)
{
	uint8_t i;
	if (uart_name_len > UART_NAME_MAX_LEN)
	{
		return RET_PARA_ERR;
	}

	for (i = 0; i < sizeof(uart_map_array) / sizeof(uart_map_t); i++)
	{
		if (strncmp(uart_map_array[i].uart_name, uart_name, uart_name_len) == 0)
		{
			*handle = uart_map_array[i].uart_handle;
			return RET_OK;
		}
	}
	return RET_PARA_NOT_COMPATIBLE;
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	DBG_LOG_ISR("HAL_UART_ErrorCallback\n");
	while (1)
		;
}
static void uart1TimerCallback(TimerHandle_t xTimer)
{
	uint8_t j;
	uart_map_array[0].uart_rx_flag = UART_RX_RECEIVE_FRAME_NOT_DEAL;
	for (j = 0; j < uart_map_array[0].uart_call_back_count; j++)
	{
		if (uart_map_array[0].uart_read_call_back_array[j] != NULL)
		{
			uart_map_array[0].uart_read_call_back_array[j](uart_map_array[0].pfifo);
		}
	}
	int fifo_len = fifo_used(uart_map_array[0].pfifo);
	if (fifo_len > 0)
	{
		int len = fifo_out(uart_map_array[0].pfifo, NULL, fifo_len);
		if (len != fifo_len)
		{
			DBG_LOG("read out uart 1 fifo fail\n");
		}
	}
	uart_map_array[0].uart_rx_flag = UART_RX_RECEIVE_NO_FRAME;
}

static void uart2TimerCallback(TimerHandle_t xTimer)
{
	uint8_t j;
	uart_map_array[1].uart_rx_flag = UART_RX_RECEIVE_FRAME_NOT_DEAL;
	for (j = 0; j < uart_map_array[1].uart_call_back_count; j++)
	{
		if (uart_map_array[1].uart_read_call_back_array[j] != NULL)
		{
			uart_map_array[1].uart_read_call_back_array[j](uart_map_array[1].pfifo);
		}
	}
	int fifo_len = fifo_used(uart_map_array[1].pfifo);
	if (fifo_len > 0)
	{
		int len = fifo_out(uart_map_array[1].pfifo, NULL, fifo_len);
		if (len != fifo_len)
		{
			DBG_LOG("read out uart 2 fifo fail\n");
		}
	}
	uart_map_array[1].uart_rx_flag = UART_RX_RECEIVE_NO_FRAME;
}
static void uart3TimerCallback(TimerHandle_t xTimer)
{

	uint8_t j;
	uart_map_array[2].uart_rx_flag = UART_RX_RECEIVE_FRAME_NOT_DEAL;
	for (j = 0; j < uart_map_array[2].uart_call_back_count; j++)
	{
		if (uart_map_array[2].uart_read_call_back_array[j] != NULL)
		{
			uart_map_array[2].uart_read_call_back_array[j](uart_map_array[2].pfifo);
		}
	}
	int fifo_len = fifo_used(uart_map_array[2].pfifo);
	if (fifo_len > 0)
	{
		int len = fifo_out(uart_map_array[2].pfifo, NULL, fifo_len);
		if (len != fifo_len)
		{
			DBG_LOG("read out uart 3 fifo fail\n");
		}
	}
	uart_map_array[2].uart_rx_flag = UART_RX_RECEIVE_NO_FRAME;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

	/* Prevent unused argument(s) compilation warning */
	HAL_StatusTypeDef status;
	int i;
	BaseType_t pxHigherPriorityTaskWoken;
	for (i = 0; i < sizeof(uart_map_array) / sizeof(uart_map_t); i++)
	{
		if (huart == uart_map_array[i].uart)
		{
			//读取下一个数据
			status = HAL_UART_Receive_IT(huart, &uart_map_array[i].uart_rx_tmp_data, 1);
			if (status != HAL_OK && status != HAL_BUSY)
			{
				DBG_LOG_ISR("HAL_UART_Receive_IT fail  or busy %d\n", status);
			}
#if 1
			//如果没有接收到一帧数据或者是数据没有被处理，不在存放新的数据到fifo
			if (uart_map_array[i].uart_rx_flag == UART_RX_RECEIVE_NO_FRAME &&
				fifo_used(uart_map_array[i].pfifo) <= MAX_UART_QUEUE_SIZE)
			{
				int len = fifo_in(uart_map_array[i].pfifo, huart->pRxBuffPtr, huart->RxXferSize);
				xTimerResetFromISR(uart_map_array[i].xTimers, &pxHigherPriorityTaskWoken);
				xTimerStartFromISR(uart_map_array[i].xTimers, &pxHigherPriorityTaskWoken);
			}
#else
			int len = fifo_in(uart_map_array[i].pfifo, huart->pRxBuffPtr, huart->RxXferSize);
#endif
		}
	}
}

/*
@pdata:data buff pointer
@data_size:need to write data size
@flag:
*/
HAL_StatusTypeDef uart_read_one_frame_data(UART_HandleTypeDef *huart, uint8_t *pdata, uint16_t data_size)
{
	HAL_StatusTypeDef status;
	uint8_t count = 0;
reveive_it:
	status = HAL_UART_Receive_IT(huart, pdata, data_size);
	if (status != HAL_OK && status != HAL_BUSY)
	{
		DBG_LOG("HAL_UART_Receive_IT fail  or busy %d\n", status);
	}
	else if (status == HAL_BUSY)
	{
		vTaskDelay(1);
		if (count++ > 100)
		{
			return status;
		}
		goto reveive_it;
	}
	return status;
}

user_error_t uart_core_read_register(uart_handle_t handle, uart_read_call_back call_back_func)
{
	uint8_t i;
	user_error_t ret;
	for (i = 0; i < sizeof(uart_map_array) / sizeof(uart_map_t); i++)
	{
		if (uart_map_array[i].uart_handle == handle)
		{
			if (uart_map_array[i].uart_call_back_count++ < MAX_READ_CALL_BACK)
			{
				DBG_LOG("i = %d,uart_call_back_count = %d\n", i, uart_map_array[i].uart_call_back_count);
				uart_map_array[i].uart_read_call_back_array[uart_map_array[i].uart_call_back_count - 1] = call_back_func;
				DBG_LOG("uart -core register %s at %d\n", uart_map_array[i].uart_name, uart_map_array[i].uart_call_back_count);
				if (uart_read_one_frame_data(uart_map_array[i].uart, &uart_map_array[i].uart_rx_tmp_data, sizeof(uart_map_array[i].uart_rx_tmp_data)) != HAL_OK)
					ret = RET_ERROR;
				else
					ret = RET_OK;
			}
			else
			{
				ret = RET_OVER_FLOW;
				goto return_status;
			}
		}
	}

return_status:
	return ret;
}

void uart_core_read_task_function(void const *argument)
{
#if 0
	UNUSED(argument);
	while(1){
		EventBits_t uxBits;
		const TickType_t xTicksToWait = pdMS_TO_TICKS( portMAX_DELAY);
		uxBits = xEventGroupWaitBits( uart_core_read_event_group,UART_CORE_READ_BIT_ALL,
							          pdTRUE,pdFALSE,xTicksToWait);
		//DBG_LOG("xEventGroupWaitBits wait %d\n",uxBits);
		//	
		//uart_read_one_frame_data(&huart2,uart_map_array[1].huart_queue_p,MAX_UART_QUEUE_SIZE);
		//uart_read_one_frame_data(&huart3,uart_map_array[2].huart_queue_p,MAX_UART_QUEUE_SIZE);
		if( ( uxBits & UART1_READ_CORE_BIT_0 ) != 0 ){
			//DBG_LOG("uart1 start readhaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n");
			uart_read_one_frame_data(&huart1,uart_map_array[0].huart_queue_p,1);
			/* xEventGroupWaitBits() returned because just BIT_0 was set. */
		}else if( ( uxBits & UART2_READ_CORE_BIT_1 ) != 0 ){
			uart_read_one_frame_data(&huart2,uart_map_array[1].huart_queue_p,1);
			//DBG_LOG("uart2 start readhaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n");
		/* xEventGroupWaitBits() returned because just BIT_4 was set. */
		}else if( ( uxBits & UART3_READ_CORE_BIT_2 ) != 0 ){
			uart_read_one_frame_data(&huart3,uart_map_array[2].huart_queue_p,1);
			//DBG_LOG("uart3 start read\n");
			/* xEventGroupWaitBits() returned because just BIT_6 was set. */
		}else{
		/* xEventGroupWaitBits() returned because xTicksToWait ticks passed
		without either BIT_0 or BIT_4 becoming set. */
		}
	}
#endif
	uint8_t buff[10];
	int i = 0;
	int len = 0;
	while (1)
	{
		vTaskDelay(10000);
		do
		{
			len = fifo_out(uart_map_array[0].pfifo, buff, sizeof(buff));
			for (i = 0; i < len; i++)
			{
				printf("%c", buff[i]);
			}
		} while (len != 0);
	}
}

/********************************************************************************
	This is uart-core write funciton,including tx call back,tx funcition
********************************************************************************/

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE, xResult;

	uint8_t i;
	for (i = 0; i < sizeof(uart_map_array) / sizeof(uart_map_t); i++)
	{
		if (uart_map_array[i].uart == huart)
		{
			xResult = xEventGroupSetBitsFromISR(uart_core_write_event_group, uart_map_array[i].write_event_bits,
												&xHigherPriorityTaskWoken);
			/* Was the message posted successfully */
			if (xResult != pdFAIL)
			{
				/* If xHigherPriorityTaskWoken is now set to pdTRUE then a context
				switch should be requested. The macro used is port specific and will
				be either portYIELD_FROM_ISR() or portEND_SWITCHING_ISR() - refer to
				the documentation page for the port being used. */
				portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
			}
		}
	}
}

/*
@pdata:data buff pointer
@data_size:need to write data size
@actual_size:return actual_size transmit
@notice:in this funciton is block transfer
*/
user_error_t uart_write_data(uart_handle_t handle, uint8_t *pdata, uint16_t data_size)
{
	user_error_t ret = RET_OK;

	HAL_StatusTypeDef hal_status;
	UART_HandleTypeDef *huart = NULL;
	uint8_t count = 0;
	EventBits_t uxBits;
	uint8_t i = 0;
	//jutice para of data size and data pointer
	if (data_size == 0 || pdata == NULL)
	{
		ret = RET_PARA_ERR;
		goto return_status;
	}
	for (i = 0; i < sizeof(uart_map_array) / sizeof(uart_map_t); i++)
	{
		if (uart_map_array[i].uart_handle == handle)
		{
			if (uart_map_array[i].xSemaphore != NULL)
			{
				huart = uart_map_array[i].uart;
				xSemaphoreTake(uart_map_array[i].xSemaphore, portMAX_DELAY);
				goto dma_transmit;
			}
		}
	}
	if (huart == NULL)
	{
		ret = RET_DO_NOT_SUPPORT_CURRENT;
		goto return_status;
	}

dma_transmit:
	hal_status = HAL_UART_Transmit_DMA(huart, pdata, data_size);
	if (hal_status == HAL_OK)
	{
		uxBits = xEventGroupWaitBits(uart_core_write_event_group, uart_map_array[i].write_event_bits,
									 pdTRUE, pdFALSE, portMAX_DELAY);
		if ((uxBits & uart_map_array[i].write_event_bits) != 0)
		{
			if ((uart_map_array[i].uart->ErrorCode & HAL_UART_ERROR_DMA) != 0)
			{
				uart_map_array[i].uart->ErrorCode &= (~HAL_UART_ERROR_DMA);
				ret = RET_ERROR;
			}
			else
			{
				ret = RET_OK;
			}
			xSemaphoreGive(uart_map_array[i].xSemaphore);
			goto return_status;
		}
	}
	else if (hal_status == HAL_BUSY)
	{
		vTaskDelay(1);
		if (count++ > 100)
		{
			DBG_LOG("uart core %s write data fail\n", uart_map_array[i].uart_name);
			xSemaphoreGive(uart_map_array[i].xSemaphore);
			ret = RET_TIME_OUT;
			goto return_status;
		}
		goto dma_transmit;
	}

return_status:
	return ret;
}

void uart_core_module_start()
{
	uint8_t i;
	/* Attempt to create the event group. */
	uart_core_write_event_group = xEventGroupCreate();
	/* Was the event group created successfully */
	if (uart_core_write_event_group == NULL)
	{
		DBG_LOG("uart core write group create fail\n");
	}

	for (i = 0; i < sizeof(uart_map_array) / sizeof(uart_map_t); i++)
	{
		if (strncmp(uart_map_array[i].uart_name, UART_DEBUG_PORT, strlen(uart_map_array[i].uart_name)) != 0)
		{
			vSemaphoreCreateBinary(uart_map_array[i].xSemaphore);
			if (uart_map_array[i].xSemaphore == NULL)
			{
				DBG_LOG("%s seam create fail\n", uart_map_array[i].uart_name);
			}
		}
	}
	for (i = 0; i < sizeof(uart_map_array) / sizeof(uart_map_t); i++)
	{

		uart_map_array[i].xTimers = xTimerCreate("Timer",
												 pdMS_TO_TICKS(UART_RRAME_TIME_GAP_MS),
												 pdFALSE,
												 (void *)0,
												 uart_map_array[i].xtimerCallbackFunction);
		if (uart_map_array[i].xTimers == NULL)
		{
			DBG_LOG("%s timers create fail\n", uart_map_array[i].uart_name);
		}
	}
	int ret = fifo_init(uart_map_array[0].pfifo, uart1_queue, sizeof(uart1_queue));
	ret += fifo_init(uart_map_array[1].pfifo, uart2_queue, sizeof(uart2_queue));
	ret += fifo_init(uart_map_array[2].pfifo, uart3_queue, sizeof(uart3_queue));
	if (ret != 0)
	{
		DBG_LOG("uart fifo init fail %d\n", ret);
	}
#if 1
	/*uart read task */

	osThreadId uart_core_task_read_handle;
	osThreadDef(uart_read_core, uart_core_read_task_function, osPriorityNormal, 0, 64);
	uart_core_task_read_handle = osThreadCreate(osThread(uart_read_core), NULL);
	if (uart_core_task_read_handle == NULL)
	{
		DBG_LOG("uart_core_read_task_function create fail\n");
	}
	//this make uart_core_read_task_function can enter
	vTaskDelay(10);
#endif
}
