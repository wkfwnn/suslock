
#include "bc26_module.h"
#include "user_define.h"
#include "cmsis_os.h"
#include "stm32l1xx_hal.h"
#include "uart-core.h"
#include "bc26_module_at_command.h"

#define BC26_MAX_CACHE_DATA_SIZE  1024
static uart_handle_t bc26_uart_handle;
static uint8_t bc26_data[BC26_MAX_CACHE_DATA_SIZE];

#define container_of(ptr, type, member) ({\
    const typeof(((type *)0)->member) *__mptr = (ptr);\
    (type *)((char *)__mptr - offsetof(type, member)); \
})

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

int commandIndexPara(const char *member_command, At_commond_struct_t *data_struct_pointer)
{
    if (member_command == NULL)
    {
        data_struct_pointer = NULL;
        return 1;
    }
    int i, size;
	for (i = 0; i < sizeof(bc26_command) / sizeof(At_commond_struct_t); i++)
    {
        if ((size = strlen(member_command)) == strlen(bc26_command[i].at_commond))
        {
            if (strncmp(member_command, bc26_command[i].at_commond, size) == 0)
            {
                data_struct_pointer = &bc26_command[i];
                return 0;
            }
        }
    }
    return 1;
}
void bc26_module_start(void)
{
	
}

void iot_send_data(uint8_t *data, uint8_t size)
{
}

static int checkCommandOK(const char *command)
{
    At_commond_struct_t *p = NULL;
    int ret = commandIndexPara(command, p);
    if(ret != 0){
        goto exit;
    }
    //p->
exit:
    return 1;
}

int send_command(char *at_command)
{
	user_error_t sc = 0;
    sc = uart_write_data(bc26_uart_handle, at_command, strlen(at_command));
    if (sc != RET_OK)
    {
        DBG_LOG("send %s fail ret = %d\n", AT, sc);
    }
    int ret = checkCommandOK(AT);
    if (ret != 0){
        DBG_LOG("send %s fail\n",AT);
    }

}

void  bc26_module_selftest()
{
    user_error_t sc = 0;
    int ret = 0;
    //开机
    BC26_MODULE_POWER_ON_AND_NEVER_SLEEP();
    osDelay(100);
    ret = send_command(AT);
    if(ret != 0){
        DBG_LOG("send %s fail\n",AT);
    }
    ret = send_command(AT_CPIN$);
	
	ret = send_command(AT_CSQ);

}
void bc26_uart_read_call_back(uint8_t *data,uint16_t size)
{
	uint8_t i = 0;
	if(size > BC26_MAX_CACHE_DATA_SIZE) {
		size = BC26_MAX_CACHE_DATA_SIZE;
	}
	memcpy(bc26_data,data,size);
    DBG_LOG_ISR("bc26 call back\n");
	
}
void bc26_module_task_function(void * argument)
{
	user_error_t sc;
    // DBG_LOG("bc26 task func\n");
    // sc = uart_core_read_register(bc26_uart_handle,bc26_uart_read_call_back);
    // if(sc != RET_OK){
    //     DBG_LOG("bc26 uart core read register fail %d\n",sc);
    // }
    // while(1){
    //     osDelay(1000);
    // }

}
 
void bc26_module_init()
{
    user_error_t sc;
	memset(bc26_data,0x00,sizeof(bc26_data));
    sc = uart_get_handle("uart1", sizeof("uart1"), &bc26_uart_handle);
    if (sc != RET_OK)
        DBG_LOG("uart1 get handle fail\n", sc);
    DBG_LOG("handle is %d\n",bc26_uart_handle);
    sc = uart_core_read_register(bc26_uart_handle,bc26_uart_read_call_back);
    if(sc != RET_OK){
        DBG_LOG("bc26 uart core read register fail %d\n",sc);
    }
    // osThreadDef(bc26_task, bc26_module_task_function, osPriorityNormal, 0, 128);
	// osThreadId task_handle = osThreadCreate(osThread(bc26_task), NULL);
	// if(task_handle == NULL){
	// 	DBG_LOG("bc26_module_task_function create fail\n");
	// }
}
