#include "TM1652_show_task.h"
#include "TM1652.h"
#include "User_KEY.h"
#include "User_BLE.h"
#include "User_encoder.h"
#include "esp_log.h"
#include "User_Protocol.h"
#include "User_dev_inc.h"
#include "User_sports.h"
#include "User_mid_inc.h"
#include "esp_fault.h"
#include "esp_sleep.h"
#include "User_LED.h"
#include "User_WiFi.h"
#include "User_pull_motor_dev.h"
#include "User_pull_motor.h"
#include "User_NVS.h"
#define TAG "TM1652_SHOW"

enum TM1652_SHOW_LIST
{
    TM1652_SHOW_IDIE,
    TM1652_SHOW_START,
    TM1652_SHOW_RUNNING,
    TM1652_SHOW_PAUSE,
    TM1652_SHOW_PAUSE_CLOSE,
    TM1652_SHOW_STOP,
    TM1652_SHOW_RES,
    TM1652_SHOW_SHUTDOWN,
    TM1652_SHOW_OTA,
    TM1652_SHOW_ERR,
    TM1652_SHOW_E01,
};

static int idie_show_task(void)
{
    EventBits_t r_event =
        xEventGroupWaitBits(
            KEY_Event_Handle, /* 事件对象句柄 */
            GUI_SPORTS_EV_G,
            pdTRUE,  /* 退出时清除事件位 */
            pdFALSE, /* 满足感兴趣的所有事件 */
            0);      /* 指定超时事件, 一直等 */
    TM1652_mini_set_idle();
    set_RES_LOCK();       //禁止阻力变更
    if (r_event&KEY1_LONG_PRESS_EVENT)
    {
        return TM1652_SHOW_SHUTDOWN;
    }
    else if(r_event&(KEY1_SHORT_PRESS_EVENT|KEY1_RUN_EVENT))
    {
        ble_send_status(BLE_SEND_STATUS_START,00);//向APP发送开始信号
        update_Training_Status(pre_workout);//开启前读秒时状态,必须支持
        return TM1652_SHOW_START;
    }
    else if (is_EV())
    {
        if (is_0rpm_over_time(30)) // 待机十分钟后去关机
        {
            ESP_LOGI(TAG, "关机时间到，");
            return TM1652_SHOW_SHUTDOWN;
        }
    }
    else if (is_0rpm_over_time(600)) // 待机十分钟后去关机
    {
        ESP_LOGI(TAG, "关机时间到，");
        return TM1652_SHOW_SHUTDOWN;
    }
    return TM1652_SHOW_IDIE;
}

static int start_show_task(void)
{
    static uint8_t cnt = 0;
    switch (cnt)
    {
    case 0:
        user_mid_beep_short();
        TM1652_mini_set_down(3);
        break;
    case 10:
        user_mid_beep_short();
        TM1652_mini_set_down(2);
        break;
    case 20:
        user_mid_beep_short();
        TM1652_mini_set_down(1);
        break;
    case 30:
    {
        cnt = 0;
        run_timer_create();
        RES_update_flag_clean();
        reset_RES_LOCK(); //禁止阻力变更
        sports_start();
        update_Training_Status(quick_start);//设置运动状态
        xEventGroupClearBits(KEY_Event_Handle,KEY1_LONG_PRESS_EVENT);
        return TM1652_SHOW_RUNNING;
        break;
    }

    default:
        break;
    }
    cnt++;
    return TM1652_SHOW_START;
}

static int res_show_task(void)
{
    static uint8_t cnt = 0;
    static uint8_t beep_fg = 0;
    if (get_RES_update_flag())
    {
        cnt = 1;
        if (beep_fg == 0)
        {
            beep_fg = 1;
            user_mid_beep_short();
        }
    }
    else if (cnt == 10)
    {
        beep_fg = 0;
    }

    if (cnt >= 20)
    {
        cnt = 0;
        return TM1652_SHOW_RUNNING;
    }
    else
    {
        TM1652_mini_set_res(get_RES());
    }
    cnt++;
    return TM1652_SHOW_RES;
}

