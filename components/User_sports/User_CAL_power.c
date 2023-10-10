#include "esp_log.h"
#include "User_CAL_power.h"
#include "User_CAL_power_M1S.h"
#include "User_CAL_power_M1M.h"
#include "User_CAL_power_M2M.h"
#include "User_CAL_power_E80.h"
#include "User_CAL_power_EF25EV.h"
#include "User_NVS.h"

#define TAG "User_CAL_power"

uint8_t model = DEV_MODEL_M1S;

uint32_t CAL_power(uint32_t res_val, uint32_t rpm_val)
{
  uint32_t temp = 0;
  switch (model)
  {
  case DEV_MODEL_M1S:
    temp = get_M1S_power(res_val, rpm_val);
    break;
  case DEV_MODEL_M1M:
    temp = get_M1M_power(res_val, rpm_val);
    break;
  case DEV_MODEL_M2M:
    temp = get_M2M_power(res_val, rpm_val);
    break;
  case DEV_MODEL_E80:
    temp = get_E80_power(res_val, rpm_val);
    break;
  case DEV_MODEL_EF25EV:
    temp = get_EF25EV_power(res_val, rpm_val);
    break;
  default:
    temp = get_M1S_power(res_val, rpm_val);
    break;
  }
  return (uint32_t)temp;
}

uint32_t get_CAL_target_res(uint32_t target_power, uint32_t rpm_val)
{
  uint32_t target_res = 0;
  uint32_t power_temp = 0;
  /*将RPM传入循环判断是否大于ftp功率阈值，如果大于，那么返回档位，最大100*/
  for (size_t i = 1; i <= 100; i++)
  {
    power_temp = CAL_power(i, rpm_val);
    target_res = i;
    if (power_temp > target_power)
    {
      break;
    }
  }
  return target_res;
}

void CAL_power_init(void)
{
  model = get_model();
}
