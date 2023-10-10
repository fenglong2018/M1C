#include "User_BLE_IPC.h"
#include "esp_log.h"

xQueueHandle BLE_queue = NULL;

void user_BLE_queue_init(void)
{
    BLE_queue = xQueueCreate(10, sizeof(User_BLE_IPC_t)); //comment if you don't want to use capture module
}

void user_ipc_send_queue(uint32_t ssid,uint8_t *val_p,uint8_t len)
{
    User_BLE_IPC_t ble_send_temp ; 
    ble_send_temp.ssid = ssid;
    memcpy(ble_send_temp.buf, val_p, len);
    ble_send_temp.len = len;
    xQueueSend(BLE_queue, &ble_send_temp, 100 / portTICK_PERIOD_MS);
}

int get_mi_task_queue(User_BLE_IPC_t *p)
{
    if(BLE_queue==NULL)return false;
    return xQueueReceive(BLE_queue, p, 0);
}