static bool is_M_pause_close(uint32_t timer)
{
    if (is_EV())
    {
        if (get_ble_connect())
        {
            if (timer > 30 * 10)
            {
                return true;
            }
        }
    }
    return false;
}

static bool is_M_pause_stop(uint32_t timer)
{
    if(is_EV())
    {
        if(get_ble_connect()==0)
        {
            if(timer>30 * 10)
            {
                return true;
            }
        }
    }
    return false;
}

static int pause_show_close_task(void)
{
    static uint32_t cnt = 0;
    EventBits_t r_event =
        xEventGroupWaitBits(
            KEY_Event_Handle, /* 事件对象句柄 */
            GUI_SPORTS_EV_G,
            pdTRUE,  /* 退出时清除事件位 */
            pdFALSE, /* 满足感兴趣的所有事件 */
            0);      /* 指定超时事件, 一直等 */

    /*检测到长按*/
    if (r_event & KEY1_LONG_PRESS_EVENT ||get_ble_connect()==0)
    {
        ESP_LOGI(TAG, "触发结束");
        run_timer_delete();
        update_Training_Status(post_workout); // 结束后读秒时状态,必须支持
        ble_send_status(BLE_SEND_STATUS_STOP, 00);
        cnt = 0;
        return TM1652_SHOW_STOP;
    }
    else if (r_event & (KEY1_SHORT_PRESS_EVENT | KEY1_RUN_EVENT))
    {
        ESP_LOGI(TAG, "触发暂停恢复");
        reset_run_timer_lock(); // 恢复时间计算
        reset_RES_LOCK();       // 恢复阻力变更
        cnt = 0;
        sports_resume();
        ble_send_status(BLE_SEND_STATUS_START, 00); // 向APP发送开始信号
        update_Training_Status(pre_workout);        // 开启前读秒时状态,必须支持
        return TM1652_SHOW_RUNNING;
    }
    else if (get_RPM() != 0)
    {
        ESP_LOGI(TAG, "rpm不为0，回到暂停界面");
        cnt = 0;
        return TM1652_SHOW_PAUSE;
    }
    else if (cnt >= (10 * 30 * 9)) // 4.5分钟后结算
    {
        ESP_LOGI(TAG, "触发结束");
        run_timer_delete();
        update_Training_Status(post_workout); // 结束后读秒时状态,必须支持
        ble_send_status(BLE_SEND_STATUS_STOP, 00);
        cnt = 0;
        return TM1652_SHOW_STOP;
    }
    else
    {
        TM1652_mini_show_clean();
        cnt++;
    }
    return TM1652_SHOW_PAUSE_CLOSE;
}

