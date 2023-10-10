#ifndef _USER_LED_H_
#define _USER_LED_H_

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


void RGB_init(void);
/*对RGB进行反向初始化，并关闭RGB线程，需要与RGB_init()成对出现*/
void RGB_deinit(void);

void user_rgb_send_queue(uint8_t r, uint8_t g, uint8_t b);

#endif
