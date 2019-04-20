#ifndef _BC26_MODULE_H_
#define _BC26_MODULE_H_

#include <stdint.h>

#define BC26_GPIO_GROUP        GPIOA

#define BC26_PIN7_PWERKEY      GPIO_PIN_5
#define BC26_PIN7_RESET        GPIO_PIN_4

void bc26_module_start(void);
void iot_send_data(uint8_t *data,uint8_t size);
void bc26_module_init(void);
void bc26_module_selftest();
#endif

