#ifndef _USER_BLE_ALTERNATOR_H_
#define _USER_BLE_ALTERNATOR_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "esp_gatt_defs.h"
/* Attributes State Machine */
enum
{
    IDX_SVC_EEE0,
    IDX_CHAR_EEE1,
    IDX_CHAR_EEE1_VAL,
    IDX_CHAR_EEE2,
    IDX_CHAR_EEE2_VAL,
    IDX_CHAR_EEE2_CFG,
    IDX_CHAR_EEE3,
    IDX_CHAR_EEE3_VAL,
    IDX_CHAR_EEE4,
    IDX_CHAR_EEE4_VAL,
    IDX_CHAR_EEE4_CFG,
    IDX_EEE0_MAX
};

#define BLE_EEE0_SERVER_UUID (0XEEE0)

uint16_t EEE0_handle_table[IDX_EEE0_MAX];

/*自定义发电机服务*/
const esp_gatts_attr_db_t gatt_db_EEE0[IDX_EEE0_MAX];

void BLE_EEE4_write(uint8_t* buf,uint16_t len);




#endif
