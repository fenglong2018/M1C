#include "User_CAL_power_M1S.h"

#define TAG "User_CAL_power"

/*
x	阻力									
y	rpm									
Linear model Poly33:										
     f(x,y) = p00 + p10*x + p01*y + p20*x^2 + p11*x*y + p02*y^2 + p30*x^3 + p21*x^2*y 										
                    + p12*x*y^2 + p03*y^3										
Coefficients (with 95% confidence bounds):										
       p00 =       18.55  (3.811, 33.29)										
       p10 =      0.7495  (0.3263, 1.173)										
       p01 =      -1.269  (-1.932, -0.605)										
       p20 =    -0.02732  (-0.03464, -0.02)										
       p11 =   -0.001217  (-0.007835, 0.005401)										
       p02 =     0.03218  (0.02252, 0.04184)										
       p30 =   0.0001205  (7.573e-05, 0.0001652)										
       p21 =   0.0005282  (0.0004902, 0.0005662)										
       p12 =  -1.841e-05  (-5.638e-05, 1.956e-05)										
       p03 =  -0.0001428  (-0.0001875, -9.801e-05)										
										
Goodness of fit:										
  SSE: 3847										
  R-square: 0.9981										
  Adjusted R-square: 0.9979										
  RMSE: 5.887										
*/

#define P00 18.55f
#define P10 0.7495f
#define P01 -1.269f
#define P20 -0.02732f
#define P11 -0.001217f
#define P02 0.03218f
#define P30 0.0001205f
#define P21 0.0005282f
#define P12 (-1.841e-05)
#define P03 -0.0001428

uint32_t get_M1S_power(uint32_t res_val, uint32_t rpm_val)
{
  if (rpm_val < 20)
  {
    return 0;
  }
  else if (rpm_val > 150)
  {
    rpm_val = 150;
  }
  float p00_val = P00;
  float p10_val = P10 * res_val;
  float p01_val = P01 * rpm_val;
  float p20_val = P20 * res_val * res_val;
  float p11_val = P11 * res_val * rpm_val;
  float p02_val = P02 * rpm_val * rpm_val;
  float p30_val = P30 * res_val * res_val * res_val;
  float p21_val = P21 * res_val * res_val * rpm_val;
  float p12_val = P12 * res_val * rpm_val * rpm_val;
  float p03_val = P03 * rpm_val * rpm_val * rpm_val;
  float temp = p00_val + p10_val + p01_val + p20_val +
               p11_val + p02_val + p30_val + p21_val +
               p12_val + p03_val;
  if (temp > 999) //功率超过999，那么只输出999
  {
    return 999;
  }
  return (uint32_t)temp;
}
