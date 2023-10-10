#include "esp_log.h"
#include "User_CAL_power_M2M.h"
#include "math.h"
#define TAG "User_CAL_power"

/*
x	阻力									
y	rpm									
Linear model Poly33:					
     f(x,y) = p00 + p10*x + p01*y + p20*x^2 + p11*x*y + p02*y^2 + p30*x^3 + p21*x^2*y 					
                    + p12*x*y^2 + p03*y^3					
Coefficients (with 95% confidence bounds):					
       p00 =      -35.86  (-107, 35.3)					
       p10 =      0.4642  (-1.58, 2.508)					
       p01 =       2.012  (-1.193, 5.216)					
       p20 =   -0.004452  (-0.03979, 0.03089)					
       p11 =    -0.02262  (-0.05457, 0.009341)					
       p02 =    -0.01582  (-0.06248, 0.03083)					
       p30 =  -1.849e-05  (-0.0002346, 0.0001976)					
       p21 =   0.0004481  (0.0002648, 0.0006315)					
       p12 =   0.0002116  (2.828e-05, 0.000395)					
       p03 =   6.128e-05  (-0.0001548, 0.0002774)					
					
Goodness of fit:					
  SSE: 8.968e+04					
  R-square: 0.9601					
  Adjusted R-square: 0.9568					
  RMSE: 28.42					
						
*/


#define P00 3.296f
#define P10 0.8194f
#define P01 -0.2864f
#define P20 -0.0244f
#define P11 -0.01196f
#define P02 0.0183f
#define P30 0.0001016f
#define P21 0.0005424f
#define P12 0.00008766f
#define P03 -0.00008529f

uint32_t get_M2M_power(uint32_t res_val, uint32_t rpm_val)
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