static int pause_show_task(void)
{
    static uint32_t cnt = 0;
    if (cnt == 0)
    {
        set_run_timer_lock(); //暂停时间计算
        set_RES_LOCK();       //禁用阻力调节
        user_mid_beep_short();
    }
    EventBits_t r_event =
        xEventGroupWaitBits(
            KEY_Event_Handle, /* 事件对象句柄 */
            GUI_SPORTS_EV_G,
            pdTRUE,  /* 退出时清除事件位 */
            pdFALSE, /* 满足感兴趣的所有事件 */
            0);      /* 指定超时事件, 一直等 */

    /*检测到长按*/
    if (r_event & KEY1_LONG_PRESS_EVENT)
    {
        ESP_LOGI(TAG, "触发结束");
        run_timer_delete();
        update_Training_Status(post_workout); // 结束后读秒时状态,必须支持
        ble_send_status(BLE_SEND_STATUS_STOP, 00);
        cnt = 0;
        return TM1652_SHOW_STOP;
    }
    /*M1S默认10分钟停止运动*/
    else if (is_EV()==false && cnt >= (10 * 60 * 10))
    {
        ESP_LOGI(TAG, "触发结束");
        run_timer_delete();
        update_Training_Status(post_workout); // 结束后读秒时状态,必须支持
        ble_send_status(BLE_SEND_STATUS_STOP, 00);
        cnt = 0;
        return TM1652_SHOW_STOP;
    }
    else if (is_M_pause_stop(cnt))
    {
        ESP_LOGI(TAG, "触发结束");
        run_timer_delete();
        update_Training_Status(post_workout); // 结束后读秒时状态,必须支持
        ble_send_status(BLE_SEND_STATUS_STOP, 00);
        cnt = 0;
        return TM1652_SHOW_STOP;
    }
    else if (is_M_pause_close(cnt))
    {
        ESP_LOGI(TAG, "进入暂停息屏界面");
        cnt = 0;
        return TM1652_SHOW_PAUSE_CLOSE;
    }
    else if (r_event & (KEY1_SHORT_PRESS_EVENT | KEY1_RUN_EVENT))
    {
        ESP_LOGI(TAG, "触发暂停恢复");
        reset_run_timer_lock(); // 恢复时间计算
        reset_RES_LOCK();       //恢复阻力变更
        cnt = 0;
        sports_resume();
        ble_send_status(BLE_SEND_STATUS_START,00);//向APP发送开始信号
        update_Training_Status(pre_workout);//开启前读秒时状态,必须支持
        return TM1652_SHOW_RUNNING;
    }
    else if(is_has_rpm_3s())
    {
        ESP_LOGI(TAG, "触发暂停恢复");
        reset_run_timer_lock(); //恢复时间计算
        reset_RES_LOCK();       //恢复阻力变更
        cnt = 0;
        sports_resume();
        ble_send_status(BLE_SEND_STATUS_START,00);//向APP发送开始信号
        update_Training_Status(pre_workout);//开启前读秒时状态,必须支持
        return TM1652_SHOW_RUNNING;
    }
    else
    {
        TM1652_mini_set_time(get_timer());
        cnt++;
    }
    return TM1652_SHOW_PAUSE;
}

static int stop_show_task(void)
{
    static uint8_t sw = 0; //默认初始处于RPM界面
    static uint32_t timer = 0; //默认初始处于RPM界面
    static uint8_t cnt = 0;
    static uint32_t dir_temp = 0;
    static uint32_t cal_temp = 0;
    if (cnt == 0)
    {
        dir_temp = get_sports_dir();
        cal_temp = get_sports_cal();
        sports_stop();
        cnt = 1;
        user_mid_beep_short();
    }
    timer++;
    EventBits_t r_event =
        xEventGroupWaitBits(
            KEY_Event_Handle,
            GUI_SPORTS_EV_G,
            pdTRUE,  /* 退出时清除事件位 */
            pdFALSE, /* 满足感兴趣的所有事件 */
            0);      /* 指定超时事件, 一直等 */
    if (r_event & KEY1_LONG_PRESS_EVENT)
    {
        timer = 0;
        return TM1652_SHOW_SHUTDOWN;
    }
    /*开始事件*/
    else if(r_event & KEY1_RUN_EVENT)
    {
        cnt = 0;
        timer = 0;
        ble_send_status(BLE_SEND_STATUS_START,00);//向APP发送开始信号
        update_Training_Status(pre_workout);//开启前读秒时状态,必须支持
        return TM1652_SHOW_START;
    }
    else if (r_event & (KEY1_SHORT_PRESS_EVENT | KEY1_RUN_EVENT) ||
             is_has_rpm_3s())
    {
        cnt = 0;
        timer = 0;
        update_Training_Status(idle);//设置空闲状态
        return TM1652_SHOW_IDIE;
    }
    else if (r_event & KEY1_TOUCH_PRESS_EVENT)
    {
        if (sw == 1)
        {
            sw = 0;
        }
        else
        {
            sw = 1;
        }
    }
    else if(timer == 15 * 10)
    {
        timer = 0;
        return TM1652_SHOW_SHUTDOWN;
    }
    if (sw == 0)
    {
        TM1652_mini_set_cal(cal_temp);
    }
    else
    {
        TM1652_mini_set_dir(dir_temp);
    }
    return TM1652_SHOW_STOP;
}

