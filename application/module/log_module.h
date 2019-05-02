#include "stdio.h"
#include "FreeRTOS.h"
#include "string.h"


extern uint32_t tick;
#define __FILENAME__ (strrchr(__FILE__, '\\') ? (strrchr(__FILE__, '\\') + 1):__FILE__)

#define DBG_LOG (printf("[%u ms] %s line:%u \t",  tick, __FILENAME__, __LINE__), printf)
#define DBG_LOG_ISR (printf("[%u ms] %s line:%u \t", tick, __FILENAME__, __LINE__), printf)
#define DBG_LOG_WITH_OUT_TIME  (printf(" %s line:%u \t", __FILENAME__, __LINE__), printf)
//void log_module_init(void);
