#include "User_dev_pwm.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"

#define TAG "user_dev_pwm"

#ifdef CONFIG_IDF_TARGET_ESP32
#define LEDC_HS_TIMER          LEDC_TIMER_0
#define LEDC_HS_MODE           LEDC_HIGH_SPEED_MODE
#define LEDC_HS_CH0_GPIO       (25)
#define LEDC_HS_CH0_CHANNEL    LEDC_CHANNEL_0
#define LEDC_HS_CH1_GPIO       (26)
#define LEDC_HS_CH1_CHANNEL    LEDC_CHANNEL_1
#endif
#define LEDC_LS_TIMER          LEDC_TIMER_1
#define LEDC_LS_MODE           LEDC_LOW_SPEED_MODE
#define LEDC_LS_CH2_GPIO       (27)
#define LEDC_LS_CH2_CHANNEL    LEDC_CHANNEL_2
#define LEDC_0_CH_NUM          (4)

#define BEEP_PWM_IO            (5)
#define BEEP_PWM_CHANNEL       (LEDC_CHANNEL_3)
#define BEEP_PWM_MODE          (LEDC_HIGH_SPEED_MODE)

static bool pwm0_init_flag = false;

static const ledc_channel_config_t ledc_channel[LEDC_0_CH_NUM] = {
    {
        .channel    = LEDC_HS_CH0_CHANNEL,
        .duty       = 0,
        .gpio_num   = LEDC_HS_CH0_GPIO,
        .speed_mode = LEDC_HS_MODE,
        .hpoint     = 0,
        .timer_sel  = LEDC_HS_TIMER
    },
    {
        .channel    = LEDC_HS_CH1_CHANNEL,
        .duty       = 0,
        .gpio_num   = LEDC_HS_CH1_GPIO,
        .speed_mode = LEDC_HS_MODE,
        .hpoint     = 0,
        .timer_sel  = LEDC_HS_TIMER
    },
    {
        .channel    = LEDC_LS_CH2_CHANNEL,
        .duty       = 0,
        .gpio_num   = LEDC_LS_CH2_GPIO,
        .speed_mode = LEDC_HS_MODE,
        .hpoint     = 0,
        .timer_sel  = LEDC_HS_TIMER
    },   
    {
        .channel    = BEEP_PWM_CHANNEL,
        .duty       = 0,
        .gpio_num   = BEEP_PWM_IO,
        .speed_mode = LEDC_HS_MODE,
        .hpoint     = 0,
        .timer_sel  = LEDC_HS_TIMER
    },   
};

static void pwm_tim_init(void)
{
    static const ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_8_BIT, // resolution of PWM duty
        .freq_hz = 1760,                     // frequency of PWM signal
        .speed_mode = LEDC_HS_MODE,          // timer mode
        .timer_num = LEDC_HS_TIMER,          // timer index
        .clk_cfg = LEDC_USE_REF_TICK,        // Auto select the source clock
    };
    ledc_timer_config(&ledc_timer);
}

void user_dev_pwm_init(void)
{
    if (pwm0_init_flag == false)
    {
        pwm_tim_init();
        // Set LED Controller with previously prepared configuration
        for (int ch = 0; ch < LEDC_0_CH_NUM; ch++)
        {
            ledc_channel_config(&ledc_channel[ch]);
        }
        // Initialize fade service.
        ledc_fade_func_install(0);
        pwm0_init_flag = true;
    }
    else
    {
        ESP_LOGW(TAG, "pwm已经初始化，请勿重复初始化");
    }
}

void user_dev_pwm_deinit(void)
{
    for (int ch = 0; ch < LEDC_0_CH_NUM; ch++) {
        ledc_stop(ledc_channel[ch].speed_mode,ledc_channel[ch].channel,0);
    }
    ledc_fade_func_uninstall();
}

void user_dev_set_pwm_0(uint8_t val)
{
    if(val==0){
        ledc_set_duty_and_update(ledc_channel[0].speed_mode,
                                 ledc_channel[0].channel, 0, 0);
    }else{
        ledc_set_duty_and_update(ledc_channel[0].speed_mode,
                                 ledc_channel[0].channel, val, 0);
    }
}

void user_dev_set_pwm_1(uint8_t val)
{
    if(val==0){
        ledc_set_duty_and_update(ledc_channel[1].speed_mode,
                                 ledc_channel[1].channel, 0, 0);
    }else{
        ledc_set_duty_and_update(ledc_channel[1].speed_mode,
                                 ledc_channel[1].channel, val, 0);
    }
}

void user_dev_set_pwm_2(uint8_t val)
{
    if(val==0){
        ledc_set_duty_and_update(ledc_channel[2].speed_mode,
                                 ledc_channel[2].channel, 0, 0);
    }else{
        ledc_set_duty_and_update(ledc_channel[2].speed_mode,
                                 ledc_channel[2].channel, val, 0);
    }
}

void user_dev_set_pwm_3(uint8_t val)
{
    if(val==0){
        ledc_set_duty_and_update(ledc_channel[3].speed_mode,
                                 ledc_channel[3].channel, 0, 0);
    }else{
        ledc_set_duty_and_update(ledc_channel[3].speed_mode,
                                 ledc_channel[3].channel, val, 0);
    }
}