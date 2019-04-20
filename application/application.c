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


void soc_start()
{
	
	//uart_core_task_create();
	create_state_task();
	console_task_create();
	//ble_task_create();
	iotmodule_task_create();
	lock_task_create();
	motor_lock_task_create();
	
#if TEST_FUNC
	test_func_start();
#endif

}


void bsp_module_start()
{
	//首先打开uart-core 模块
	uart_core_module_start();
	bc26_module_init();
	motor_module_start();
	infrared_detection_module_start();
}

void application_start()
{
	bsp_module_start();
	soc_start();
}
