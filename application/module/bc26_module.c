
#include "bc26_module.h"
#include "user_define.h"
#include "cmsis_os.h"
#include "stm32l1xx_hal.h"
#include "uart-core.h"
#include "bc26_module_at_command.h"
#include "fifo.h"

#define BC26_DEBUG_ENABLE 1
#if BC26_DEBUG_ENABLE
#define BC26_LOG(...) DBG_LOG(__VA_ARGS__)
#else
#define BC26_LOG(...)
#endif

#define BC26_MAX_CACHE_DATA_SIZE 512
#define BC26_FIFO_LEN BC26_MAX_CACHE_DATA_SIZE
static uart_handle_t bc26_uart_handle;
static uint8_t bc26_data[BC26_MAX_CACHE_DATA_SIZE];
static uint8_t bc26_at_ret_data[BC26_MAX_CACHE_DATA_SIZE];
static struct fifo bc26_module_fifo;

//IMEI len is 15, add \0 1 byte
#define IMEI_LEN 15
static uint8_t imei_data[IMEI_LEN + 1];

//singal quality
static uint8_t csq_singal_value;

//头2个字节描述的是字节长度,1 为添加\0
#define TMP_DATA_OFFSET 2
static uint8_t bc26_tmp_data[BC26_MAX_CACHE_DATA_SIZE + TMP_DATA_OFFSET + 1];

#define BC26_DATA_EVENT_BITS (1 << 0)
static EventGroupHandle_t bc26_data_event_handle;

static uint8_t bc26_init_flag = 0;
#define MAX_NET_WORK_CALLBACK_SIZE   2
static network_data_call_back network_data_callback_array[MAX_NET_WORK_CALLBACK_SIZE];

#define container_of(ptr, type, member) ({\
    const typeof(((type *)0)->member) *__mptr = (ptr);\
    (type *)((char *)__mptr - offsetof(type, member)); \
})


//static SemaphoreHandle_t at_command_mutex_handle;


#define BC26_MODULE_POWER_ON()                                                 \
    do                                                                         \
    {                                                                          \
        HAL_GPIO_WritePin(BC26_GPIO_GROUP, BC26_PIN7_PWERKEY, GPIO_PIN_SET);   \
        osDelay(550);                                                          \
        HAL_GPIO_WritePin(BC26_GPIO_GROUP, BC26_PIN7_PWERKEY, GPIO_PIN_RESET); \
    } while (0);

#define BC26_MODULE_POWER_ON_AND_NEVER_SLEEP()                                 \
    do                                                                         \
    {                                                                          \
        HAL_GPIO_WritePin(BC26_GPIO_GROUP, BC26_PIN7_PWERKEY, GPIO_PIN_SET);   \
        osDelay(550);                                                          \
        HAL_GPIO_WritePin(BC26_GPIO_GROUP, BC26_PIN7_PWERKEY, GPIO_PIN_RESET); \
        osDelay(50);                                                           \
        HAL_GPIO_WritePin(BC26_GPIO_GROUP, BC26_PIN7_PWERKEY, GPIO_PIN_SET);   \
    } while (0);
	
#define BC26_MODULE_RESET()                             \
		do																		   \
		{																		   \
			HAL_GPIO_WritePin(BC26_GPIO_GROUP, BC26_PIN15_RESET, GPIO_PIN_SET);   \
			osDelay(100);														   \
			HAL_GPIO_WritePin(BC26_GPIO_GROUP, BC26_PIN15_RESET, GPIO_PIN_RESET);\
		} while (0);

void bc26_send_test_data();

static At_commond_struct_t *commandIndexPara(const char *member_command)
{
    if (member_command == NULL)
    {
        return NULL;
    }
    int i, size;
    for (i = 0; i < sizeof(bc26_command) / sizeof(At_commond_struct_t); i++)
    {
        // BC26_LOG("i = %d\n", i);
        if ( strlen(member_command) >= (size = strlen(bc26_command[i].at_commond)))
        {
            if (strncmp(member_command, bc26_command[i].at_commond, size) == 0)
            {
                BC26_LOG("delay is %d\n", bc26_command[i].immediately_time_out_ms);
                return (&bc26_command[i]);
            }
        }
    }
    return NULL;
}

int iot_send_data(uint8_t *data, uint8_t size)
{
}

#define IMMEDIATELY_TIME_OUT_MS_UNIT 10
#define FLOW_UP_TIME_OUT_MS_UNIT 100

