#ifndef _USER_KEY_H_
#define _USER_KEY_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include <stdio.h>

#define KEY1_IO 32
#define TOUCH_KEY1_IO 33
/*按键事件组*/
#define KEY1_LONG_PRESS_EVENT       (0X01<<0)
#define KEY1_SHORT_PRESS_EVENT      (0X01<<1)
#define KEY1_TOUCH_PRESS_EVENT      (0X01<<2)
#define KEY1_LONG_TOUCH_PRESS_EVENT (0X01<<3)
#define KEY1_SWITCH_TO_PWR		    (0X01<<4)
#define KEY1_SWITCH_TO_BPM		    (0X01<<5)
#define KEY1_SWITCH_TO_RPM		    (0X01<<6)
#define KEY1_RUN_EVENT              (0X01<<7)
#define KEY1_SUSPEND_EVENT          (0X01<<8)
#define KEY1_CLEAN_SLEEP_EVENT      (0X01<<9)
#define KEY1_STOP_EVENT             (0X01<<10)

#define GUI_SPORTS_EV_G (KEY1_LONG_PRESS_EVENT |  \
                         KEY1_SHORT_PRESS_EVENT | \
                         KEY1_TOUCH_PRESS_EVENT | \
                         KEY1_SUSPEND_EVENT |     \
                         KEY1_RUN_EVENT |         \
                         KEY1_STOP_EVENT)
/*触摸事件组*/
#define TOUCH_PRESS_L_EVENT (0X01<<0)

EventGroupHandle_t KEY_Event_Handle;
/*初始化按键，并启用按键检测线程*/
void KEY_Init(void);
uint8_t get_key1(void);
uint8_t get_touch_key1(void);
void key_ev_create(void);
void KEY_Task(void *parameter);

#endif
