#include "iot_module_task.h"
#include "A9500_iot_module.h"
#include "bc26_module.h"
#include "cmsis_os.h"
#include "stdio.h"
#include "user_define.h"

void iot_module_function(void const * argument)
{
	//iot_module_power_and_selftest();
	DBG_LOG("start bc26 selftest\n");
	char buff[2] = {0x01,0x20};
	bc26_module_send_data(buff,sizeof(buff));
	while(1){
		vTaskDelay(1000);
	}

}

void iot_task_data_call_back(uint8_t *data,uint16_t size)
{
	#if 0
	BaseType_t xResult;
	int i = 0;
	memcpy(motor_lock_task.motor_lock_task_buff,data,
		  (motor_lock_task.motor_lock_task_size = (size >= MAX_UART_QUEUE_SIZE? MAX_UART_QUEUE_SIZE:size)));
	xResult = xEventGroupSetBits(motor_lock_task.motor_lock_task_event_handle,MOTOR_LOCK_TASK_BLE_DATA_EVENT_BITS);
	/* Was the message posted successfully */
	if( xResult != pdFAIL ){
		DBG_LOG("xEventGroupSetBits set fail\n");
	}
	#endif
	
	
}


void iotmodule_task_create()
{
#if 1
	DBG_LOG("iotmodule_task_create");
	osThreadId iot_module_thread_handle;
	osThreadDef(iot_module_task, iot_module_function, osPriorityLow, 0, 128);
	iot_module_thread_handle = osThreadCreate(osThread(iot_module_task),NULL);
	if(iot_module_thread_handle == NULL){
		DBG_LOG("iot_module_function create fail\n");
	}
	osDelay(20);
#endif
}
