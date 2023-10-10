#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "User_LED.h"
#include "User_KEY.h"
#include "User_BLE.h"
#include "../components/User_encoder/User_encoder.h"
#include "esp_sleep.h"
#include "User_Protocol.h"
#include "User_pull_motor.h"
#include "esp_fault.h"
#include "User_NVS.h"
#include "esp_gap_ble_api.h"
#include "User_pull_motor_dev.h"
#include "TM1652_show_task.h"
#include "User_sports.h"
#include "User_dev_inc.h"
#include "User_mid_inc.h"
#include <esp_log.h>

#define TAG "main"

void app_main()
{
    vTaskDelay(250 / portTICK_PERIOD_MS);
    user_NVS_init(); // 配置出厂数据
    user_mid_init();
    user_dev_pwm_init();
    KEY_Init();
    RGB_init(); // 对RGB进行初始化，并启动RGB线程
    encode_init();
    ble_init();
    TM1652_show_task_init();

    if (is_EV())
    {
        /*串口2初始化与解码任务是配套的*/
        uart2_init();
        User_pull_motor();
        sports_init();
    }
    else
    {
        adc_init();
        user_motor_init();
        sports_init();
    }
    OTA_init();
    // vTaskDelay(10000 / portTICK_PERIOD_MS);
    // ESP_LOGI("CPU_INFO", "Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());
}
