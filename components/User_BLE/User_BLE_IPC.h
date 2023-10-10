#ifndef _USER_BLE_IPC_H_
#define _USER_BLE_IPC_H_

#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include <string.h>

typedef struct User_BLE_IPC
{
    uint8_t buf[30];
    uint8_t ssid;
    uint8_t len;
} User_BLE_IPC_t;

enum User_BLE_IPC_SIID_LIST
{
    BLE_IPC_SIID_INDOOR_BIKE,
    BLE_IPC_SIID_CHECK_INFO,
};

xQueueHandle BLE_queue;

void user_BLE_queue_init(void);
void user_ipc_send_queue(uint32_t ssid, uint8_t *val_p, uint8_t len);
int get_mi_task_queue(User_BLE_IPC_t *p);

#endif