static int checkCommandOK(const char *command)
{
    int ret = 1;
    int immediately_time_count = 0;
    int flow_up_delay_time_cout = 0;
    int i = 0, j = 0;
    At_commond_struct_t *p = commandIndexPara(command);
    if (p == NULL)
    {
        goto exit;
    }
    BC26_LOG("at command delay is %d\n", p->immediately_time_out_ms);
    //如果不够一个单位，则按照一个单位计算
    int count = p->immediately_time_out_ms / IMMEDIATELY_TIME_OUT_MS_UNIT;
    immediately_time_count = count > 0 ? count : (count + 1);
    do
    {
        int len = fifo_used(&bc26_module_fifo);
        if (len > 0)
        {
            fifo_out(&bc26_module_fifo, bc26_at_ret_data, len);
            bc26_at_ret_data[len] = 0;
            BC26_LOG("immediately:%s,expect value %s\n", bc26_at_ret_data, p->immediately_expect_value);
            if (strstr(bc26_at_ret_data, p->immediately_expect_value) != NULL)
            {
                ret = 0;
                goto judge_follow_value;
            }
            for (j = 0; j < MAX_OTHER_VALUE_SIZE; j++)
            {
                if (p->immediately_other_value[j] != NULL)
                {
                    if (strstr(bc26_at_ret_data, p->immediately_other_value[j]) != NULL)
                    {
                        ret = 1;
                        goto judge_follow_value;
                    }
                }
            }
        }
        i++;
        osDelay(IMMEDIATELY_TIME_OUT_MS_UNIT);
    } while (i < immediately_time_count);

judge_follow_value:
    if (ret != 0)
    {
        goto exit;
    }

    if (p->follow_up_value != NULL)
    {
        i = 0;
        count = p->follow_up_time_out_ms / FLOW_UP_TIME_OUT_MS_UNIT;
        flow_up_delay_time_cout = count > 0 ? count : (count + 1);
        do
        {
            int len = fifo_used(&bc26_module_fifo);
            if (len > 0)
            {
                fifo_out(&bc26_module_fifo, bc26_at_ret_data, len);
                bc26_at_ret_data[len] = 0;
                BC26_LOG("flow-up:%s expect %s\n", bc26_at_ret_data, p->follow_up_value);
                if (strstr(bc26_at_ret_data, p->follow_up_value) != NULL)
                {
                    ret = 0;
                    goto exit;
                }
                for (j = 0; j < MAX_OTHER_VALUE_SIZE; j++)
                {
                    if (p->follow_up_other_value[j] != NULL)
                    {
                        if (strstr(bc26_at_ret_data, p->follow_up_other_value[j]) != NULL)
                        {
                            ret = 1;
                            goto exit;
                        }
                    }
                }
            }
            i++;
            osDelay(FLOW_UP_TIME_OUT_MS_UNIT);
        } while (i < flow_up_delay_time_cout);
    }

exit:
    BC26_LOG("checkCommand %s return %d\n", command, ret);
    return ret;
}

int send_command(char *at_command, int times)
{
    int i = 0, ret = 0;
    user_error_t sc = 0;
	//xSemaphoreTake(at_command_mutex_handle,portMAX_DELAY);
	
    while ((i++) < times)
    {
        BC26_LOG("send command %s for times %d\n", at_command, i);
        sc = uart_write_data(bc26_uart_handle, at_command, strlen(at_command));
        if (sc != RET_OK)
        {
            BC26_LOG("send %s fail ret = %d\n", at_command, sc);
            ret = RET_ERROR;
            continue;
        }
        ret = checkCommandOK(at_command);
        if (ret != 0)
        {
            BC26_LOG("send %s fail\n", at_command);
            continue;
        }
        else
        {
            ret = RET_OK;
            goto exit;
        }
    }

exit:
	//xSemaphoreGive(at_command_mutex_handle);
    return ret;
}

