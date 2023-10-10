#ifndef _USER_WIFI_H_
#define _USER_WIFI_H_

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

/*WIFI事件组*/
#define WIFI_START_EVENT    (0X01 << 0)
#define WIFI_OTA_EVENT      (0X01 << 1)
#define OTA_START_EVENT     (0X01 << 2)
#define OTA_FAIL_EVENT     (0X01 << 3)

EventGroupHandle_t WIFI_Event_Handle;


void set_wifi_cfg(uint8_t *buf,uint16_t len);

void WiFi_Init(void);

void OTA_Task(void *parameter);
/*创建OTA任务，同时对WiFi进行初始化*/
void OTA_init(void);

int is_wifi_ota_start(void);
/*OTA失败事件*/
int is_wifi_ota_fail_ev(void);

#endif
