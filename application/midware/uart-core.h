#ifndef __UART_CORE_H_
#define __UART_CORE_H_

#include "user_define.h"
#include "stdint.h"
#include "cmsis_os.h"
#include "fifo.h"

typedef uint8_t uart_handle_t;
typedef void (*uart_read_call_back)(struct fifo *uart_fifo);

void uart_core_module_start(void);
user_error_t uart_get_handle(const char *uart_name, uint8_t uart_name_len,uart_handle_t *handle);
user_error_t  uart_core_read_register(uart_handle_t handle,uart_read_call_back call_back_func);
user_error_t uart_write_data(uart_handle_t handle,uint8_t *pdata,uint16_t data_size);



#endif
