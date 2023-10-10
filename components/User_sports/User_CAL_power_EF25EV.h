#ifndef _USER_CAL_POWER_EF25EV_H_
#define _USER_CAL_POWER_EF25EV_H_

#include "stdint.h"
/*******************************************************************************************************
 * @brief 根据阻力与rpm，计算出瞬时功率
 *
 * @note  NULL
 *
 * @param res_val：uint32_t 外部传入的阻力值
 * 
 * @param rpm_val：uint32_t 外部传入的RPM值
 *
 * @return int32_t 单位为瓦特的瞬时功率值
 *******************************************************************************************************/
uint32_t get_EF25EV_power(uint32_t res_val, uint32_t rpm_val);

#endif