static int run_show_task(void)
{
    static uint8_t sw = 0;//默认初始处于RPM界面
    static uint8_t rpm_show_timer = 0;
    EventBits_t r_event =
        xEventGroupWaitBits(
            KEY_Event_Handle, /* 事件对象句柄 */
            GUI_SPORTS_EV_G,
            pdTRUE,  /* 退出时清除事件位 */
            pdFALSE, /* 满足感兴趣的所有事件 */
            0);      /* 指定超时事件, 一直等 */

    if (r_event & KEY1_LONG_PRESS_EVENT)
    {
        rpm_show_timer = 0;
        ESP_LOGI(TAG, "触发结束");
        run_timer_delete();
        update_Training_Status(post_workout); //结束后读秒时状态,必须支持
        ble_send_status(BLE_SEND_STATUS_STOP, 00);
        sw = 0;
        return TM1652_SHOW_STOP;
    }
    else if ((r_event & (KEY1_SHORT_PRESS_EVENT | KEY1_SUSPEND_EVENT)) ||
             (get_ble_connect()==0 && is_0rpm_over_time(30)))
    {
        rpm_show_timer = 0;
        ESP_LOGI(TAG, "触发暂停");
        set_run_timer_lock(); // 暂停时间计算
        set_RES_LOCK();       // 禁止阻力变更
        sports_pause();
        ble_send_status(BLE_SEND_STATUS_SUSPEND,00);
        sw = 0;
        return TM1652_SHOW_PAUSE;
    }
    else if (is_EV() &&
             is_0rpm_over_time(300))
    {
        rpm_show_timer = 0;
        ESP_LOGI(TAG, "超时触发结束");
        run_timer_delete();
        update_Training_Status(post_workout); // 结束后读秒时状态,必须支持
        ble_send_status(BLE_SEND_STATUS_STOP, 00);
        sw = 0;
        return TM1652_SHOW_STOP;
    }
    else if(is_0rpm_over_time(600))
    {
        rpm_show_timer = 0;
        ESP_LOGI(TAG, "超时触发结束");
        run_timer_delete();
        update_Training_Status(post_workout); //结束后读秒时状态,必须支持
        ble_send_status(BLE_SEND_STATUS_STOP, 00);
        sw = 0;
        return TM1652_SHOW_STOP;
    }
    if (r_event & KEY1_TOUCH_PRESS_EVENT)
    {
        rpm_show_timer = 0;
        /*按键切换触摸，要清0RPM计时器，防止自动暂停*/
        user_sports_send_queue(USER_SPORTS_IPS_CLEAR_0RPM, 0);
        sw++;
        if (sw >= 4)
        {
            sw = 0;
        }
    }
    if(get_RES_update_flag())
    {
        /*旋转编码器也需要清0RPM计时器，防止自动暂停*/
        user_sports_send_queue(USER_SPORTS_IPS_CLEAR_0RPM, 0);
        sw = 0;
        rpm_show_timer = 0;
        return TM1652_SHOW_RES;
    }
    switch (sw)
    {
    case 0: // rpm
    {
        if (rpm_show_timer == 0)
        {
            TM1652_mini_set_rpm(get_RPM());
        }
        rpm_show_timer++;
        if (rpm_show_timer == 10)
        {
            rpm_show_timer = 0;
        }
    }
    break;
    case 1: // 时间
    {
        rpm_show_timer = 0;
        TM1652_mini_set_time(get_timer());
    }
    break;
    case 2: //里程
    {
        rpm_show_timer = 0;
        TM1652_mini_set_dir(get_sports_dir());
    }
    break;
    case 3: //阻力显示页面
    {
        rpm_show_timer = 0;
        TM1652_mini_set_res(get_RES());
    }
    break;
    default:
        break;
    }
    return TM1652_SHOW_RUNNING;
}