int bc26_module_selftest()
{
    user_error_t sc = 0;
    int ret = 0;
    //开机
    BC26_MODULE_POWER_ON_AND_NEVER_SLEEP();
    osDelay(100);
	BC26_MODULE_RESET();
	osDelay(1000);
    ret = send_command(AT, 3);
    if (ret != RET_OK)
    {
        BC26_LOG("send %s fail\n", AT);
        goto exit;
    }
	ret = send_command(AT_CGSN_1, 3);
    if (ret != RET_OK)
    {
        BC26_LOG("send %s fail\n", AT_CGSN_1);
    }
	//设置不进入sleep模式
	ret = send_command(AT_QSCLK_DISABLE_SLEEP, 3);
    if (ret != RET_OK)
    {
        BC26_LOG("send %s fail\n", AT_QSCLK_DISABLE_SLEEP);
    }
#if 0
    ret = send_command(AT_CPIN$, 3);
    if (ret != RET_OK)
    {
        BC26_LOG("send %s fail\n", AT_CPIN$);
        goto exit;
    }
    ret = send_command(AT_CSQ, 3);
    if (ret != RET_OK)
    {
        BC26_LOG("send %s fail\n", AT_CSQ);
        goto exit;
    }
#endif
    
#if 0
    ret = send_command(AT_CFUN_1);

    ret = send_command(AT_CGSN_1);

    ret = send_command(AT_CGATT$);

    ret = send_command(AT_CSCON$);

    ret = send_command(AT_CEREG$);

    bc26_send_test_data();
#endif

exit:
    return RET_OK;
}

void bc26_uart_read_call_back(struct fifo *fifo)
{

    uint16_t *tmp_len = (uint16_t *)bc26_tmp_data;
    *tmp_len = fifo_out_peek(fifo, &bc26_tmp_data[TMP_DATA_OFFSET], (sizeof(bc26_tmp_data) - TMP_DATA_OFFSET - 1));
    if (*tmp_len > 0)
    {
        bc26_tmp_data[*tmp_len] = '\0';
        xEventGroupSetBits(bc26_data_event_handle, BC26_DATA_EVENT_BITS);
    }
}

void data_care_about_prase_func(uint8_t *data, uint16_t data_len)
{
    char *tmp = NULL;
    if (data != NULL)
    {
        //update IMEI string return format : "+CGSN: 866971032315140"
        if ((tmp = strstr((char *)data, CGSN$)) != NULL)
        {
#if 1
            //BC26_LOG(" IMEI is %s\n",data);
            imei_data[IMEI_LEN] = '\0';
            strncpy((char *)imei_data, (char *)tmp + sizeof(CGSN$) - 1, IMEI_LEN);
            BC26_LOG("IMEI is %s\n", imei_data);
#endif
        } //update signal quality  return format:"+CSQ: 7,0"
        else if ((tmp = strstr((char *)data, CSQ_VALUE)) != NULL)
        {
#if 1
#define MAX_CSQ_BUFF_LEN 2
            uint8_t csq_buff[MAX_CSQ_BUFF_LEN + 1];
            memset(csq_buff, 0x00, sizeof(csq_buff));
            char *tmp1 = strstr((char *)data, ",");
            if (tmp1 != NULL)
            {
                char *csq_index = tmp + sizeof(CSQ_VALUE) - 1;
                uint8_t csq_len = tmp1 - csq_index;
                //DBG_LOG_WITH_OUT_TIME("CSQ len is %d\n",csq_len);
                if (csq_len <= MAX_CSQ_BUFF_LEN)
                {
                    memcpy(csq_buff, csq_index, csq_len);
                    //DBG_LOG_WITH_OUT_TIME("CSQ value is %s\n",csq_buff);
                    csq_singal_value = (uint8_t)atoi(csq_buff);
                    BC26_LOG("CSQ value is %d\n", csq_singal_value);
                    csq_len = 0;
                }
            }
#undef MAX_CSQ_BUFF_LEN
#endif
        }//字符串格式“+QLWDATARECV: 19,1,0,4,AAAA0000\r\n”,4为数据长度，数据是AAAA0000
		else if((tmp = strstr(data, RETURN_NETWORK_DATA)) != NULL){
			uint8_t comma_count = 0;
			uint8_t *comma_index = NULL;
			uint8_t *comma_index1 = NULL;
#define MAX_DATA_SIZE  3
			uint8_t data_size_buff[MAX_DATA_SIZE + 1];
			memset(data_size_buff,0x00,sizeof(data_size_buff));
			uint8_t i = 0;
			for(i = 0;i < data_len;i++){
				if(data[i] == ',')comma_count++;
				if(comma_count == 3)if(comma_index == NULL)comma_index = &data[i];
				if(comma_count == 4)if(comma_index1 == NULL)comma_index1 = &data[i];
			}
			//printf("comma_index:%s comma_index1:%s\n",comma_index,comma_index1);
			if(comma_index != NULL && comma_index1 != NULL){
				uint8_t size =	(comma_index1 - comma_index - 1) >= MAX_DATA_SIZE? MAX_DATA_SIZE:(comma_index1 - comma_index - 1);
				memcpy(data_size_buff,(comma_index + 1),size);
#undef MAX_DATA_SIZE
				uint8_t data_size = atoi(data_size_buff) * 2;
				uint8_t *data_index = comma_index1 + 1;
				for(i = 0;i < MAX_NET_WORK_CALLBACK_SIZE;i++){
					if(network_data_callback_array[i] != NULL){
						network_data_callback_array[i](data_index,data_size);
					}
				}
			}else{
				DBG_LOG("can not parser\n");
			}
	
		}
    }
}

