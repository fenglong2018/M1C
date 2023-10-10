#include "User_BLE_battery.h"
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_log.h"

static const uint16_t primary_service_uuid = ESP_GATT_UUID_PRI_SERVICE;
static const uint16_t character_declaration_uuid = ESP_GATT_UUID_CHAR_DECLARE;
static const uint16_t character_client_config_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
static const uint8_t char_prop_read_notify = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;

static const uint16_t GATT_UUID_BATTERY_SVC = BLE_BATTERY_SERVER_UUID;
static const uint16_t GATT_UUID_BATTERY_CHAR = 0x2A19;
#define GATTS_DEMO_CHAR_VAL_LEN_MAX 500
#define CHAR_DECLARATION_SIZE (sizeof(uint8_t))

static uint8_t char_value[1] = {0x64};
static const uint8_t heart_measurement_ccc[2] = {0x00, 0x00};

#define TAG "BLE_BATTERY"

const esp_gatts_attr_db_t gatt_db_BATTERY[IDX_BATTERY_MAX] =
    {
        // Service Declaration
        [IDX_SVC_BATTERY] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid, ESP_GATT_PERM_READ, sizeof(uint16_t),
             sizeof(GATT_UUID_BATTERY_SVC), (uint8_t *)&GATT_UUID_BATTERY_SVC}},
        /* Characteristic Declaration */
        [IDX_CHAR_BATTERY] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE,
             (uint8_t *)&char_prop_read_notify}},

        /* Characteristic Value */
        [IDX_CHAR_BATTERY_VAL] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATT_UUID_BATTERY_CHAR, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, GATTS_DEMO_CHAR_VAL_LEN_MAX,
             sizeof(char_value), (uint8_t *)char_value}},

        /* Characteristic Value */
        [IDX_CHAR_BATTERY_CFG] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
             sizeof(uint16_t), sizeof(heart_measurement_ccc), (uint8_t *)heart_measurement_ccc}},
};


// void ble_send_battery(uint8_t val)
// {
//     esp_ble_gatts_set_attr_value(BATTERY_handle_table[IDX_CHAR_BATTERY_VAL], 1, &val);
//     esp_ble_gatts_send_indicate(ble_profile_tab[0].gatts_if, ble_profile_tab[0].conn_id,
//                                 BATTERY_handle_table[IDX_CHAR_BATTERY_VAL],
//                                 1, &val, false);
// }