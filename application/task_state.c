#include "task_state.h"
#include "cmsis_os.h"
#include "stdio.h"
#include "console.h"
#include "string.h"


static osThreadId task_state_handle;


void task_state_function(void const * argument)
{
	char buff[240] = {0x00,};
	uint8_t counter = 0;
	while(1){
		if(counter == 0){
			vTaskSuspend(NULL);
			counter++;
		}else{
		  if(counter++ < 4){
			vTaskList(buff);
			DBG_LOG("%s\n",buff);
			vTaskGetRunTimeStats(buff);
			DBG_LOG("%s\n",buff);
			vTaskDelay(1000);
		  }else{
			counter = 0;
		  }
		}	
	}
}


void task_uart_commond_excute_func(uint8_t *data,uint8_t size)
{
	DBG_LOG("task_uart_commond_excute_func\n");
	vTaskResume(task_state_handle);
}

void create_state_task(void)
{
	user_error_t ret;
	osThreadDef(task_state, task_state_function, osPriorityLow, 0, 256);
	task_state_handle = osThreadCreate(osThread(task_state), NULL);
	if(task_state_handle == NULL){
		DBG_LOG("task_state_function create fail\n");
	}
	ret = console_commond_register("CPU",strlen("CPU"),task_uart_commond_excute_func,"cpu usage,stack, left stack");
	if(ret != RET_OK){
		DBG_LOG("cpu commond register fail\n");
	}
	osDelay(20);
	
}






