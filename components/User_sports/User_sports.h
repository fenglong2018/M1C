#ifndef _USER_SPORTS_H_
#define _USER_SPORTS_H_

#include "stdint.h"
#include "math.h"
#include "stdbool.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "User_CAL_power.h"

typedef struct User_sports
{
    uint32_t data;
    uint8_t ssid;
} User_sports_IPC_t;

enum
{
    USER_SPORTS_IPC_DISTANCE,
    USER_SPORTS_IPC_CAL,
    USER_SPORTS_IPC_SETTING_CAL,//设置卡路里单位
    USER_SPORTS_IPS_CLEAR_0RPM,
};

/*******************************************************************************************************
 * @brief 初始化单车数据线程
 *
 * @note  NULL
 *
 * @param cmd :指示需要初始化的设备型号
 *
 * @return int
 * @retval 0:初始化成功
 * @retval !=0:初始化失败
 *******************************************************************************************************/
int sports_init(void);
void sports_start(void);
void sports_pause(void);
void sports_resume(void);
void sports_stop(void);
/*******************************************************************************************************
 * @brief 返回本次运动的累计卡路里，单位为0.1卡路里(cal)
 *
 * @note  NULL
 *
 * @param void
 *
 * @return uint32_t
 * @retval 单位为0.1卡路里(cal)的卡路里值
 *******************************************************************************************************/
uint32_t get_sports_cal(void);
/*******************************************************************************************************
 * @brief 返回本次运动的累计运动距离，单位为米(M)
 *
 * @note  NULL
 *
 * @param void
 *
 * @return uint32_t
 * @retval 单位为米(M)的累计运动距离
 *******************************************************************************************************/
uint32_t get_sports_dir(void);

int is_0rpm_over_time(uint32_t time_val);

void user_sports_send_queue(uint8_t ssid, uint32_t val);

int is_has_rpm_3s(void);

#endif