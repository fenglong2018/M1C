#include "User_pull_motor_dev.h"
#include "User_dev_ADC.h"
#include "User_Protocol.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_attr.h"
#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"
#include "User_dev_ADC.h"
#include "soc/rtc.h"
#include "User_pull_motor_rpm_dev.h"

static const char *TAG = "MCPWM_TAG";

#define GPIO_PWM0A_OUT 16 // Set GPIO 15 as PWM0A
#define GPIO_PWM0B_OUT 17 // Set GPIO 16 as PWM0B
#define MAX_MOTOR_AD (1955)
#define MIN_MOTOR_AD (1355)
#define MOTOR_AD_RANGE ((MAX_MOTOR_AD - MIN_MOTOR_AD) / 100.0f)
#define MOTOR_AD_ERROR_RANGE (12)
static uint8_t res_err = 0;
/**
 * @brief motor moves in forward direction, with duty cycle = duty %
 */
static void brushed_motor_forward(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num, float duty_cycle)
{
    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_B);
    mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_A, duty_cycle);
    mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_A, MCPWM_DUTY_MODE_0); // call this each time, if operator was previously in low/high state
}

/**
 * @brief motor moves in backward direction, with duty cycle = duty %
 */
static void brushed_motor_backward(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num, float duty_cycle)
{
    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_A);
    mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_B, duty_cycle);
    mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_B, MCPWM_DUTY_MODE_0); // call this each time, if operator was previously in low/high state
}

/**
 * @brief motor stop
 */
static void brushed_motor_stop(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num)
{
    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_A);
    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_B);
}

/**
 * @brief motor stop
 */
static void brushed_motor_stop_high(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num)
{
    mcpwm_set_signal_high(mcpwm_num, timer_num, MCPWM_OPR_A);
    mcpwm_set_signal_high(mcpwm_num, timer_num, MCPWM_OPR_B);
}

#define MOTOR_DIR_F (0)
#define MOTOR_DIR_B (1)
#define MOTOR_DIR_H (2)

int check_motor(uint32_t ad, uint8_t state)
{
    static bool init_fg = false;
    static uint32_t old_pos = 0;
    static uint32_t mv_low_200_fg = 0;
    static uint32_t ok_err_cnt = 0;
    if (init_fg == false)
    {
        old_pos = ad;
        init_fg = true;
        return 0;
    }
    int cha = ad - old_pos;
    if (cha > 100 || cha < -100)
    {
        ESP_LOGE(TAG, "阻力异常4,cha  =%d", cha);
        return -1;
    }
    else
    {
        switch (state)
        {
        case MOTOR_DIR_F:
        {
            ok_err_cnt++;
            if (cha < -50)
            {
                ESP_LOGE(TAG, "阻力异常1,cha  =%d", cha);
                return -1;
            }
        }
        break;
        case MOTOR_DIR_B:
        {
            ok_err_cnt++;
            if (cha > 50)
            {
                ESP_LOGE(TAG, "阻力异常2,cha  =%d", cha);
                return -1;
            }
        }
        break;
        case MOTOR_DIR_H:
        {
            ok_err_cnt=0;
            if (cha > 100 || cha < -100)
            {
                ESP_LOGE(TAG, "阻力异常3,cha  =%d", cha);
                return -1;
            }
        }
        break;

        default:
            break;
        }
    }
    if(ok_err_cnt>300)
    {
        return -2;
    }
    if (ad < 200 || ad > 2400)
    {
        ESP_LOGE(TAG, "阻力异常,发现电压小于200mv");

        if (mv_low_200_fg > 4)
        {
            return -1;
        }
        mv_low_200_fg++;
    }
    old_pos = ad;
    return 0;
}

