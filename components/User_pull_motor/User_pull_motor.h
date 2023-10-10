#ifndef _USER_PULL_MOTOR_H_
#define _USER_PULL_MOTOR_H_


#include <stdint.h>


#define FRAME_HEADER 				0XFE
#define FRAME_TAIL	 				0XFF


#define FRAME_READ	 				0X21
#define FRAME_WHITE	 				0X20

//M1M
#define RES_MAX						347

//M2M
// #define RES_MAX						460

#define RES_MIN						180

#define FRAME_GET_MOTOR_STATE		(44)
#define FRAME_GET_COUNT				(46)
#define FRAME_GET_CLK				(76)//0x4c
#define FRAME_GET_ZERO				(48)
#define FRAME_GET_ERR				(72)
#define FRAME_GET_RPM				(74)
#define FRAME_GET_DIR				(75)
#define FRAME_GET_BATT				(77)
#define FRAME_GET_COUNT				(46)
#define FRAME_GET_GE_POWER			(80)
#define FRAME_SET_RPM_COUNT_TIME	(0X12)


void User_pull_motor(void);

void User_pull_motor_close(void);

uint8_t is_power_err(void);

uint8_t is_res_err(void);

int has_res_err(void);

int has_power_err(void);

#endif
