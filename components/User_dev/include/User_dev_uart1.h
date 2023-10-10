#ifndef _USER_DEV_UART1_H_
#define _USER_DEV_UART1_H_

#include "driver/uart.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

void User_dev_uart1_init(void);
void User_dev_uart1_send_queue(uint8_t *data_p, uint32_t len);

#endif
