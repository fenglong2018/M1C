#ifndef _USER_MID_BEEP_H_
#define _USER_MID_BEEP_H_

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void user_mid_beep_init(void);
void user_mid_beep_short(void);
void user_mid_beep_long(void);

#endif
