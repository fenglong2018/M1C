#ifndef _USER_DEV_PWM_H_
#define _USER_DEV_PWM_H_

#include <stdint.h>

void user_dev_pwm_init(void);

void user_dev_set_pwm_0(uint8_t val);
void user_dev_set_pwm_1(uint8_t val);
void user_dev_set_pwm_2(uint8_t val);
void user_dev_set_pwm_3(uint8_t val);

#endif