void bc26_module_task_function(void *argument)
{
    EventBits_t uxBits;
    const TickType_t xTicksToWait = pdMS_TO_TICKS(portMAX_DELAY);
    while (1)
    {
        if (bc26_data_event_handle)
        {
            uxBits = xEventGroupWaitBits(bc26_data_event_handle, BC26_DATA_EVENT_BITS,
                                         pdTRUE, pdFALSE, xTicksToWait);

            if ((uxBits & BC26_DATA_EVENT_BITS) != 0)
            {
                data_care_about_prase_func(&bc26_tmp_data[TMP_DATA_OFFSET], *((uint16_t *)bc26_tmp_data));
                if (BC26_FIFO_LEN - fifo_used(&bc26_module_fifo) < *((uint16_t *)bc26_tmp_data))
                {
                    fifo_out(&bc26_module_fifo, NULL,
                             *((uint16_t *)bc26_tmp_data) - (BC26_FIFO_LEN - fifo_used(&bc26_module_fifo)));
                }
                fifo_in(&bc26_module_fifo, &bc26_tmp_data[TMP_DATA_OFFSET], *((uint16_t *)bc26_tmp_data));
            }
            else
            {
                DBG_LOG("bc26 task receive unaccepted event\n");
            }
        }
    }
}

void bc26_module_suspend()
{
}
void bc26_module_resume()
{
}

void bc26_send_test_data()
{
    int ret = 0;
    //wake up bc26 module
	uint8_t buff[30];
	memset(buff,0x00,sizeof(buff));
	static int i = 0;
	if(i == 0){
		send_command(AT_QLWUPDATE,3);
		send_command(AT_QLWRD_TEST, 3);
		return;
		#if 0
		ret = send_command(AT_QLWRD$, 3);
		if(ret != RET_OK){
			DBG_LOG("send AT_QLWRD$ fail\n");
			goto exit;
		}
		#endif
	}

	i = 0;
#if 0
    ret = send_command(AT_CFUN_1, 3);
	if(ret != RET_OK){
		DBG_LOG("send AT_CFUN_1 fail\n");
		goto exit;
	}
    ret = send_command(AT_CEREG$, 3);
	
	if(ret != RET_OK){
		DBG_LOG("send AT_CEREG$ fail\n");
		goto exit;
	}

    ret = send_command(AT_CGATT$, 3);
	if(ret != RET_OK){
		DBG_LOG("send AT_CGATT$ fail\n");
		goto exit;
	}

    ret = send_command(AT_CGPADDR_1, 3);
	
	if(ret != RET_OK){
		DBG_LOG("send AT_CGPADDR_1 fail\n");
		goto exit;
	}
	
    ret = send_command(AT_QLWSERV_IP, 3);
	if(ret != RET_OK){
		DBG_LOG("send AT_QLWSERV_IP fail\n");
		goto exit;
	}
	
	memcpy(buff,AT_QLWCONF,sizeof(AT_QLWCONF));
	strcat(buff,"\"");
	strcat(buff,imei_data);
	strcat(buff,"\"");
	strcat(buff,"\r");
		
    ret = send_command(buff, 3);
	if(ret != RET_OK){
		DBG_LOG("send AT_QLWCONF fail\n");
		goto exit;
	}
	
    ret = send_command(AT_QLWADDOBJ_WRITE_PAR, 3);
	if(ret != RET_OK){
		DBG_LOG("send AT_QLWADDOBJ_WRITE_PAR fail\n");
		goto exit;
	}
    ret = send_command(AT_QLWADDOBJ_READ_PAR, 3);
	if(ret != RET_OK){
		DBG_LOG("send AT_QLWADDOBJ_READ_PAR fail\n");
		goto exit;
	}
    osDelay(200);
    ret = send_command(AT_QLWOPEN_1, 3);
	if(ret != RET_OK){
		DBG_LOG("send AT_QLWOPEN_1 fail\n");
		goto exit;
	}
	osDelay(200);
    ret = send_command(AT_QLWCFG_HEX_MODE, 3);
	if(ret != RET_OK){
		DBG_LOG("send AT_QLWCFG_HEX_MODE fail\n");
		goto exit;
	}
	osDelay(1000);
	#if 1
   // ret = send_command(AT_QLWDATASEND_CON_DATA_TEST, 3);
	//if(ret != RET_OK){
	//	DBG_LOG("send AT_QLWDATASEND_CON_DATA_TEST fail\n");
	//	goto exit;
	//}
	ret = send_command(AT_QLWUPDATE, 3);
	if(ret != RET_OK){
		DBG_LOG("send AT_QLWUPDATE fail\n");
		goto exit;
	}
	osDelay(200);
	ret = send_command(AT_QLWRD_TEST, 3);
	if(ret != RET_OK){
		DBG_LOG("send AT_QLWRD_TEST fail\n");
		goto exit;
	}

	
	#if 1
	osDelay(200);
	
	ret = send_command(AT_QLWCLOSE, 3);
	if(ret != RET_OK){
		DBG_LOG("send AT_QLWCLOSE fail\n");
		goto exit;
	}
	ret = send_command(AT_QLWDEL, 3);
	if(ret != RET_OK){
		DBG_LOG("send AT_QLWDEL fail\n");
		goto exit;
	}
	#endif
	
#endif
exit:
	if(ret != RET_OK){
		ret = send_command(AT_QLWCLOSE, 3);
		if(ret != RET_OK){
			DBG_LOG("send AT_QLWCLOSE fail\n");
		}
		ret = send_command(AT_QLWDEL, 3);
		if(ret != RET_OK){
			DBG_LOG("send AT_QLWDEL fail\n");
		}
	}
	//return ret;
	#endif
	
}

