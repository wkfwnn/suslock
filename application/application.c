#include "cmsis_os.h"
#include "task_state.h"
#include "application.h"
#include "console.h"
#include "uart-core.h"
#include "test-core.h"
#include "lock.h"
#include "ble_bluetooth_module.h"
#include "ble_bluetooth.h"
#include "motor_module.h"
#include "motor_lock.h"
#include "infrared_detection_module.h"
#include "bc26_module.h"
#include "iot_module_task.h"
#include "timed_report_task.h"
#include "iot_net_data_task.h"
#include "rtc.h"



void soc_start()
{
	console_task_create();
	create_state_task();
	//ble_task_create();
	iotmodule_task_create();
	lock_task_create();
	motor_lock_task_create();
	timed_report_task_create();
	iot_net_data_task_create();
#if TEST_FUNC
	test_func_start();
#endif

}


void bsp_module_start()
{
	int ret = 0;
	//log_module_init();
	//首先打开uart-core 模块
	uart_core_module_start();
	ret = bc26_module_init();
	if(ret != RET_OK){
		DBG_LOG("bc26 init fail %d\n",ret);
	}
	motor_module_start();
	infrared_detection_module_start();
	rtc_init();
}

void application_start()
{
	bsp_module_start();
	soc_start();
}
