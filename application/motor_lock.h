#ifndef __MOTOR_LOCK_H_
#define __MOTOR_LOCK_H_

#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "ble_bluetooth_module.h"

typedef struct {
	osThreadId task_handle;
	uint8_t motor_lock_task_buff[20];
	uint8_t motor_lock_task_size;
	EventGroupHandle_t motor_lock_task_event_handle;
	uint8_t motor_lock_status;
	uint8_t motor_lock_control_mode;
}motor_lock_task_struct_t;

void motor_lock_task_create(void);

#endif
