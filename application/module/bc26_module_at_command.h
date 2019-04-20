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
//设置连接时的IMEI(15位) 追加15位IMEI，可以不用,
#define AT_QLWCONF    "AT+QLWCONF="

//电信平台的对应object，固定值，此为写object
#define AT_QLWADDOBJ_WRITE_PAR "AT+QLWADDOBJ=19,0,1,\" 0 \"\r"
//电信平台对应的读object
#define AT_QLWADDOBJ_READ_PAR "AT+QLWADDOBJ=19,1,1,\" 0 \"\r"
//向IoT平台发起注册请求, 并设置数据接收模式为[Buffer access mode]，响应时间是300ms， 之后会有CONNECT OK的字样，或者CONNECT FAIL，其命令返回的连接状态最大的超时时间是128S,
#define AT_QLWOPEN_1 "AT+QLWOPEN=1\r"
//设置数据接收以及发送都是为Hex mode
#define AT_QLWCFG_HEX_MODE "AT+QLWCFG=\"dataformat \",1,1\r"
//发送数据,AT+QLWDATASEND=19,0,0,2,aa00,0x0100,2代表是长度为2，aa00是 0xaa，0x00,命令响应时间是300ms，如果设置CON message，timeout时间是128s
#define AT_QLWDATASEND_CON "AT+QLWDATASEND=19,0,0,"

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
#define SEND_OK "SEND OK"
#define CPIN_READY "+CPIN: READY"
#define CSQ_VALUE "+CSQ:"
#define CGATT$ "+CGATT: 1"
#define CEREG$ "+CEREG: 0,1"
#define CGSN$  "+CGSN:"
//#define 

typedef struct
{
    const char *at_commond;
    const char *immediately_expect_value;
    int immediately_time_out_ms;
    const char *follow_up_value;
    int follow_up_time_out_s;
} At_commond_struct_t;

At_commond_struct_t bc26_command[] = {
    {AT, OK, 300, "", 0},
    {AT_CPIN$, CPIN_READY, 5000, "", 0},
    {AT_CSQ, CSQ_VALUE, 300, "", 0},
    {AT_CFUN_1, OK, 15000, "", 0},
    {AT_CGATT$, CGATT$, 75000, "", 0},
    {AT_CEREG$,CEREG$,300,"",0},
    {AT_CGSN_1,CGSN$,300,"",0},
    {AT_QSCLK_DISABLE_SLEEP,OK,300,"",0},
    {AT_QSCLK_ENABLE_LIGHT_DEEP_SLEEP_WAKEBY_PSM_EINT,OK,300,"",0},
    {AT_QSCLK_ENABLE_LIGHT_SLEEP_WAKEBY_MAIN_UART,OK,300,"",0},
    {AT_QLWSERV,OK,300,"",0},
    {AT_QLWCONF,OK,300,"",0},
    {AT_QLWADDOBJ_WRITE_PAR,OK,300,"",0},
    {AT_QLWADDOBJ_READ_PAR,OK,300,"",0},
    {AT_QLWOPEN_1,OK,300,CONNECT_OK,128},
    {AT_QLWCFG_HEX_MODE,OK,300,"",0},
    {AT_QLWDATASEND_CON,OK,300,SEND_OK,128},
};

#endif