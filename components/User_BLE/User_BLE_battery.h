#ifndef _USER_BLE_BATTERY_H_
#define _USER_BLE_BATTERY_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "esp_gatt_defs.h"
/* Attributes State Machine */
enum
{
    IDX_SVC_BATTERY,
    IDX_CHAR_BATTERY,
    IDX_CHAR_BATTERY_VAL,
    IDX_CHAR_BATTERY_CFG,
    IDX_BATTERY_MAX
};

#define BLE_BATTERY_SERVER_UUID (0X180F)

uint16_t BATTERY_handle_table[IDX_BATTERY_MAX];

/*自定义发电机服务*/
const esp_gatts_attr_db_t gatt_db_BATTERY[IDX_BATTERY_MAX];

void ble_send_battery(uint8_t val);




#endif
