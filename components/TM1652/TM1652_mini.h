#ifndef _TM1652_MINI_H_
#define _TM1652_MINI_H_

#include <stdint.h>
#include "TM1652_dev.h"

void TM1652_mini_ble_icon(bool en);
void TM1652_mini_set_time(uint32_t time_val);
void TM1652_mini_set_rpm(uint32_t val);
void TM1652_mini_set_res(uint32_t val);
void TM1652_mini_set_cal(uint32_t val);
void TM1652_mini_set_dir(uint32_t val);
void TM1652_mini_set_num(uint32_t num);
void TM1652_mini_show(void);
void TM1652_mini_set_down(uint32_t val);
void TM1652_mini_set_idle(void);
/*每100ms调用一次*/
void TM1652_mini_ota_anime(void);
void TM1652_mini_error_code(uint8_t error_code);
/*has_batt指示是否需要显示电池标*/
void TM1652_mini_boot(bool has_batt);
void TM1652_mini_batt_icon(uint8_t batt_val);
void TM1652_mini_show_clean(void);

#endif
