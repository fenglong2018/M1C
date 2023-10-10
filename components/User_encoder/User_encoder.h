#ifndef _USER_ENCODER_H_
#define _USER_ENCODER_H_

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "../User_Protocol/User_Protocol.h"


#define GPIO_INPUT_IO_0     34
#define GPIO_INPUT_IO_1     35
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_INPUT_IO_0) | (1ULL<<GPIO_INPUT_IO_1))

xQueueHandle gpio_evt_queue;

void set_RES_LOCK(void);
void reset_RES_LOCK(void);
void encode_init(void);
uint8_t get_RES_update_flag(void);
void IRAM_ATTR RES_update_flag_clean_2(void);
uint8_t IRAM_ATTR get_RES_update_flag_2(void);
void IRAM_ATTR RES_update_flag_clean(void);

#endif
