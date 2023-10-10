#include "User_BLE_alternator.h"
#include "esp_log.h"

static const uint16_t primary_service_uuid = ESP_GATT_UUID_PRI_SERVICE;
static const uint16_t character_declaration_uuid = ESP_GATT_UUID_CHAR_DECLARE;
static const uint16_t character_client_config_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
static const uint8_t char_prop_write                = ESP_GATT_CHAR_PROP_BIT_WRITE;
static const uint8_t char_prop_read_write_notify = ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;

static const uint16_t GATT_UUID_EEE0 = BLE_EEE0_SERVER_UUID;
static const uint16_t GATT_UUID_EEE1 = 0xEEE1;
static const uint16_t GATT_UUID_EEE2 = 0xEEE2;
static const uint16_t GATT_UUID_EEE3 = 0xEEE3;
static const uint16_t GATT_UUID_EEE4 = 0xEEE4;
#define GATTS_DEMO_CHAR_VAL_LEN_MAX 500
#define CHAR_DECLARATION_SIZE (sizeof(uint8_t))
static const uint8_t char_prop_read = ESP_GATT_CHAR_PROP_BIT_READ;

static const uint8_t char_value[4]                   = {0x11, 0x22, 0x33, 0x44};
static const uint8_t heart_measurement_ccc[2]        = {0x00, 0x00};
static const uint8_t alternator_power_val_default[4] = {0x00,0x00,0x23,0x51};

#define TAG "BLE_EEE0"


const esp_gatts_attr_db_t gatt_db_EEE0[IDX_EEE0_MAX] =
{
    // Service Declaration
    [IDX_SVC_EEE0]        =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid, ESP_GATT_PERM_READ,
      sizeof(uint16_t), sizeof(GATT_UUID_EEE0), (uint8_t *)&GATT_UUID_EEE0}},
    /* Characteristic Declaration */
    [IDX_CHAR_EEE1]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read}},

    /* Characteristic Value */
    [IDX_CHAR_EEE1_VAL]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATT_UUID_EEE1, ESP_GATT_PERM_READ,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(alternator_power_val_default), (uint8_t *)alternator_power_val_default}},
    /* Characteristic Declaration */
    [IDX_CHAR_EEE2]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write_notify}},

    /* Characteristic Value */
    [IDX_CHAR_EEE2_VAL]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATT_UUID_EEE2, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX,  sizeof(char_value), (uint8_t *)char_value}},

    /* Characteristic Value */
    [IDX_CHAR_EEE2_CFG]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      sizeof(uint16_t),sizeof(heart_measurement_ccc), (uint8_t *)heart_measurement_ccc}},

    [IDX_CHAR_EEE3]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_write}},

    /* Characteristic Value */
    [IDX_CHAR_EEE3_VAL]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATT_UUID_EEE3, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(alternator_power_val_default), (uint8_t *)alternator_power_val_default}},

        /* Characteristic Declaration */
    [IDX_CHAR_EEE4]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write_notify}},

    /* Characteristic Value */
    [IDX_CHAR_EEE4_VAL]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATT_UUID_EEE4, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX,  sizeof(char_value), (uint8_t *)char_value}},

    /* Characteristic Value */
    [IDX_CHAR_EEE4_CFG]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      sizeof(uint16_t),sizeof(heart_measurement_ccc), (uint8_t *)heart_measurement_ccc}},
};



void BLE_EEE4_write(uint8_t* buf,uint16_t len)
{
  ESP_LOGI(TAG,"len=%u",len);
}

