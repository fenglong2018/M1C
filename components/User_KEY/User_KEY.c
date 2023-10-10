#include "User_KEY.h"
#include "driver/touch_pad.h"
#include "esp_log.h"
#define TOUCH_THRESH_NO_USE   (0)
#define TOUCHPAD_FILTER_TOUCH_PERIOD (10)


EventGroupHandle_t KEY_Event_Handle = NULL;

void KEY_Init(void)
{
    gpio_pad_select_gpio(KEY1_IO);
    gpio_set_direction(KEY1_IO,GPIO_MODE_DEF_INPUT);
    gpio_pad_select_gpio(TOUCH_KEY1_IO);
    gpio_set_direction(TOUCH_KEY1_IO,GPIO_MODE_DEF_INPUT);
    key_ev_create();
    xTaskCreatePinnedToCore(KEY_Task,"KEY_Task",2048,NULL,3,NULL,tskNO_AFFINITY);
}

inline uint8_t get_key1(void)
{
    return !(gpio_get_level(KEY1_IO));
}

inline uint8_t get_touch_key1(void)
{
    return (gpio_get_level(TOUCH_KEY1_IO));
}

void key_ev_create(void)
{
    KEY_Event_Handle = xEventGroupCreate();
    if(KEY_Event_Handle!=NULL){
        ESP_LOGD("key","key_Event_Handle 创建成功");
    }else{
        ESP_LOGE("key","key_Event_Handle 创建失败");
    }
}

void IRAM_ATTR KEY_Task(void *parameter)
{
    uint32_t key_timer = 0;
    uint32_t touch_timer = 0;
    while (1)
    {
        if(get_key1()){  
            key_timer++;
            if(key_timer==150){
                xEventGroupSetBits(KEY_Event_Handle,KEY1_LONG_PRESS_EVENT|KEY1_CLEAN_SLEEP_EVENT);/* 触发一个长按事件 */
            }
        }else if(key_timer>0&&key_timer<25){           
            key_timer = 0;
            xEventGroupSetBits(KEY_Event_Handle,KEY1_SHORT_PRESS_EVENT|KEY1_CLEAN_SLEEP_EVENT);/* 触发一个短按事件 */
        }else{
            key_timer = 0;
        }
        if(get_touch_key1()){
            touch_timer++;
            if(touch_timer==250){
                xEventGroupSetBits(KEY_Event_Handle,KEY1_LONG_TOUCH_PRESS_EVENT|KEY1_CLEAN_SLEEP_EVENT);/* 触发一个长按事件 */
            }
        }else if(touch_timer>0&&touch_timer<25){
            xEventGroupSetBits(KEY_Event_Handle,KEY1_TOUCH_PRESS_EVENT|KEY1_CLEAN_SLEEP_EVENT);/* 触发一个触摸事件 */
            touch_timer = 0;
        }else{
            touch_timer = 0;
        }
        vTaskDelay(((20)/ portTICK_PERIOD_MS));
    }
}

