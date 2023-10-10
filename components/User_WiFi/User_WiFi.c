#include "User_WiFi.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_https_ota.h"
#include "esp_fault.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "User_http_ota.h"
static const char *TAG = "wifi";

extern void update_wifi_state(const uint8_t* buf,uint8_t buf_size);

static wifi_config_t wifi_config = {
    .sta ={
        .ssid  = "123",
        .password = "123456789",
    },
};

static uint8_t url[260] = "http://esp32ts.oss-cn-shenzhen.aliyuncs.com/ESP32_EncoderLCD.bin";

static const uint8_t connection_fail[] = "connection fail";
static const uint8_t connected_successfully[] = "connected_successfully";
static const uint8_t OTA_START[] = "OTA_START";
static const uint8_t OTA_OK[] = "OTA_OK";
static const uint8_t OTA_FAIL[] = "OTA_FAIL_0000";
static const uint8_t OTA_FAIL_URL_OVER[] = "OTA_FAIL_0001";
static const uint8_t OTA_FAIL_SSID_OVER[] = "OTA_FAIL_0002";
static const uint8_t OTA_FAIL_PASS_OVER[] = "OTA_FAIL_0003";
static const uint8_t OTA_FAIL_URL_ERR[] = "OTA_FAIL_0004";
static const uint8_t OTA_FAIL_URL_RE_ERR[] = "OTA_FAIL_0005";

/*外部的HTTPS证书*/
extern const uint8_t server_cert_pem_start[] asm("_binary_ca_cert_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_ca_cert_pem_end");


void ota_send_OTA_FAIL_URL_ERR(void)
{
    ESP_LOGE(TAG,"OTA URL_ERR");
    update_wifi_state((const uint8_t*)OTA_FAIL_URL_ERR,sizeof(OTA_FAIL_URL_ERR));//发送ota成功的通知
}

void ota_send_OTA_FAIL_URL_RE_ERR(void)
{
    ESP_LOGE(TAG,"OTA URL_ERR");
    update_wifi_state((const uint8_t*)OTA_FAIL_URL_RE_ERR,sizeof(OTA_FAIL_URL_RE_ERR));//发送ota成功的通知
}

void set_wifi_cfg(uint8_t *buf,uint16_t len)
{
    if((buf[0]=='u')&&(buf[1]=='r')&&(buf[2]=='l')){
        if(len>260){
            ESP_LOGE(TAG,"URL_LEN_ERR=%u",len);
            update_wifi_state((const uint8_t*)OTA_FAIL_URL_OVER,sizeof(OTA_FAIL_URL_OVER));//发送ota成功的通知
            return;//限制写入长度保护，防止内存越界
        }
        memset(url,0,sizeof(url));
        memcpy(url,buf+3,len-3);
        ESP_LOGI(TAG,"len =%u url %s\r\n",len,url);
    }else if(buf[0]=='S'){
        if(len>33){
            ESP_LOGE(TAG,"OTA_FAIL_SSID_OVER=%u",len);
            update_wifi_state((const uint8_t*)OTA_FAIL_SSID_OVER,sizeof(OTA_FAIL_SSID_OVER));//发送ota成功的通知
            return;//限制写入长度保护，防止内存越界
        }
        memset(wifi_config.sta.ssid,0,sizeof(wifi_config.sta.ssid));
        memcpy(wifi_config.sta.ssid,buf+1,len-1);
        ESP_LOGI(TAG,"ssid %s\r\n",wifi_config.sta.ssid);
    }else if(buf[0]=='P'){
        if(len>65){
            ESP_LOGE(TAG,"OTA_FAIL_PASS_OVER=%u",len);
            update_wifi_state((const uint8_t*)OTA_FAIL_PASS_OVER,sizeof(OTA_FAIL_PASS_OVER));//发送ota成功的通知
            return;//限制写入长度保护，防止内存越界
        }
        memset(wifi_config.sta.password,0,sizeof(wifi_config.sta.password));
        memcpy(wifi_config.sta.password,buf+1,len-1);
        ESP_LOGI(TAG,"PASS %s\r\n",wifi_config.sta.password);
    }else if(buf[0]=='C'){
        xEventGroupSetBits(WIFI_Event_Handle,WIFI_START_EVENT);/* 发送WiFi开始事件，申请开始OTA */
    }else{
        ESP_LOGE(TAG,"无法识别的命令:%s",buf);
    }
}


/* The examples use WiFi configuration that you can set via project configuration menu

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_MAXIMUM_RETRY  1

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1


static int s_retry_num = 0;

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        update_wifi_state(connected_successfully,sizeof(connected_successfully));//发送WiFi连接成功的通知
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 wifi_config.sta.ssid, wifi_config.sta.password);
        xEventGroupSetBits(WIFI_Event_Handle,WIFI_OTA_EVENT);/* 连接成功后发送ota开始事件 */
    } else if (bits & WIFI_FAIL_BIT) {
        update_wifi_state(connection_fail,sizeof(connection_fail));//发送WiFi连接失败的通知
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 wifi_config.sta.ssid, wifi_config.sta.password);
        esp_wifi_stop();
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_wifi_event_group);
}

