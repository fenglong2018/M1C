#include "User_mid_beep.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "User_dev_inc.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "User_dev_inc.h"

#define TAG "user_mid_beep"
#define BEEP_SHORT (0X01<<0)
#define BEEP_LONG  (0X01<<1)

EventGroupHandle_t beep_Event_Handle = NULL;

void beep_ev_create(void)
{
    beep_Event_Handle = xEventGroupCreate();
    if(beep_Event_Handle!=NULL){
        ESP_LOGI(TAG,"beep_Event_Handle 创建成功");
    }else{
        ESP_LOGE(TAG,"beep_Event_Handle 创建失败");
    }
}

void user_mid_beep_short(void)
{
    xEventGroupSetBits(beep_Event_Handle, BEEP_SHORT);
}

void user_mid_beep_long(void)
{
    xEventGroupSetBits(beep_Event_Handle, BEEP_LONG);
}

static void beep_Task(void *parameter)
{
    beep_ev_create();
    while (1)
    {
        EventBits_t r_event =
            xEventGroupWaitBits(
                beep_Event_Handle, /* 事件对象句柄 */
                BEEP_SHORT | BEEP_LONG,
                pdTRUE,         /* 退出时清除事件位 */
                pdFALSE,        /* 满足感兴趣的所有事件 */
                portMAX_DELAY); /* 指定超时事件, 一直等 */
        if (r_event & BEEP_LONG)
        {
            ESP_LOGI(TAG, "BEEP_LONG");
        }
        else if (BEEP_SHORT & r_event)
        {
            ESP_LOGI(TAG, "BEEP_SHORT");
            user_dev_set_pwm_3(250);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            user_dev_set_pwm_3(0);
        }
    }
}

void user_mid_beep_init(void)
{
    xTaskCreatePinnedToCore(beep_Task,"beep_Task",2048,NULL,1,NULL,tskNO_AFFINITY);
}