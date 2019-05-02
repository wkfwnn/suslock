#include "iot_module_task.h"
#include "bc26_module.h"
#include "cmsis_os.h"
#include "stdio.h"
#include "user_define.h"
#include "batterySoc.h"
#include "timed_poll_net_data_task.h"



void timed_poll_net_task_func(void * argument)
{
	while(1)
	{
		osDelay(10);
	}
}

void timed_poll_net_data_task_create(void)
{
	osThreadId timed_poll_net_thread_handle;
	osThreadDef(timed_poll_net_task, timed_poll_net_task_func, osPriorityLow, 0, 128);
	timed_poll_net_thread_handle = osThreadCreate(osThread(timed_poll_net_task),NULL);
	if(timed_poll_net_thread_handle == NULL){
		DBG_LOG("timed_poll_net_task_func create fail\n");
	}
	osDelay(20);
}

