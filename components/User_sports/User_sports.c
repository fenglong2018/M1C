#include "esp_log.h"
#include "User_sports.h"
#include "User_CAL_power.h"
#include "User_Protocol.h"
#include "User_BLE_IPC.h"
#include "User_BLE.h"
#include "ble_ftms.h"
#include "User_NVS.h"

#define TAG "sports"

#define M1S_FLAGS INDOOR_BIKE_INSTANTANEOUS_SPEED |       \
                      INDOOR_BIKE_INSTANTANEOUS_CADENCE | \
                      INDOOR_BIKE_TOTAL_DISTANCE |        \
                      INDOOR_BIKE_INSTANTANEOUS_POWER |   \
                      INDOOR_BIKE_EXPENDED_ENERGY |       \
                      INDOOR_BIKE_ELAPSED_TIME |          \
                      INDOOR_BIKE_RESISTANCE_LEVEL

#define E80_FLAGS  CROSS_TRAINER_INSTANTANEOUS_SPEED |     \
                      CROSS_TRAINER_AVERAGE_SPEED |       \
                      CROSS_TRAINER_TOTAL_DISTANCE |      \
                      CROSS_TRAINER_STEP_COUNT |          \
                      CROSS_TRAINER_STRIDE_COUNT |        \
                      CROSS_TRAINER_INSTANTANEOUS_POWER | \
                      CROSS_TRAINER_ELAPSED_TIME |        \
                      CROSS_TRAINER_AVERAGE_POWER |       \
                      CROSS_TRAINER_EXPENDED_ENERGY |     \
                      CROSS_TRAINER_RESISTANCE_LEVEL
static INDOOR_BIKE_DATA_T bike_data;
static CROSS_TRAINER_DATA_T *cross_data = NULL;
static bool sports_lock = true; // 指示运动数据是否应该开始累计
static float dir_temp = 0;
static float cal_temp = 0;      // 单位默认0.1cal
static uint8_t ble_set_cnt = 0; // 用来指示是否需要计算卡路里
static uint8_t ble_dir_set_cnt = 0; // 用来指示是否需要计算里程
static xQueueHandle sports_queue = NULL;
static uint8_t cal_unit = 0; // 卡路里单位，为0表示1卡路里，为01表示0.1卡路里
static uint32_t rpm_0_timer = 0;
static uint32_t has_rpm_timer = 0;

int is_0rpm_over_time(uint32_t time_val)
{
    if (rpm_0_timer > time_val)
    {
        return 1;
    }
    return 0;
}

void inline update_0rpm_timer(uint32_t rpm)
{
    if (rpm == 0)
    {
        rpm_0_timer++;
        has_rpm_timer = 0;
    }
    else
    {
        if (rpm >= 30)
        {
            has_rpm_timer++;
        }
        rpm_0_timer = 0;
    }
}

int is_has_rpm_3s(void)
{
    if(has_rpm_timer>=3)
    {
        return 1;
    }
    return 0;
}

