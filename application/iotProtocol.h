#ifndef __IOT_PROTOCOL_H
#define __IOT_PROTOCOL_H

#include "stdint.h"


#define REVERT_BIT_HIGH_8_LOW_8(x) ((((x&0x00ff)<<8)\
	                                + ((x&0xff00)>>8)))  
/*定时上报数据*/
#define STATUS_REPORT_FILED_FIX_ID_ORIGIN  0xaabb
#define DEVICE_ID_LEN               10

#define STATUS_REPORT_FILED_FIX_ID REVERT_BIT_HIGH_8_LOW_8(STATUS_REPORT_FILED_FIX_ID_ORIGIN)
#pragma pack(1)
typedef struct{
	uint16_t report_filed_id;
	uint8_t device_id[DEVICE_ID_LEN];
	uint8_t soc;
	uint8_t lock_status;
}status_report;
#pragma pack()


/*iot platform 命令*/
#define COMMAND_FILED_FIX_ID_RECEIVE     0x0001
#define COMMAND_FILED_FIX_ID_SEND     REVERT_BIT_HIGH_8_LOW_8(COMMAND_FILED_FIX_ID_RECEIVE)
#pragma pack(1)
typedef struct{
	uint16_t command_filed_id;
	uint16_t mid_filed;
	uint8_t lock_command;
}command_report;
#pragma pack()

//命令响应
#define COMMAND_ACK_FILED_FIX_ID_RECEIVE  0x0002
#define COMMAND_ACK_FILED_FIX_ID_SEND    REVERT_BIT_HIGH_8_LOW_8(COMMAND_ACK_FILED_FIX_ID_RECEIVE) 
#pragma pack(1)
typedef struct{
	uint16_t command_filed_id;
	uint16_t mid_filed;
	uint8_t errcode;
	uint8_t result;
}command_report_ack;
#pragma pack()

#endif