static int shutdown_show_task(void)
{
    uint32_t timer = 0;

    TM1652_clear();
    user_mid_beep_short();
    set_res(0);
    RGB_deinit();
    xEventGroupClearBits(KEY_Event_Handle, GUI_SPORTS_EV_G);
    while (true)
    {
        EventBits_t r_event =
            xEventGroupWaitBits(
                KEY_Event_Handle, /* 事件对象句柄 */
                GUI_SPORTS_EV_G,
                pdTRUE,  /* 退出时清除事件位 */
                pdFALSE, /* 满足感兴趣的所有事件 */
                0);      /* 指定超时事件, 一直等 */

        /*检测到长按*/
        if (r_event & KEY1_SHORT_PRESS_EVENT)
        {
            _ESP_FAULT_RESET();
        }
        else if (get_RPM() > 15)
        {
            _ESP_FAULT_RESET();
        }
        else
        {
            if (timer >= 70)
            {
                User_pull_motor_close();
                if (timer >= 80)
                {
                    gpio_wakeup_enable(32, GPIO_INTR_LOW_LEVEL);
                    if (gpio_get_level(15) == 1)
                    {
                        gpio_wakeup_enable(15, GPIO_INTR_LOW_LEVEL);
                    }
                    else
                    {
                        gpio_wakeup_enable(15, GPIO_INTR_HIGH_LEVEL);
                    }
                    vTaskDelay(500 / portTICK_PERIOD_MS);
                    esp_sleep_enable_gpio_wakeup();
                    /* Enter sleep mode */
                    esp_light_sleep_start();
                    _ESP_FAULT_RESET();
                }
            }
        }
        timer++;
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    return TM1652_SHOW_SHUTDOWN;
}

static int err_show_task(uint32_t sw)
{
    static uint32_t timer = 0;
    static uint8_t err_sw = 0;
    if(timer==0)
    {
        if (has_res_err()||has_m1s_err())
        {
            TM1652_mini_error_code(1);
            err_sw = 0;
        }
        else if(has_power_err())
        {
            TM1652_mini_error_code(3);
            err_sw = 1;
        }
    }
    else if (timer % 10 == 0)
    {
        if (err_sw == 0)
        {
            if (has_power_err())
            {
                TM1652_mini_error_code(3);
                err_sw = 1;
            }
            else if (has_res_err())
            {
                TM1652_mini_error_code(1);
                err_sw = 0;
            }
        }
        else if (err_sw == 1)
        {
            if (has_res_err())
            {
                TM1652_mini_error_code(1);
                err_sw = 0;
            }
            else if (has_power_err())
            {
                TM1652_mini_error_code(3);
                err_sw = 1;
            }
        }
    }
    timer++;
    EventBits_t r_event =
        xEventGroupWaitBits(
            KEY_Event_Handle, /* 事件对象句柄 */
            GUI_SPORTS_EV_G,
            pdTRUE,  /* 退出时清除事件位 */
            pdFALSE, /* 满足感兴趣的所有事件 */
            0);      /* 指定超时事件, 一直等 */

    if (r_event & KEY1_LONG_PRESS_EVENT)
    {
        return TM1652_SHOW_SHUTDOWN;
    }
    else if (r_event & (KEY1_SHORT_PRESS_EVENT))
    {
        timer = 0;
        return sw;
    }
    else if (timer == 30 * 10)
    {
        return TM1652_SHOW_SHUTDOWN;
    }
    return TM1652_SHOW_ERR;
}

static int ota_show_task(void)
{
    static uint8_t err_fg = 0;
    static uint8_t err_timer = 0;
    if (err_fg == 1)
    {
        TM1652_mini_error_code(2);

        switch (err_timer)
        {
        case 0:
        {
            user_mid_beep_short();
            user_rgb_send_queue(255, 0, 0);
        }
        break;
        case 1:
        {
            user_mid_beep_short();
            user_rgb_send_queue(0, 0, 0);
        }
        break;
        case 5:
        {
            user_mid_beep_short();
            user_rgb_send_queue(255, 0, 0);
        }
        break;
        case 6:
        {
            user_mid_beep_short();
            user_rgb_send_queue(0, 0, 0);
        }
        break;
        case 10:
        {
            user_mid_beep_short();
            user_rgb_send_queue(255, 0, 0);
        }
        break;
        case 11:
        {
            user_mid_beep_short();
            user_rgb_send_queue(0, 0, 0);
        }
        break;
        case 30:
        {
            _ESP_FAULT_RESET();
        }
        break;

        default:
            break;
        }
        err_timer++;
    }
    else if (is_wifi_ota_fail_ev())
    {
        err_fg = 1;
    }
    else
    {
        TM1652_mini_ota_anime();
    }
    return TM1652_SHOW_OTA;
}

static IRAM_ATTR void TM1652_show_task(void *arg)
{   
    uint8_t sw = TM1652_SHOW_IDIE;
    uint8_t err_sw = TM1652_SHOW_IDIE;
    User_dev_uart1_init();
    TM1652_mini_boot(false);
    TM1652_mini_show();
    vTaskDelay(10 / portTICK_PERIOD_MS);
    TM1652_init();
    user_mid_beep_short();
    vTaskDelay(900 / portTICK_PERIOD_MS);
    set_RES_LOCK();               // 禁止阻力变更
    update_Training_Status(idle); // 设置空闲状态
    
    while (1)
    {
        /******************************************
        OTA是特权级，检测到OTA无条件进入动画
        但是已经开始关机的情况下，就不要在管了，继续关机
        *******************************************/
        if (is_wifi_ota_start() && sw != TM1652_SHOW_SHUTDOWN)
        {
            ESP_LOGI(TAG, "ota");
            sw = TM1652_SHOW_OTA;
        }
        else if(is_res_err()||is_m1s_res_err())
        {
            err_sw = sw;
            sw = TM1652_SHOW_ERR;
        }
        else if (is_power_err())
        {
            err_sw = sw;
            sw = TM1652_SHOW_ERR;
        }

        switch (sw)
        {
        case TM1652_SHOW_IDIE:
            sw = idie_show_task();
            break;
        case TM1652_SHOW_START:
            sw = start_show_task();
            break;
        case TM1652_SHOW_RUNNING:
            sw = run_show_task();
            break;
        case TM1652_SHOW_RES:
            sw = res_show_task();
            break;
        case TM1652_SHOW_PAUSE: //暂停
            sw = pause_show_task();
            break;
        case TM1652_SHOW_STOP: //暂停
            sw = stop_show_task();
            break;
        case TM1652_SHOW_SHUTDOWN:
            sw = shutdown_show_task();
            break;
        case TM1652_SHOW_OTA:
            sw = ota_show_task();
            break;
        case TM1652_SHOW_ERR:
            sw = err_show_task(err_sw);
            break;
        case TM1652_SHOW_PAUSE_CLOSE:
            sw = pause_show_close_task();
            break;
        default:
            break;
        }
        if(sw!=TM1652_SHOW_PAUSE_CLOSE)//暂停特定的息屏界面，不显示两个图标
        {
            TM1652_mini_batt_icon(get_batt());
            TM1652_mini_ble_icon(get_ble_connect()); // 根据蓝牙状态，刷新蓝牙ICON
        }
        TM1652_mini_show();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void TM1652_show_task_init(void)
{
    xTaskCreate(TM1652_show_task, "TM1652_show_task", 1024 * 2, NULL, 7, NULL);
}
