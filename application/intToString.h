#ifndef __INT_2_SRING_H_
#define __INT_2_SRING_H_

#include "stdint.h"

void intostring(uint8_t *originData,uint8_t originDataLen,uint8_t *outputData,uint8_t radix);
int myatoi(uint8_t *data,int radix);
#endif