void WiFi_Init(void)
{
    WIFI_Event_Handle = xEventGroupCreate();
    if(WIFI_Event_Handle!=NULL){
        ESP_LOGI(TAG,"WIFI_Event_Handle Create succeeed\r\n");
    }else{
        ESP_LOGE(TAG,"WIFI_Event_Handle Create FAIL\r\n");
    }
    ESP_ERROR_CHECK(esp_netif_init());//创建一个LWIP核心任务
    ESP_ERROR_CHECK(esp_event_loop_create_default());//创建一个系统事件任务
    esp_netif_create_default_wifi_sta();//创建有 TCP/IP 堆栈的默认网络接口实例绑定 station
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));//Wi-Fi 驱动程序任务，并初始化 Wi-Fi 驱动程序
    // wifi_config.sta.ssid
}

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
        ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
        break;
    }
    return ESP_OK;
}

void OTA_TIMER_Task(void *parameter);
bool OTA_FG = false;
extern void rpm_set_close(void);

void user_ota_run(void)
{
    ESP_LOGI(TAG, "Starting OTA");
    update_wifi_state(OTA_START,sizeof(OTA_START));//发送OTA开始的通知
    xEventGroupSetBits(WIFI_Event_Handle, OTA_START_EVENT);
    rpm_set_close();
    esp_http_client_config_t config = {
        .cert_pem = (char *)server_cert_pem_start,
        .event_handler = _http_event_handler,
    };
    config.url = (const char *)url;
    esp_err_t ret = user_esp_https_ota(&config);
    if (ret == ESP_OK) {
        OTA_FG = false;
        update_wifi_state(OTA_OK,sizeof(OTA_OK));//发送ota成功的通知
        vTaskDelay(((500)/ portTICK_PERIOD_MS));
        _ESP_FAULT_RESET();//需要完全复位
    } else {
        update_wifi_state(OTA_FAIL,sizeof(OTA_FAIL));//发送WiFi连接失败的通知
        xEventGroupSetBits(WIFI_Event_Handle, OTA_FAIL_EVENT);
        ESP_LOGE(TAG, "Firmware upgrade failed");
        /*一分钟后如果设备没有重启，这里做一个强制重启*/
        vTaskDelay(((60000)/ portTICK_PERIOD_MS));
        _ESP_FAULT_RESET();//需要完全复位
    }
}

int is_wifi_ota_start(void)
{
    if (WIFI_Event_Handle != NULL)
    {
        EventBits_t r_event = xEventGroupWaitBits(WIFI_Event_Handle, OTA_START_EVENT, pdTRUE, pdFALSE, 0);
        if (r_event & OTA_START_EVENT)
        {
            return 1;
        }
    }
    return 0;
}

int is_wifi_ota_fail_ev(void)
{
    if (WIFI_Event_Handle != NULL)
    {
        EventBits_t r_event = xEventGroupWaitBits(WIFI_Event_Handle, OTA_FAIL_EVENT, pdTRUE, pdFALSE, 0);
        ESP_LOGI(TAG,"err = %x", r_event);
        if (r_event & OTA_FAIL_EVENT)
        {
            return 1;
        }
    }
    return 0;
}

void OTA_Task(void *parameter)
{
    while (1)
    {
        EventBits_t r_event = xEventGroupWaitBits(WIFI_Event_Handle,WIFI_START_EVENT|WIFI_OTA_EVENT,pdTRUE,pdFALSE,portMAX_DELAY);//portMAX_DELAY
        if(r_event&WIFI_START_EVENT){
            wifi_init_sta();
        }else if(r_event&WIFI_OTA_EVENT){
            OTA_FG  = true;
            user_ota_run();
        }
        vTaskDelay(((1000)/ portTICK_PERIOD_MS));
    }
}

void OTA_TIMER_Task(void *parameter)
{
    uint32_t timer = 0;
    while (1)
    {
        if(OTA_FG){
            timer ++;
            // ESP_LOGI("OTA_TIMER_Task","%u",timer);
            if(timer==120){
            update_wifi_state(OTA_FAIL,sizeof(OTA_FAIL));//发送WiFi连接失败的通知
            ESP_LOGE(TAG, "Firmware upgrade failed");
            vTaskDelay(((500)/ portTICK_PERIOD_MS));
            _ESP_FAULT_RESET();//需要完全复位
            }
        }
        
        vTaskDelay(((1000)/ portTICK_PERIOD_MS));
    }
}


void OTA_init(void)
{
    WiFi_Init();
    xTaskCreatePinnedToCore(OTA_TIMER_Task,"OTA_TIMER_Task",2048,NULL,1,NULL,PRO_CPU_NUM);  
    xTaskCreatePinnedToCore(OTA_Task,"OTA_Task",8192,NULL,1,NULL,PRO_CPU_NUM);  
}

