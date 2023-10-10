#include "TM1652_mini.h"
#include "esp_log.h"

#define TAG "TM1652_mini"


#define TM1652_MINI_NUM_ZERO    (0XFC)
#define TM1652_MINI_NUM_ONE     (0X60)
#define TM1652_MINI_NUM_TWO     (0XDA)
#define TM1652_MINI_NUM_THREE   (0XF2)
#define TM1652_MINI_NUM_FOUR    (0X66)
#define TM1652_MINI_NUM_FIVE    (0XB6)
#define TM1652_MINI_NUM_SIX     (0XBE)
#define TM1652_MINI_NUM_SEVEB   (0XE0)
#define TM1652_MINI_NUM_EIGHT   (0XFE)
#define TM1652_MINI_NUM_NINE    (0XF6)
#define TM1652_MINI_LETTER_E    (0X9E)  //字母E

enum TM1652_ICON_LIST
{
    TM1652_TIME_ICON,
    TM1652_RPM_ICON,
    TM1652_RES_ICON,
    TM1652_CAL_ICON,
    TM1652_DIR_ICON,
    TM1652_OFF_ICON,
};

// {0XEE, 0X3E, 0X9C, 0X7A, 0X9E, 0X8E, 0X7C, 0X6E, 0X08, 0X70, 0X5E}; //共阴数码管a~k字型码，需根据硬件接法定义
/*显示缓冲区*/
static uint8_t disp_buf[] = {0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
/*数字的数据*/
static const uint8_t num_data[] =
    {
        0XFC,
        0X60,
        0XDA,
        0XF2,
        0X66,
        0XB6,
        0XBE,
        0XE0,
        0XFE,
        0XF6,
};

/*设置第一位数字,传入数字如果大于十，那么不显示*/
static inline void TM1652_mini_set_num_1(uint8_t num)
{
    disp_buf[1] &= (~(num_data[8]));
    if (num < 10)
    {
        disp_buf[1] |= num_data[num];
    }
}

/*设置第二位数字,传入数字如果大于十，那么不显示*/
static inline void TM1652_mini_set_num_2(uint8_t num)
{
    disp_buf[2] &= (~(num_data[8]));
    if (num < 10)
    {
        disp_buf[2] |= num_data[num];
    }
}

/*设置第三位数字,传入数字如果大于十，那么不显示*/
static inline void TM1652_mini_set_num_3(uint8_t num)
{
    disp_buf[3] &= (~(num_data[8]));
    if (num < 10)
    {
        disp_buf[3] |= num_data[num];
    }
}

/*设置第四位数字,传入数字如果大于十，那么不显示*/
static inline void TM1652_mini_set_num_4(uint8_t num)
{
    disp_buf[4] &= (~(num_data[8]));
    if (num < 10)
    {
        disp_buf[4] |= num_data[num];
    }
}

/*冒号*/
static inline void TM1652_mini_set_colon(bool en)
{
    if(en==true){
        disp_buf[2] |= ((0x01));
    }else{
        disp_buf[2] &= (~(0x01));
    }
}

static inline void TM1652_mini_set_time_icon(bool en)
{
    if(en==true){
        disp_buf[1] |= ((0x01));
    }else{
        disp_buf[1] &= (~(0x01));
    }
}

static inline void TM1652_mini_set_rpm_icon(bool en)
{
    if(en==true){
        disp_buf[5] |= ((0x80));
    }else{
        disp_buf[5] &= (~(0x80));
    }
}

static inline void TM1652_mini_set_res_icon(bool en)
{
    if(en==true){
        disp_buf[5] |= ((0x40));
    }else{
        disp_buf[5] &= (~(0x40));
    }
}

static inline void TM1652_mini_set_cal_icon(bool en)
{
    if(en==true){
        disp_buf[5] |= ((0x20));
    }else{
        disp_buf[5] &= (~(0x20));
    }
}

static inline void TM1652_mini_set_dir_icon(bool en)
{
    if(en==true){
        disp_buf[5] |= ((0x10));
    }else{
        disp_buf[5] &= (~(0x10));
    }
}

static inline void M1652_mini_set_icon(uint8_t sw)
{
    switch(sw)
    {
        case(TM1652_TIME_ICON):
        {
            TM1652_mini_set_time_icon(false);
            TM1652_mini_set_rpm_icon(false);
            TM1652_mini_set_res_icon(true);
            TM1652_mini_set_cal_icon(false);
            TM1652_mini_set_dir_icon(false);
        }break;
        case(TM1652_RPM_ICON):
        {
            TM1652_mini_set_time_icon(false);
            TM1652_mini_set_rpm_icon(true);
            TM1652_mini_set_res_icon(false);
            TM1652_mini_set_cal_icon(false);
            TM1652_mini_set_dir_icon(false);
        }break;
        case(TM1652_RES_ICON):
        {
            TM1652_mini_set_time_icon(true);
            TM1652_mini_set_rpm_icon(false);
            TM1652_mini_set_res_icon(false);
            TM1652_mini_set_cal_icon(false);
            TM1652_mini_set_dir_icon(false);
        }break;
        case(TM1652_CAL_ICON):
        {
            TM1652_mini_set_time_icon(false);
            TM1652_mini_set_rpm_icon(false);
            TM1652_mini_set_res_icon(false);
            TM1652_mini_set_cal_icon(false);
            TM1652_mini_set_dir_icon(true);
        }break;
        case(TM1652_DIR_ICON):
        {
            TM1652_mini_set_time_icon(false);
            TM1652_mini_set_rpm_icon(false);
            TM1652_mini_set_res_icon(false);
            TM1652_mini_set_cal_icon(true);
            TM1652_mini_set_dir_icon(false);
        }break;    
        case(TM1652_OFF_ICON):
        {
            TM1652_mini_set_time_icon(false);
            TM1652_mini_set_rpm_icon(false);
            TM1652_mini_set_res_icon(false);
            TM1652_mini_set_cal_icon(false);
            TM1652_mini_set_dir_icon(false);
        }break;
        default :
        {
            TM1652_mini_set_time_icon(false);
            TM1652_mini_set_rpm_icon(false);
            TM1652_mini_set_res_icon(false);
            TM1652_mini_set_cal_icon(false);
            TM1652_mini_set_dir_icon(false);
        }break;
    }
}

void TM1652_mini_ble_icon(bool en)
{
    static uint8_t timer = 0;
    if (en == true)
    {
        disp_buf[5] |= ((0x08));
        timer = 0;
    }
    else
    {
        switch (timer)
        {
        case 0:
            disp_buf[5] |= ((0x08));
            break;
        case 2:
            disp_buf[5] &= (~(0x08));
            break;

        default:
            break;
        }
        if (timer == 10)
        {
            timer = 0;
        }
        else
        {
            timer++;
        }
    }
}

void TM1652_mini_batt_icon(uint8_t batt_val)
{
    static uint8_t timer = 0;
    if (batt_val >= 30)
    {
        disp_buf[4] &= (~(0x01));
        timer = 0;
    }
    else
    {
        switch (timer)
        {
        case 0:
            disp_buf[4] |= ((0x01));
            break;
        case 2:
            disp_buf[4] &= (~(0x01));
            break;

        default:
            break;
        }
        if (timer == 10)
        {
            timer = 0;
        }
        else
        {
            timer++;
        }
    }
}

static inline void TM1652_set_dot(void)
{
    disp_buf[3] |= 0x01;
}

static inline void TM1652_reset_dot(void)
{
    disp_buf[3] &= (~(0x01));
}

void TM1652_mini_set_down(uint32_t val)
{
    M1652_mini_set_icon(TM1652_OFF_ICON);
    TM1652_reset_dot();
    TM1652_mini_set_colon(false);//熄灭冒号
    disp_buf[1] &= (~(num_data[8]));
    disp_buf[2] &= (~(num_data[8]));
    disp_buf[3] &= (~(num_data[8]));
    disp_buf[4] &= (~(num_data[8]));
    disp_buf[3] |= num_data[val % 10];
}

void TM1652_mini_set_time(uint32_t time_val)
{
    if (time_val > 5999) // 如果超过了5999，那么数码管的显示上限就到了，不会再继续多了
    {
        time_val = 5999;
    }
    uint32_t timer_s = time_val % 60;
    uint32_t timer_m = time_val / 60;
    M1652_mini_set_icon(TM1652_TIME_ICON);
    TM1652_mini_set_colon(true); // 点亮冒号
    TM1652_reset_dot();
    if (timer_m >= 10)
    {
        TM1652_mini_set_num_1(timer_m / 10);
    }
    else
    {
        TM1652_mini_set_num_1(0xff);
    }
    TM1652_mini_set_num_2(timer_m % 10);
    TM1652_mini_set_num_3(timer_s / 10);
    TM1652_mini_set_num_4(timer_s % 10);
}

void TM1652_mini_set_num(uint32_t num)
{
    TM1652_mini_set_num_1(num % 10000 / 1000);
    TM1652_mini_set_num_2(num % 1000 / 100);
    TM1652_mini_set_num_3(num % 100 / 10);
    TM1652_mini_set_num_4(num % 10);
}

/*有数字才会显示，没数字的时候显示为黑*/
void TM1652_mini_set_valid_num(uint32_t val)
{
    if (val < 10)
    {
        TM1652_mini_set_num_1(0xff);
        TM1652_mini_set_num_2(0xff);
        TM1652_mini_set_num_3(val % 10);
        TM1652_mini_set_num_4(0xff);
    }
    else if (val < 100)
    {
        TM1652_mini_set_num_1(0xff);
        TM1652_mini_set_num_2(val % 100 / 10);
        TM1652_mini_set_num_3(val % 10);
        TM1652_mini_set_num_4(0xff);
    }
    else if (val < 1000)
    {
        TM1652_mini_set_num_1(val % 1000 / 100);
        TM1652_mini_set_num_2(val % 100 / 10);
        TM1652_mini_set_num_3(val % 10);
        TM1652_mini_set_num_4(0xff);
    }
    else if (val < 10000)
    {
        TM1652_mini_set_num(val);
    }
}

/*有数字才会显示，没数字的时候显示为黑,右对齐*/
void TM1652_mini_right_set_valid_num(uint32_t val)
{
    if (val < 10)
    {
        TM1652_mini_set_num_1(0xff);
        TM1652_mini_set_num_2(0xff);
        TM1652_mini_set_num_3(0);
        TM1652_mini_set_num_4(val % 10);
    }
    else if (val < 100)
    {
        TM1652_mini_set_num_1(0xff);
        TM1652_mini_set_num_2(0xff);
        TM1652_mini_set_num_3(val % 100 / 10);
        TM1652_mini_set_num_4(val % 10);
    }
    else if (val < 1000)
    {
        TM1652_mini_set_num_1(0xff);
        TM1652_mini_set_num_2(val % 1000 / 100);
        TM1652_mini_set_num_3(val % 100 / 10);
        TM1652_mini_set_num_4(val % 10);
    }
    else if (val < 10000)
    {
        TM1652_mini_set_num(val);
    }
}

void TM1652_mini_set_rpm(uint32_t val)
{
    M1652_mini_set_icon(TM1652_RPM_ICON);
    TM1652_mini_set_colon(false);//熄灭冒号
    TM1652_reset_dot();
    TM1652_mini_set_valid_num(val);
}

void TM1652_mini_set_res(uint32_t val)
{
    M1652_mini_set_icon(TM1652_RES_ICON);
    TM1652_mini_set_colon(false);//熄灭冒号
    TM1652_reset_dot();
    TM1652_mini_set_valid_num(val);
}

void TM1652_mini_set_cal(uint32_t val)
{
    if(val>99999)
    {
        val = 99999;
    }
    M1652_mini_set_icon(TM1652_CAL_ICON);
    TM1652_mini_set_colon(false); // 熄灭冒号
    if (val > 9999)
    {
        TM1652_reset_dot();
        TM1652_mini_right_set_valid_num(val / 10);
    }
    else
    {
        TM1652_set_dot();
        TM1652_mini_right_set_valid_num(val);
    }
}

void TM1652_mini_set_dir(uint32_t val)
{
    if (val >= 1000000) // 距离超过999.9km时显示未999.9
    {
        val = 999999;
    }
    M1652_mini_set_icon(TM1652_DIR_ICON);
    TM1652_mini_set_colon(false);                          //熄灭冒号
    TM1652_set_dot();                                      //点亮小数点
    uint32_t mi_temp = val % 1000 / 100;                   //计算小数点后面的数字
    uint32_t km_temp = val % (1000 * 1000) / (1000 * 100); //计算百位具体有多少公里
    uint32_t km_temp_2 = val % (1000 * 100) / (1000 * 10); //计算十位具体有多少公里
    uint32_t km_temp_3 = val % (1000 * 10) / 1000;         //计算个位具体有多少公里
    if (km_temp != 0)
    {
        TM1652_mini_set_num_1(km_temp);
        TM1652_mini_set_num_2(km_temp_2);
        TM1652_mini_set_num_3(km_temp_3);
        TM1652_mini_set_num_4(mi_temp);
    }
    else
    {
        TM1652_mini_set_num_1(0xff);
        if (km_temp_2 != 0)
        {
            TM1652_mini_set_num_2(km_temp_2);
            TM1652_mini_set_num_3(km_temp_3);
            TM1652_mini_set_num_4(mi_temp);
        }
        else
        {
            TM1652_mini_set_num_2(0xff);
            TM1652_mini_set_num_3(km_temp_3);
            TM1652_mini_set_num_4(mi_temp);
        }
    }
}

void TM1652_mini_set_idle(void)
{
    disp_buf[1] |= 0xFF;
    disp_buf[2] |= 0xFF;
    disp_buf[3] |= 0xFF;
    disp_buf[4] |= 0xFE; //除了低电量指示，其余图标全亮
    disp_buf[5] |= 0xF7;
}

void TM1652_mini_boot(bool has_batt)
{
    disp_buf[1] |= 0xFF;
    disp_buf[2] |= 0xFF;
    disp_buf[3] |= 0xFF;
    if (has_batt == true)
    {
        disp_buf[4] |= 0xFF; // 除了低电量指示，其余图标全亮
    }
    else
    {
        disp_buf[4] |= 0xFE; // 除了低电量指示，其余图标全亮
    }
    disp_buf[5] |= 0xFF;
}

void TM1652_mini_ota_anime(void)
{
    static uint8_t anime_cnt = 0;
    disp_buf[1] = 0;
    disp_buf[2] = 0;
    disp_buf[3] = 0;
    disp_buf[4] = 0; // 除了低电量指示，其余图标全亮
    disp_buf[5] = 0;

    if (anime_cnt <= 2)
    {
        disp_buf[1] |= 0x02;
    }
    else if (anime_cnt <= 4)
    {
        disp_buf[1] |= 0x02;
        disp_buf[2] |= 0x02;
    }
    else if (anime_cnt <= 6)
    {
        disp_buf[1] |= 0x02;
        disp_buf[2] |= 0x02;
        disp_buf[3] |= 0x02;
    }
    else if (anime_cnt <= 10)
    {
        disp_buf[1] |= 0x02;
        disp_buf[2] |= 0x02;
        disp_buf[3] |= 0x02;
        disp_buf[4] |= 0x02; // 除了低电量指示，其余图标全亮
    }
    else if (anime_cnt < 13)
    {
    }
    else if (anime_cnt == 13)
    {
        anime_cnt = 0;
    }
    anime_cnt++;
}

void TM1652_mini_error_code(uint8_t error_code)
{
    disp_buf[1] = 0;
    disp_buf[2] = 0;
    disp_buf[3] = 0;
    disp_buf[4] = 0; // 除了低电量指示，其余图标全亮
    disp_buf[5] = 0;
    disp_buf[1] |= TM1652_MINI_LETTER_E;
    disp_buf[2] |= num_data[0];
    disp_buf[3] |= num_data[error_code];
}

void TM1652_mini_show_clean(void)
{
    disp_buf[1] = 0;
    disp_buf[2] = 0;
    disp_buf[3] = 0;
    disp_buf[4] = 0;
    disp_buf[5] = 0;
}

void TM1652_mini_show(void)
{
    TM1652_send_data(disp_buf,sizeof(disp_buf));
}