void update_connection(void )
{
	int ret = send_command(AT_QLWUPDATE, 3);
	if(ret != RET_OK){
		DBG_LOG("send AT_QLWUPDATE fail\n");
		goto exit;
	}
exit:
	if(ret != RET_OK){
		create_iot_connection();
	}
	return;
}


int create_iot_connection(void)
{
	int ret = RET_OK;
	uint8_t buff[30];
	memset(buff,0x00,sizeof(buff));
	
	ret = send_command(AT_CFUN_1, 3);
	if(ret != RET_OK){
		DBG_LOG("send AT_CFUN_1 fail\n");
		goto exit;
	}
    ret = send_command(AT_CEREG$, 3);
	
	if(ret != RET_OK){
		DBG_LOG("send AT_CEREG$ fail\n");
		goto exit;
	}

    ret = send_command(AT_CGATT$, 3);
	if(ret != RET_OK){
		DBG_LOG("send AT_CGATT$ fail\n");
		goto exit;
	}

    ret = send_command(AT_CGPADDR_1, 3);
	
	if(ret != RET_OK){
		DBG_LOG("send AT_CGPADDR_1 fail\n");
		goto exit;
	}
	
    ret = send_command(AT_QLWSERV_IP, 3);
	if(ret != RET_OK){
		DBG_LOG("send AT_QLWSERV_IP fail\n");
		goto exit;
	}
	
	memcpy(buff,AT_QLWCONF,sizeof(AT_QLWCONF));
	strcat(buff,"\"");
	strcat(buff,imei_data);
	strcat(buff,"\"");
	strcat(buff,"\r");
		
    ret = send_command(buff, 3);
	if(ret != RET_OK){
		DBG_LOG("send AT_QLWCONF fail\n");
		goto exit;
	}
	
    ret = send_command(AT_QLWADDOBJ_WRITE_PAR, 3);
	if(ret != RET_OK){
		DBG_LOG("send AT_QLWADDOBJ_WRITE_PAR fail\n");
		goto exit;
	}
    ret = send_command(AT_QLWADDOBJ_READ_PAR, 3);
	if(ret != RET_OK){
		DBG_LOG("send AT_QLWADDOBJ_READ_PAR fail\n");
		goto exit;
	}
    osDelay(200);
    ret = send_command(AT_QLWOPEN_0, 3);
	if(ret != RET_OK){
		DBG_LOG("send AT_QLWOPEN_1 fail\n");
		goto exit;
	}
	osDelay(200);
    ret = send_command(AT_QLWCFG_HEX_MODE, 3);
	if(ret != RET_OK){
		DBG_LOG("send AT_QLWCFG_HEX_MODE fail\n");
		goto exit;
	}
	osDelay(1500);
	ret = send_command(AT_QLWUPDATE, 3);
	if(ret != RET_OK){
		DBG_LOG("send AT_QLWUPDATE fail\n");
		goto exit;
	}
	
exit:
	if(ret != RET_OK){
		ret = send_command(AT_QLWDEL, 3);
		if(ret != RET_OK){
			DBG_LOG("send AT_QLWDEL fail\n");
		}
	}
	return ret;

}