static int res_run(uint32_t motor_target, uint32_t motor_ad_val)
{
    static uint32_t sha_che_cnt = 0;
    static int motor_dir = MOTOR_DIR_F;
    static int motor_ok_cnt = 0;
    int motor_cha = motor_target - motor_ad_val;
    int err = 0;
    if (sha_che_cnt > 0) // 如果刹车标志为真，本次刹车
    {
        sha_che_cnt--;
        brushed_motor_stop_high(MCPWM_UNIT_1, MCPWM_TIMER_0); // 强制刹车
        err = check_motor(motor_ad_val, MOTOR_DIR_H);
        // ESP_LOGE(TAG, "H1 ad %u", motor_ad_val);
    }
    else if (motor_cha < -MOTOR_AD_ERROR_RANGE)
    {
        if (motor_dir == MOTOR_DIR_F)
        {
            brushed_motor_forward(MCPWM_UNIT_1, MCPWM_TIMER_0, 100);
            // ESP_LOGI(TAG, "F ad %u", motor_ad_val);
            err = check_motor(motor_ad_val, MOTOR_DIR_F);
        }
        else
        {
            sha_che_cnt = 6; // 重置刹车标志
            motor_dir = MOTOR_DIR_F;
        }
        motor_ok_cnt = 0;
    }
    else if (motor_cha > MOTOR_AD_ERROR_RANGE)
    {
        if (motor_dir == MOTOR_DIR_B)
        {
            brushed_motor_backward(MCPWM_UNIT_1, MCPWM_TIMER_0, 100);
            err = check_motor(motor_ad_val, MOTOR_DIR_B);
            // ESP_LOGW(TAG, "B ad %u", motor_ad_val);
        }
        else
        {
            sha_che_cnt = 6; // 重置刹车标志
            motor_dir = MOTOR_DIR_B;
        }
        motor_ok_cnt = 0;
    }
    else // 转换为空闲模式
    {
        err = check_motor(motor_ad_val, MOTOR_DIR_H);
        if (motor_ok_cnt < 20)
        {
            motor_ok_cnt++;
            brushed_motor_stop_high(MCPWM_UNIT_1, MCPWM_TIMER_0); // 强制刹车
            // ESP_LOGE(TAG, "H2 ad %u", motor_ad_val);
        }
        else
        {
            brushed_motor_stop(MCPWM_UNIT_1, MCPWM_TIMER_0);
            return 1;
        }
    }
    return err;
}

void update_actual_res(uint32_t motor_ad_val)
{
    uint32_t actual_res = 0;
    /*根据实际电位器反馈计算*/
    if (motor_ad_val <= MIN_MOTOR_AD)
    {
        actual_res = 0;
    }
    else
    {
        actual_res = (motor_ad_val - MIN_MOTOR_AD)/ MOTOR_AD_RANGE;
        if (actual_res > 100) // 阻力上限是100%，所以计算出来的值不可以超过100
        {
            actual_res = 100;
        }
        else
        {
            /*根据离目标距离还有多远进行判定，如果目标小于8%了，
            那么直接等于目标值，规避来回修正位置导致的问题*/
            int32_t temp = actual_res - get_RES();
            if (abs(temp) < 8)
            {
                actual_res = get_RES();
            }
        }
    }
    set_actual_res(actual_res); // 更新实际阻力到协议缓冲内
}

static IRAM_ATTR void motor_control_task(void *arg)
{
    vTaskDelay(2000 / portTICK_RATE_MS);
    while (1)
    {
        uint8_t res = get_RES();
        uint32_t motor_ad_val = get_motor_ad();
        uint32_t motor_target = (res)*MOTOR_AD_RANGE + MIN_MOTOR_AD;

        
        ESP_LOGI(TAG, "motor_target = %u,motor_ad_val = %u",motor_target, motor_ad_val);
        int ret = res_run(motor_target, motor_ad_val);
        if (ret < 0)
        {
            ESP_LOGE(TAG, "电机故障");
            res_err = 1;
            brushed_motor_stop(MCPWM_UNIT_1, MCPWM_TIMER_0);
            break;
        }
        update_actual_res(motor_ad_val);
        vTaskDelay(50 / portTICK_RATE_MS);
    }
    while (1)
    {
        vTaskDelay(500 / portTICK_RATE_MS);
    }
}

void user_motor_init(void)
{
    mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM0A, GPIO_PWM0A_OUT);
    mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM0B, GPIO_PWM0B_OUT);

    mcpwm_config_t pwm_config;
    pwm_config.frequency = 13000; // frequency = 10K,
    pwm_config.cmpr_a = 0;        // duty cycle of PWMxA = 0
    pwm_config.cmpr_b = 0;        // duty cycle of PWMxb = 0
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_1, MCPWM_TIMER_0, &pwm_config); // Configure PWM0A & PWM0B with above settings

    xTaskCreate(motor_control_task, "motor_control_task", 4096, NULL, 5, NULL);
    user_pull_motor_rpm_dev_init();
}

uint8_t is_m1s_res_err(void)
{
    static uint8_t send = 0;
    if (res_err == 1 && send == 0)
    {
        send = 1;
        return 1;
    }
    return 0;
}

int has_m1s_err(void)
{
    return res_err;
}
