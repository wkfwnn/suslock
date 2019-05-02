#include "cmsis_os.h"
#include "stdio.h"
#include "console.h"
#include "string.h"
#include "deviceInfo.h"

static void device_info_write(uint8_t *data,uint8_t size)
{

}

static void device_info_read(uint8_t *data,uint8_t size)
{
	
}


void device_info_init(void)
{
	int ret;	
	ret = console_commond_register("InfoWrite",strlen("InfoWrite"),device_info_write,"device Info Write");
	if(ret != RET_OK){
		DBG_LOG("InfoWrite commond register fail\n");
	}
	ret = console_commond_register("InfoRead",strlen("InfoRead"),device_info_read,"device Info Read");
	if(ret != RET_OK){
		DBG_LOG("InfoRead commond register fail\n");
	}
	
}


