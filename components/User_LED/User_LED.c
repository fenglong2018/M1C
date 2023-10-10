#include "User_LED.h"
#include "driver/ledc.h"
#include "User_BLE.h"
#include "esp_log.h"
#include "hsv2rgb.h"
#include "User_dev_inc.h"
#include "freertos/queue.h"


typedef struct User_RGB_STU
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} User_RGB_IPC_t;

static TaskHandle_t LED_Task_Handle = NULL;     //声明一个任务句柄指针用于存储创建的led灯任务
static xQueueHandle rgb_queue = NULL;

void inline set_RGB(uint8_t R,uint8_t G,uint8_t B)
{
    user_dev_set_pwm_0(R);
    user_dev_set_pwm_1(G);
    user_dev_set_pwm_2(B);
}

/*开机迎宾灯，炫彩呼吸灯效果*/
static void startup_led(void)
{
    uint32_t r = 0;
    uint32_t g = 0;
    uint32_t b = 0;
    uint32_t h = 0;
    uint32_t s = 100;
    for(int i=0;i<360;i++){
        hsv2rgb(h++,100,100,&r,&b,&g);
        set_RGB(r,g,b);vTaskDelay(6 / portTICK_PERIOD_MS);
    }
    for(int i=0;i<100;i++){
        hsv2rgb(h++,s--,100,&r,&b,&g);
        set_RGB(r,g,b);vTaskDelay(7 / portTICK_PERIOD_MS);
    }
}

static void user_led_queue_init(void)
{
    rgb_queue = xQueueCreate(10, sizeof(User_RGB_IPC_t)); // comment if you don't want to use capture module
}

void user_rgb_send_queue(uint8_t r, uint8_t g, uint8_t b)
{
    if (rgb_queue != NULL)
    {
        User_RGB_IPC_t temp;
        temp.r = r;
        temp.g = g;
        temp.b = b;
        xQueueSend(rgb_queue, &temp, 100 / portTICK_PERIOD_MS);
    }
}

static void LED_Task(void *parameter)
{
    User_RGB_IPC_t r_queue = {.r = 0xFF,.g = 0xFF,.b = 0xFF};
    user_led_queue_init();
    startup_led(); // 开机迎宾灯
    while (1)
    {
        if (xQueueReceive(rgb_queue, &r_queue, 1000 / portTICK_PERIOD_MS))
        {
            set_RGB(r_queue.r, r_queue.g, r_queue.b);
        }
        else if (get_ble_connect() == 0)
        {
            set_RGB(0xFF, 0XFF, 0XFF);
        }
    }
}

void RGB_deinit(void)
{
    vTaskDelete(LED_Task_Handle);
    set_RGB(0, 0, 0);
}

void RGB_init(void)
{
    xTaskCreatePinnedToCore(LED_Task,"LED_Task",2048,NULL,1,&LED_Task_Handle,tskNO_AFFINITY);
}
