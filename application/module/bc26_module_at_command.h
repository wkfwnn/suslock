#ifndef __BC26_AT_COMMAND_MODULE_H_
#define __BC26_AT_COMMAND_MODULE_H_

#ifndef NULL
#define NULL (void *)0
#endif

//此命令判断 模块是否正常接收数据
#define AT "AT\r"
//用来查看sim卡是否存在
#define AT_CPIN$ "AT+CPIN?\r"
//用来查询信号强度
#define AT_CSQ "AT+CSQ\r"
//Full functionality 打开
#define AT_CFUN_1 "AT+CFUN=1\r"
// Query whether the network is activated: 1 means attached to network
// successfully, while 0 means has not been attached to network.
#define AT_CGATT$ "AT+CGATT?\r"
//Query the network registration status: 1 means registered on network, while
//2 means searching the network
#define AT_CEREG$ "AT+CEREG?\r"

/*获取IP地址，如果无法获取IP地址，意味着无法进行发送数据*/
#define AT_CGPADDR_1 "AT+CGPADDR=1\r"

//查询连接状态
//The command gives details of the TA’s perceived radio connection status (i.e. to the base station). It
// returns an indication of the current state. Please note that this state is only updated when radio events,
// such as sending and receiving, take place. This means that the current state may be out of date. The
// terminal may think it is "Connected" yet cannot currently use a base station due to a change in the link
// quality，should return +CSCON: 0,1
#define AT_CSCON$ "AT+CSCON?\r"
//获取IMEI(15位)
#define AT_CGSN_1 "AT+CGSN=1\r"

// 0 Disable sleep mode
// 1 Enable light sleep and deep sleep, wakeup by PSM_EINT (Falling Edge)
// 2 Enable light sleep only, wakeup by Main UART
#define AT_QSCLK_DISABLE_SLEEP "AT+QSCLK=0\r"
#define AT_QSCLK_ENABLE_LIGHT_DEEP_SLEEP_WAKEBY_PSM_EINT "AT+QSCLK=1\r"
#define AT_QSCLK_ENABLE_LIGHT_SLEEP_WAKEBY_MAIN_UART "AT+QSCLK=2\r"

/*下面是LwM2M相关的指令*/

//设置对应要连接的IP地址以及port AT+QLWSERV="180.101.147.115",5683

#define AT_QLWSERV    "AT+QLWSERV="
#define AT_QLWSERV_IP "AT+QLWSERV=\"180.101.147.115\",5683\r"

//设置连接时的IMEI(15位) 追加15位IMEI，可以不用,
#define AT_QLWCONF    "AT+QLWCONF="

#define AT_QLWCONF_TEST "AT+QLWCONF=\"866971030221639\"\r"

//电信平台的对应object，固定值，此为写object
#define AT_QLWADDOBJ_WRITE_PAR "AT+QLWADDOBJ=19,0,1,\"0\"\r"
//电信平台对应的读object
#define AT_QLWADDOBJ_READ_PAR "AT+QLWADDOBJ=19,1,1,\"0\"\r"
//向IoT平台发起注册请求, 并设置数据接收模式为[Buffer access mode]，响应时间是300ms， 之后会有CONNECT OK的字样，或者CONNECT FAIL，其命令返回的连接状态最大的超时时间是128S,
#define AT_QLWOPEN_1 "AT+QLWOPEN=1\r"
//设置数据接收以及发送都是为Hex mode
#define AT_QLWCFG_HEX_MODE "AT+QLWCFG=\"dataformat\",1,1\r"
//发送数据,AT+QLWDATASEND=19,0,0,2,aa00,0x0100,2代表是长度为2，aa00是 0xaa，0x00,命令响应时间是300ms，如果设置CON message，timeout时间是128s
#define AT_QLWDATASEND_CON "AT+QLWDATASEND=19,0,0,"
#define AT_QLWDATASEND_CON_DATA_TEST "AT+QLWDATASEND=19,0,0,2,aa00,0x0100\r"


//The command is used to launch a deregister request to the IoT platform,
//返回OK,ERROR(300ms),之后返回CLOSE OK或者是CLOSE FAIL，timeout时间是128S
#define AT_QLWCLOSE "AT+QLWCLOSE\r"
//This command is used to delete the LwM2M context.最大的等待时间是
#define AT_QLWDEL "AT+QLWDEL\r"
/*模组关机*/
#define AT_QPOWD_0 "AT+QPOWD=0\r"

