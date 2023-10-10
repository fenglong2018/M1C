#include "User_CAL_power_EF25EV.h"
#include <math.h>

#define TAG "User_CAL_power"


/*
x	阻力									
y	rpm									
Linear model Poly33:									
     f(x,y) = p00 + p10*x + p01*y + p20*x^2 + p11*x*y + p02*y^2 + p30*x^3 + p21*x^2*y 									
                    + p12*x*y^2 + p03*y^3									
Coefficients (with 95% confidence bounds):									
       p00 =       1.781  (-5.344, 8.907)									
       p10 =        1.05  (0.8459, 1.254)									
       p01 =     -0.1795  (-0.5015, 0.1426)									
       p20 =    -0.03002  (-0.03355, -0.02649)									
       p11 =    -0.01065  (-0.01384, -0.007472)									
       p02 =       0.018  (0.0133, 0.0227)									
       p30 =   0.0001569  (0.0001354, 0.0001785)									
       p21 =   0.0004121  (0.0003938, 0.0004305)									
       p12 =   0.0001123  (9.41e-05, 0.0001306)									
       p03 =  -7.322e-05  (-9.496e-05, -5.148e-05)									
									
Goodness of fit:									
  SSE: 893.7									
  R-square: 0.9995									
  Adjusted R-square: 0.9995									
  RMSE: 2.838									
						
*/

#define P00 1.781f
#define P10 1.05f
#define P01 -0.1795f
#define P20 -0.03002f
#define P11 -0.01065f
#define P02 0.018f
#define P30 0.0001569f
#define P21 0.0004121f
#define P12 (0.0001123f)
#define P03 (-7.322e-05f)

uint32_t get_EF25EV_power(uint32_t res_val, uint32_t rpm_val)
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
