#include "iot_module_task.h"
#include "bc26_module.h"
#include "cmsis_os.h"
#include "stdio.h"
#include "user_define.h"
#include "batterySoc.h"
#include "iot_net_data_task.h"
#include "iotProtocol.h"
#include "intToString.h"


#define MAX_NET_DATA_SIZE 50
static uint8_t net_data[MAX_NET_DATA_SIZE + 1];
#define NET_DATA_EVENT_BITS (1 << 0)
#define REGULAR_REPORTING_EVENT_BITS  (1 << 1)

static EventGroupHandle_t iot_net_event_handle;


void command_parser_and_feedback()
{		
	char dataBuff[30] = {0x00,0x00};
	memset(dataBuff,0x00,sizeof(dataBuff));
	char sizeBuff[3] = {0x00,0x00,0x00};
	command_report command;
	command_report_ack command_ack;
	int ret = RET_ERROR;
	
	uint8_t size = strlen(net_data);
	if(size == sizeof(command_report) * 2){
		
		uint8_t offset = 0x00;
	
		strncpy(dataBuff,&net_data[offset],sizeof(command.command_filed_id) * 2);
		offset += sizeof(command.command_filed_id) * 2;
		printf("dataBuff:%s\n",dataBuff);
		command.command_filed_id = myatoi(dataBuff,16);
		memset(dataBuff,0x00,sizeof(dataBuff));
		DBG_LOG("command_filed_id = %x\n",command.command_filed_id);
		strncpy(dataBuff,&net_data[offset],sizeof(command.mid_filed) * 2);
		command.mid_filed = myatoi(dataBuff,16);
		DBG_LOG("mid_filed = %x\n",command.mid_filed);
		offset += sizeof(command.mid_filed) * 2;
		memset(dataBuff,0x00,sizeof(dataBuff));
		
		strncpy(dataBuff,&net_data[offset],sizeof(command.lock_command)* 2);
		command.lock_command = myatoi(dataBuff,16);
		memset(dataBuff,0x00,sizeof(dataBuff));	
	}else{
		ret = RET_PARA_ERR;
		goto exit;	
	}

	if(command.command_filed_id == COMMAND_FILED_FIX_ID_RECEIVE){
		if(command.lock_command == 1){
			ret = open_lock();
		}else if(command.lock_command == 0){
			ret = close_lock();
		}else{
			ret = RET_PARA_ERR;
		}
	}
	
exit:
	if(ret != RET_OK){
		command_ack.errcode = 0x01;
		command_ack.result = 0x01;
		command_ack.mid_filed = 0x00;
	}else{
		command_ack.errcode = 0x00;
		command_ack.result = 0x00;
		command_ack.mid_filed = REVERT_BIT_HIGH_8_LOW_8(command.mid_filed);
	}
	
	command_ack.command_filed_id = COMMAND_ACK_FILED_FIX_ID_SEND;
	size = sizeof(command_ack);
	intostring(&size, sizeof(uint8_t), sizeBuff,10);
	intostring((uint8_t *)&command_ack,sizeof(command_ack), dataBuff,16);
	bc26_module_send_data(dataBuff,sizeBuff);	
}

void iot_net_task_func(void * argument)
{
	float voltage;
	
	EventBits_t uxBits;
    const TickType_t xTicksToWait = pdMS_TO_TICKS(portMAX_DELAY);
	
	int i = 0;
	osDelay(10000);
	create_iot_connection();
	status_report report;
	memset(&report,0x00,sizeof(report));
	report.report_filed_id = STATUS_REPORT_FILED_FIX_ID;
	report.lock_status = 1;
	uint8_t size = sizeof(report);
	memset(&report.device_id,'0',sizeof(report.device_id));
	
	while(1){
		uxBits = xEventGroupWaitBits(iot_net_event_handle, 
		                             NET_DATA_EVENT_BITS |REGULAR_REPORTING_EVENT_BITS ,
                                         pdTRUE, pdFALSE, xTicksToWait);
		if ((uxBits & NET_DATA_EVENT_BITS) != 0){
			command_parser_and_feedback();	
		}
		if ((uxBits & REGULAR_REPORTING_EVENT_BITS) != 0){
			char dataBuff[30] = {0x01,0x20};
			char sizeBuff[3] = {0x00,0x00,0x00};
			get_battery_soc(&report.soc,&voltage);
			DBG_LOG("%d\n",report.soc);
			intostring(&size, sizeof(uint8_t), sizeBuff,10);
			intostring((uint8_t *)&report,sizeof(report), dataBuff,16);
			DBG_LOG("%d,sizeBuff:%s\n",size,sizeBuff);
			DBG_LOG("%s\n",dataBuff);
			bc26_module_send_data(dataBuff,sizeBuff);	
		}
	
	}
}

static void iot_data_call_back(uint8_t *data,uint8_t size)
{
	DBG_LOG("size is %d\n",size);
	uint8_t actual_copy_size = (size >= MAX_NET_DATA_SIZE? MAX_NET_DATA_SIZE:size);
	memcpy(net_data,data,actual_copy_size);
	net_data[actual_copy_size] = 0x00; //add '\0' here
	xEventGroupSetBits(iot_net_event_handle,NET_DATA_EVENT_BITS);
}


void iot_net_data_task_create(void)
{
	
	iot_net_event_handle = xEventGroupCreate();
    if (iot_net_event_handle == NULL){
        DBG_LOG("iot_net_event_handle create fail\n");
	}else{
        xEventGroupClearBits(iot_net_event_handle, (REGULAR_REPORTING_EVENT_BITS | NET_DATA_EVENT_BITS));
    }
	
	osThreadId iot_net_thread_handle;
	osThreadDef(iot_net_task, iot_net_task_func, osPriorityLow, 0, 256);
	iot_net_thread_handle = osThreadCreate(osThread(iot_net_task),NULL);
	if(iot_net_thread_handle == NULL){
		DBG_LOG("iot_net_task_func create fail\n");
	}
	int ret = register_bc26_network_data(iot_data_call_back);
	if(ret != RET_OK){
		DBG_LOG("iot_data_call_back fail %d\n",ret);
	}
	osDelay(20);
}

