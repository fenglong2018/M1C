#include "User_CAL_power_M1S.h"
#include <math.h>

#define TAG "User_CAL_power"

uint32_t get_E80_power(uint32_t res_val, uint32_t rpm_val)
{
  if (rpm_val < 20)
  {
    return 0;
  }
  else if (rpm_val > 150)
  {
    rpm_val = 150;
  }
  float k1 = (-0.00000002723) * pow(res_val, 4) +
             (0.000004555) * pow(res_val, 3) +
             (-0.0001599) * pow(res_val, 2) +
             (0.006809) * res_val +
             (0.2598);
  float k2 = 0.00006926;
  float temp = 0;
  temp = k1 * rpm_val / (1 + k2 * rpm_val * rpm_val);
  // temp = (k1 * rpm_val) / (1 + k2 * (pow(rpm_val, 2)))* (3.14 * 2) / 60;
  temp = temp * rpm_val * 3.14 * 2 / 60;
  if (temp > 999) // 功率超过999，那么只输出999
  {
    return 999;
  }
  // temp = temp*10000;
  return (uint32_t)temp;
}
