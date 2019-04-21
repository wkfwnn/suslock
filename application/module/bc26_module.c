
#include "bc26_module.h"
#include "user_define.h"
#include "cmsis_os.h"
#include "stm32l1xx_hal.h"
#include "uart-core.h"
#include "bc26_module_at_command.h"
#include "fifo.h"

#define BC26_MAX_CACHE_DATA_SIZE 1024
static uart_handle_t bc26_uart_handle;
static uint8_t bc26_data[BC26_MAX_CACHE_DATA_SIZE];
static uint8_t bc26_parser_data[BC26_MAX_CACHE_DATA_SIZE + 1];
static struct fifo bc26_module_fifo;

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

static At_commond_struct_t *commandIndexPara(const char *member_command)
{
    if (member_command == NULL)
    {
        return NULL;
    }
    int i, size;
    for (i = 0; i < sizeof(bc26_command) / sizeof(At_commond_struct_t); i++)
    {
        DBG_LOG("i = %d\n", i);
        if ((size = strlen(member_command)) == strlen(bc26_command[i].at_commond))
        {
            if (strncmp(member_command, bc26_command[i].at_commond, size) == 0)
            {
                DBG_LOG("delay is %d\n", bc26_command[i].immediately_time_out_ms);
                return (&bc26_command[i]);
            }
        }
    }
    return NULL;
}
void bc26_module_start(void)
{
}

void iot_send_data(uint8_t *data, uint8_t size)
{
}

#define IMMEDIATELY_TIME_OUT_MS_UNIT 10
#define FLOW_UP_TIME_OUT_MS_UNIT 100

static int checkCommandOK(const char *command)
{
    int ret = 0;
    int immediately_time_count = 0;
    int flow_up_delay_time_cout = 0;
    int i = 0;
    At_commond_struct_t *p = commandIndexPara(command);
    if (p == NULL)
    {
        goto exit;
    }
    DBG_LOG("at command delay is %d\n", p->immediately_time_out_ms);
    //如果不够一个单位，则按照一个单位计算
    int count = p->immediately_time_out_ms / IMMEDIATELY_TIME_OUT_MS_UNIT;
    immediately_time_count = count > 0 ? count : (count + 1);
    do
    {
        int len = fifo_used(&bc26_module_fifo);
        if (len > 0)
        {
            fifo_out(&bc26_module_fifo, bc26_parser_data, len);
            bc26_parser_data[len] = 0;
			DBG_LOG("immediately:%s,expect value %s\n",bc26_parser_data,p->immediately_expect_value);
            if (strstr(p->immediately_expect_value, bc26_parser_data) != NULL)
            {
                ret = 0;
            }
        }
        i++;
        osDelay(IMMEDIATELY_TIME_OUT_MS_UNIT);
    } while (i < immediately_time_count);

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
                fifo_out(&bc26_module_fifo, bc26_parser_data, len);
                bc26_parser_data[len] = 0;
				DBG_LOG("flow-up:%s\n",bc26_parser_data);
                if (strstr(p->follow_up_value, bc26_parser_data) != NULL)
                {
                    ret = 0;
                }
            }
            i++;
            osDelay(FLOW_UP_TIME_OUT_MS_UNIT);
        } while (i < flow_up_delay_time_cout);
    }
	

exit:
    return ret;
}

int send_command(char *at_command)
{
    user_error_t sc = 0;
	DBG_LOG("send command %s\n",at_command);
    sc = uart_write_data(bc26_uart_handle, at_command, strlen(at_command));
    if (sc != RET_OK)
    {
        DBG_LOG("send %s fail ret = %d\n", at_command, sc);
    }
    int ret = checkCommandOK(at_command);
    if (ret != 0)
    {
        DBG_LOG("send %s fail\n", at_command);
    }
    return ret;
}

void bc26_module_selftest()
{
    user_error_t sc = 0;
    int ret = 0;
    //开机
    BC26_MODULE_POWER_ON_AND_NEVER_SLEEP();
    osDelay(100);
    ret = send_command(AT);
    if (ret != 0){
        DBG_LOG("send %s fail\n", AT);
    }

    ret = send_command(AT_CPIN$);

    ret = send_command(AT_CSQ);
}
void bc26_uart_read_call_back(uint8_t *data, uint16_t size)
{

    int len = fifo_in(&bc26_module_fifo, data, size);
    if (len != size)
    {
        DBG_LOG_ISR("bc26 fifo overflow %d\n", len);
    }
    DBG_LOG_ISR("bc26 size %d\n", size);
}

void bc26_module_task_function(void *argument)
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
    memset(bc26_data, 0x00, sizeof(bc26_data));
    int ret = fifo_init(&bc26_module_fifo, bc26_data, sizeof(bc26_data));
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
    // osThreadDef(bc26_task, bc26_module_task_function, osPriorityNormal, 0, 128);
    // osThreadId task_handle = osThreadCreate(osThread(bc26_task), NULL);
    // if(task_handle == NULL){
    // 	DBG_LOG("bc26_module_task_function create fail\n");
    // }
}