static void E80_sports_task(void *arg)
{
    if(cross_data==NULL)
    {
        cross_data = calloc(1,sizeof(CROSS_TRAINER_DATA_T));
        if(cross_data==NULL)
        {
            vTaskDelete(NULL);
        }
    }
    cross_data->Flags = E80_FLAGS;
    cross_data->Energy_Per_Hour = 0xFFFF; // 传输最大视为无效值
    cross_data->Energy_Per_Minute = 0xFF; // 传输最大视为无效值
    User_sports_IPC_t r_queue;
    while (true)
    {
        if (xQueueReceive(sports_queue, &r_queue, 0))
        {
            switch (r_queue.ssid)
            {
            case USER_SPORTS_IPC_DISTANCE:
            {
                dir_temp = r_queue.data;
                ble_dir_set_cnt = 6; // 同步数据后6秒不再累计新的数据
            }
            break;
            case USER_SPORTS_IPC_SETTING_CAL:
            {
                cal_unit = r_queue.data;
                ESP_LOGI(TAG, "设置卡路里单位 = %u", cal_unit);
            }
            break;
            case USER_SPORTS_IPC_CAL:
            {
                if (cal_unit == 0)
                {
                    cal_temp = r_queue.data * 10;
                }
                else
                {
                    cal_temp = r_queue.data;
                }
                ble_set_cnt = 6; // 同步数据后6秒不再累计新的数据
                ESP_LOGI(TAG, "同步卡路里 = %u", (uint32_t)cal_temp * 100);
            }
            break;
            case USER_SPORTS_IPS_CLEAR_0RPM:
            {
                rpm_0_timer = 0;
            }
            break;
            default:
                break;
            }
        }
        else
        {
            cross_data->Resistance_Level = get_actual_res();
            cross_data->Step_Per_Minute = get_RPM() * 2; // 单位是0.5/min
            cross_data->Elapsed_Time = get_timer();
            /*时速，按60rpm 20km/h进行推算*/
            cross_data->Instantaneous_Speed = cross_data->Step_Per_Minute / 2 * 33.334f;
            /*因为RPM上面乘2了,所以这个地方需要将传入的RPM除2处理*/
            cross_data->Instantaneous_Power = CAL_power(cross_data->Resistance_Level,
                                                      cross_data->Step_Per_Minute / 2);
            if (sports_lock == false)
            {
                if (cross_data->Step_Per_Minute > 0)
                {
                    if (ble_dir_set_cnt == 0)
                    {
                        /*60RPM时连续运动一个小时，运动距离为20km*/
                        /* distanceDiff = 60 * rpm * 1.2 * timeInMillsDiff / 3600 / 1000.0*/
                        dir_temp += cross_data->Step_Per_Minute *0.02f;
                    }
                    if (ble_set_cnt == 0)
                    {
                        /* 单位为0.1kcal,*10是为了放大原本1cal为0.1cal*/
                        cal_temp += (0.024f + 0.001f * cross_data->Instantaneous_Power) * 10;
                    }
                }
            }
            if (ble_dir_set_cnt > 0) // 倒数，是否恢复累计值计算的能力
            {
                ble_dir_set_cnt--;
            }
            if (ble_set_cnt > 0) // 倒数，是否恢复累计值计算的能力
            {
                ble_set_cnt--;
            }
            if (cal_unit == 0)
            {
                cross_data->Total_Energy = cal_temp / 10;
            }
            else
            {
                cross_data->Total_Energy = cal_temp;
            }
            cross_data->Total_Distance = dir_temp;
            CrossTrainerDataAccessNotify(cross_data);
            update_0rpm_timer(cross_data->Step_Per_Minute / 2);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
    free(cross_data);
}

static void sports_task(void *arg)
{
    bike_data.Flags = M1S_FLAGS;
    bike_data.Energy_Per_Hour = 0xFFFF; // 传输最大视为无效值
    bike_data.Energy_Per_Minute = 0xFF; // 传输最大视为无效值
    User_sports_IPC_t r_queue;
    while (1)
    {
        if (xQueueReceive(sports_queue, &r_queue, 0))
        {
            switch (r_queue.ssid)
            {
            case USER_SPORTS_IPC_DISTANCE:
            {
                dir_temp = r_queue.data;
                ble_set_cnt = 6; // 同步数据后6秒不再累计新的数据
            }
            break;
            case USER_SPORTS_IPC_SETTING_CAL:
            {
                cal_unit = r_queue.data;
                ESP_LOGI(TAG, "设置卡路里单位 = %u", cal_unit);
            }
            break;
            case USER_SPORTS_IPC_CAL:
            {
                if (cal_unit == 0)
                {
                    cal_temp = r_queue.data * 10;
                }
                else
                {
                    cal_temp = r_queue.data;
                }
                ble_set_cnt = 6; // 同步数据后6秒不再累计新的数据
                ESP_LOGI(TAG, "同步卡路里 = %u", (uint32_t)cal_temp * 100);
            }
            break;
            case USER_SPORTS_IPS_CLEAR_0RPM:
            {
                rpm_0_timer = 0;
            }
            break;
            default:
                break;
            }
        }
        else
        {
            bike_data.Resistance_Level = get_actual_res();
            bike_data.Instantaneous_Cadence = get_RPM() * 2; // 单位是0.5/min
            bike_data.Elapsed_Time = get_timer();
            /*时速，按60rpm 20km/h进行推算*/
            bike_data.Instantaneous_Speed = bike_data.Instantaneous_Cadence / 2 * 33.334f;
            /*因为RPM上面乘2了,所以这个地方需要将传入的RPM除2处理*/
            bike_data.Instantaneous_Power = CAL_power(bike_data.Resistance_Level,
                                                      bike_data.Instantaneous_Cadence / 2);
            if (sports_lock == false && ble_set_cnt == 0)
            {
                if (bike_data.Instantaneous_Cadence > 0)
                {
                    /*60RPM时连续运动一个小时，运动距离为20km*/
                    dir_temp += bike_data.Instantaneous_Cadence / 2 * 0.0925f;
                    /* 单位为0.1kcal,*10是为了放大原本1cal为0.1cal*/
                    cal_temp += (0.024f + 0.001f * bike_data.Instantaneous_Power) * 10;
                }
            }
            if (ble_set_cnt > 0) // 倒数，是否恢复累计值计算的能力
            {
                ble_set_cnt--;
            }
            if (cal_unit == 0)
            {
                bike_data.Total_Energy = cal_temp / 10;
            }
            else
            {
                bike_data.Total_Energy = cal_temp;
            }
            bike_data.Total_Distance = dir_temp;
            IndoorBikeDataAccessNotify(&bike_data);
            update_0rpm_timer(bike_data.Instantaneous_Cadence / 2);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
}

void sports_start(void)
{
    rpm_0_timer = 0;
    sports_lock = false;
}

void sports_pause(void)
{
    sports_lock = true;
    has_rpm_timer = 0;
}

void sports_resume(void)
{
    rpm_0_timer = 0;
    sports_lock = false;
}

void sports_stop(void)
{
    rpm_0_timer = 0;
    has_rpm_timer = 0;
    sports_lock = true;
    dir_temp = 0;
    cal_temp = 0;
    cal_unit = 0;                 // 运动结束后，卡路里单位还原为默认下发1cal
    bike_data.Total_Distance = 0; // 清除累计距离
    bike_data.Total_Energy = 0;   // 清除累计卡路里
}

uint32_t get_sports_dir(void)
{
    return dir_temp;
}

uint32_t get_sports_cal(void)
{
    return cal_temp;
}

static void user_sports_queue_init(void)
{
    sports_queue = xQueueCreate(10, sizeof(User_sports_IPC_t)); // comment if you don't want to use capture module
}

void user_sports_send_queue(uint8_t ssid, uint32_t val)
{
    User_sports_IPC_t sports_send_temp;
    sports_send_temp.ssid = ssid;
    sports_send_temp.data = val;
    xQueueSend(sports_queue, &sports_send_temp, 100 / portTICK_PERIOD_MS);
}

int sports_init(void)
{
    int ret = 0;
    CAL_power_init();
    user_sports_queue_init();
    switch (get_model())
    {
    case DEV_MODEL_E80EV:
        ret = xTaskCreate(E80_sports_task, "sports_task", 1024 * 2, NULL, 7, NULL);
        break;
    case DEV_MODEL_EF25EV:
        ret = xTaskCreate(E80_sports_task, "sports_task", 1024 * 2, NULL, 7, NULL);
        break;
    
    default:
        ret = xTaskCreate(sports_task, "sports_task", 1024 * 2, NULL, 7, NULL);
        break;
    }
    return ret;
}