/*return RET_ERROR,command fail quality is not update,or return updated quality*/
int get_csq_singal_quality(uint8_t *quality)
{
    int ret = send_command(AT_CSQ, 3);
    if (ret != RET_OK)
    {
        ret = RET_ERROR;
    }
    else
    {
        ret = RET_OK;
    }
    *quality = csq_singal_value;
    return ret;
}

/*return -1, or real send data size*/
int bc26_module_send_data(uint8_t *dataStr, uint8_t*sizeStr)
{

	//bc26_send_test_data();

	int ret = 0;
	uint8_t buff[70];
	memset(buff,0,sizeof(buff));
	memcpy(buff,AT_QLWDATASEND_CON,sizeof(AT_QLWDATASEND_CON));
	strcat(buff,sizeStr);
	strcat(buff,",");
	strcat(buff,dataStr);
	strcat(buff,",");
	strcat(buff,CON_MODE);
	strcat(buff,"\r");
	
    ret = send_command(buff, 3);	
exit:
	if(ret != RET_OK){
		create_iot_connection();
	}
    return ret;	
}

#if 0
void bc26_module_read_task_function(void *arg)
{
    const TickType_t xTicksToWait = pdMS_TO_TICKS(portMAX_DELAY);
	EventBits_t uxBits;
	while(1){
		osDelay(15000);
		
	}

}
#endif



int bc26_module_init()
{
    user_error_t sc;
    memset(bc26_data, 0x00, sizeof(bc26_data));
    bc26_data_event_handle = xEventGroupCreate();
    if (bc26_data_event_handle == NULL)
    {
        DBG_LOG("bc26_data_event_handle group create fail\n");
    }
    else
    {
        xEventGroupClearBits(bc26_data_event_handle, (BC26_DATA_EVENT_BITS));
    }
    int ret = fifo_init(&bc26_module_fifo, bc26_data, BC26_FIFO_LEN);
    if (ret != 0)
    {
        DBG_LOG("bc26_module fifo init\n");
    }
    sc = uart_get_handle("uart1", sizeof("uart1"), &bc26_uart_handle);
    if (sc != RET_OK)
        DBG_LOG("uart1 get handle fail\n", sc);
    DBG_LOG("handle is %d\n", bc26_uart_handle);
    sc = uart_core_read_register(bc26_uart_handle, bc26_uart_read_call_back);
    if (sc != RET_OK)
    {
        DBG_LOG("bc26 uart core read register fail %d\n", sc);
    }

    osThreadDef(bc26_task, bc26_module_task_function, osPriorityNormal, 0, 128);
    osThreadId task_handle = osThreadCreate(osThread(bc26_task), NULL);
    if (task_handle == NULL)
    {
        DBG_LOG("bc26_module_task_function create fail\n");
    }
	
#if 0
	at_command_mutex_handle  = xSemaphoreCreateMutex();
	if(at_command_mutex_handle == NULL){
		DBG_LOG("xSemaphoreCreateMutex create fail\n");
		while(1);
	}

	osThreadDef(bc26_read_task, bc26_module_read_task_function, osPriorityAboveNormal, 0, 128);

    task_handle = osThreadCreate(osThread(bc26_read_task), NULL);
    if (task_handle == NULL)
    {
        DBG_LOG("bc26_module_read_task_function create fail\n");
		
    }
#endif

    osDelay(20);

    ret = bc26_module_selftest();
    if (ret != RET_OK)
    {
        DBG_LOG("bc26 module init fail %d\n", ret);
    }
    else
    {
        bc26_init_flag = 1;
    }
    return ret;
}

int register_bc26_network_data(network_data_call_back callback)
{
	int ret = RET_ERROR;
	uint8_t i;
	if(callback == NULL){

		ret = RET_PARA_ERR;
		goto exit;
	}
	
	for(i = 0;i < MAX_NET_WORK_CALLBACK_SIZE;i++){
		if(network_data_callback_array[i] == NULL){
			network_data_callback_array[i] = callback;
			ret = RET_OK;
			goto exit;
		}
	}	
exit:
	return ret;
}