/*This is BC26 return value*/
#define ERROR "ERROR"
#define OK "OK"
#define CONNECT_OK "CONNECT OK"
#define CONNECT_FAIL "CONNECT FAIL"
#define SEND_OK "SEND OK"
#define SEND_FAIL "SEND FAIL"

#define CPIN_READY "+CPIN: READY"
#define CSQ_VALUE "+CSQ: "

//AT+CGATT? may return
#define CGATT$ "+CGATT: 1"
#define RETURN_CGATT$_OTHER  "+CGATT: 0"

#define CEREG$ "+CEREG: 0,1"
#define RETURN_CEREG$_OTHER  "+CEREG: 0,0"

#define CGSN$  "+CGSN: "
#define RETURN_CME_ERROR_OTHER  "+CME ERROR"

#define RETURN_CGPADDR_IMMEDIATELY   "+CGPADDR:"

/*接收到数据时，会返回该字段，以及接收数据的长度*/
#define RETURN_NETWORK_DATA          "+QLWDATARECV:"
//#define 

#define MAX_OTHER_VALUE_SIZE   3
typedef struct
{
    const char *at_commond;
    const char *immediately_expect_value;
	const char *immediately_other_value[MAX_OTHER_VALUE_SIZE];
    int immediately_time_out_ms;
    const void *follow_up_value;
	const void *follow_up_other_value[MAX_OTHER_VALUE_SIZE];
    int follow_up_time_out_ms;
} At_commond_struct_t;

At_commond_struct_t bc26_command[] = {
    {AT, OK, {ERROR,NULL,NULL},300, NULL,{NULL,NULL,NULL}, 0},
    {AT_CPIN$,CPIN_READY,{ERROR,NULL,NULL},5000, NULL,{NULL,NULL,NULL}, 0},
    {AT_CSQ, CSQ_VALUE,{ERROR,NULL,NULL},300, NULL,{NULL,NULL,NULL}, 0},
    {AT_CFUN_1, OK, {ERROR,RETURN_CME_ERROR_OTHER,NULL},15000, NULL, {NULL,NULL,NULL},0},
    {AT_CGPADDR_1,RETURN_CGPADDR_IMMEDIATELY,{OK,ERROR,NULL},300,NULL,{NULL,NULL,NULL},0},
    {AT_CGATT$, CGATT$,{ERROR,RETURN_CGATT$_OTHER,NULL},75000, NULL, 0},
    {AT_CEREG$,CEREG$,{ERROR,RETURN_CEREG$_OTHER,NULL},300,NULL,{NULL,NULL,NULL},0},
    {AT_CGSN_1,CGSN$,{ERROR,NULL,NULL},300,NULL,{NULL,NULL,NULL},0},
    {AT_QSCLK_DISABLE_SLEEP,OK,{ERROR,NULL,NULL},300,NULL,{NULL,NULL,NULL},0},
    {AT_QSCLK_ENABLE_LIGHT_DEEP_SLEEP_WAKEBY_PSM_EINT,OK,{ERROR,NULL,NULL},300,NULL,{NULL,NULL,NULL},0},
    {AT_QSCLK_ENABLE_LIGHT_SLEEP_WAKEBY_MAIN_UART,OK,{ERROR,NULL,NULL},300,NULL,{NULL,NULL,NULL},0},
    {AT_QLWSERV,OK,{ERROR,NULL,NULL},300,NULL,{NULL,NULL,NULL},0},
    {AT_QLWCONF,OK,{ERROR,NULL,NULL},300,NULL,0},
    {AT_QLWADDOBJ_WRITE_PAR,OK,{ERROR,NULL,NULL},300,NULL,{NULL,NULL,NULL},0},
    {AT_QLWADDOBJ_READ_PAR,OK,{ERROR,NULL,NULL},300,NULL,{NULL,NULL,NULL},0},
    {AT_QLWOPEN_1,OK,{ERROR,NULL,NULL},300,CONNECT_OK,{CONNECT_FAIL,NULL,NULL},128000},
    {AT_QLWCFG_HEX_MODE,OK,{ERROR,NULL,NULL},300,NULL,{NULL,NULL,NULL},0},
    {AT_QLWDATASEND_CON,OK,{ERROR,NULL,NULL},300,SEND_OK,{SEND_FAIL,NULL,NULL},128000},
    {AT_QLWSERV_IP,OK,{ERROR,NULL,NULL},300,NULL,{NULL,NULL,NULL},0},
	{AT_QLWDATASEND_CON_DATA_TEST,OK,{ERROR,NULL,NULL},300,SEND_OK,{SEND_FAIL,NULL,NULL},128000},
};

#endif