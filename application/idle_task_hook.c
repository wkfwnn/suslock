
#include "log_module.h"

uint32_t  tick = 0;
void vApplicationIdleHook( void )
{
	//DBG_LOG("1");
}

void vApplicationTickHook( void )
{
	++tick;
}


