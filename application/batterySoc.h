#ifndef __BATTERY_SOC_H_
#define  __BATTERY_SOC_H_

#include "stdint.h"

/*获取当前的电量0-100，adc voltage */
void get_battery_soc(uint8_t *soc,float* voltage);
#endif