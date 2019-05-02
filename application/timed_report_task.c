#include "iot_module_task.h"
#include "bc26_module.h"
#include "cmsis_os.h"
#include "stdio.h"
#include "user_define.h"
#include "batterySoc.h"
#include "timed_report_task.h"


void timed_report_task_func(void const * argument)
{

	uint8_t soc;
	float voltage;
	char buff[2] = {0x01,0x20};
	//bc26_module_send_data(buff,sizeof(buff));
	get_battery_soc(&soc,&voltage);
	DBG_LOG("soc is %d, voltage is %f\n",soc,voltage);
	while(1){
		osDelay(10);

	}



}

void timed_report_task_create(void)
{
	osThreadId timed_report_thread_handle;
	osThreadDef(timed_report_task, timed_report_task_func, osPriorityLow, 0, 64);
	timed_report_thread_handle = osThreadCreate(osThread(timed_report_task),NULL);
	if(timed_report_thread_handle == NULL){
		DBG_LOG("timed_report_task create fail\n");
	}
	osDelay(20);
}

