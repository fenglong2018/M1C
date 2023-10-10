#ifndef _USER_BLE_FFF0_H_
#define _USER_BLE_FFF0_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "esp_gatt_defs.h"


/* Attributes State Machine */
enum
{
    IDX_SVC_FFF0,
    IDX_CHAR_FFF2,
    IDX_CHAR_FFF2_VAL,
    IDX_CHAR_FFF6,
    IDX_CHAR_FFF6_VAL,
    IDX_CHAR_FFF6_CFG, 
    IDX_CHAR_FFF8,
    IDX_CHAR_FFF8_VAL,
    IDX_CHAR_FFF8_CFG, 
    IDX_CHAR_FFF9,
    IDX_CHAR_FFF9_VAL,
    IDX_CHAR_FFF9_CFG,
    IDX_FFF0_MAX
}; 

const esp_gatts_attr_db_t gatt_db_FFF0[IDX_FFF0_MAX];

uint16_t FFF0_handle_table[IDX_FFF0_MAX];

#endif
