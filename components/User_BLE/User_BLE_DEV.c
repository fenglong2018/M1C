#include "User_BLE_DEV.h"
#include "esp_log.h"

#define TAG "ble_dev"

#define GATTS_DEMO_CHAR_VAL_LEN_MAX 500
#define PREPARE_BUF_MAX_SIZE        1024
#define CHAR_DECLARATION_SIZE       (sizeof(uint8_t))



static const uint8_t char_SYSTEM_ID[6]               = {0x11,0x22,0x33,0x44,0x55,0x66};
static const uint8_t char_Model[]                    = "unknown";
static const uint8_t char_SERIAL_NUMBER_STRING[]     = "YESOUL_ESP32_TEST_SN";
static const uint8_t char_FIRMWARE_REVISION_STRING[] = "157";
static const uint8_t char_HARDWARE_REVISION_STRING[] = "V1.2_210616-ESP32";
static const uint8_t char_SOFTWARE_REVISION_STRING[] = "V1.5.7_230907";
static const uint8_t char_MANUFACTURER_NAME_STRING[] = "YESOUL";
static const uint16_t SERVICE_DEVICE_INFORMATION_UUID        = 0x180A;
static const uint16_t SYSTEM_ID                              = 0X2A23;
static const uint16_t MODEL_NUMBER_STRING                    = 0X2A24;
static const uint16_t SERIAL_NUMBER_STRING                   = 0X2A25;
static const uint16_t FIRMWARE_REVISION_STRING               = 0X2A26;
static const uint16_t HARDWARE_REVISION_STRING               = 0X2A27;
static const uint16_t SOFTWARE_REVISION_STRING               = 0X2A28;
static const uint16_t MANUFACTURER_NAME_STRING               = 0x2A29;
static const uint16_t primary_service_uuid         = ESP_GATT_UUID_PRI_SERVICE;
static const uint16_t character_declaration_uuid   = ESP_GATT_UUID_CHAR_DECLARE;
static const uint8_t char_prop_read                = ESP_GATT_CHAR_PROP_BIT_READ;

const esp_gatts_attr_db_t gatt_db_DIS[IDX_SVC_DIS_MAX] =
{
    // Service Declaration
    [IDX_SVC_DIS]        =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid, ESP_GATT_PERM_READ,
      sizeof(uint16_t), sizeof(SERVICE_DEVICE_INFORMATION_UUID), (uint8_t *)&SERVICE_DEVICE_INFORMATION_UUID}},

    /* Characteristic Declaration */
    [IDX_SVC_DIS_CHAR_SYSTEM_ID]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read}},

    /* Characteristic Value */
    [IDX_SVC_DIS_CHAR_SYSTEM_ID_VAL]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&SYSTEM_ID, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(char_SYSTEM_ID), (uint8_t *)char_SYSTEM_ID}},

    /* Characteristic Declaration */
    [IDX_SVC_DIS_CHAR_MODEL_NUMBER_STRING]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read}},

    /* Characteristic Value */
    [IDX_SVC_DIS_CHAR_MODEL_NUMBER_STRING_VAL]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&MODEL_NUMBER_STRING, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(char_Model), (uint8_t *)char_Model}},

    /* Characteristic Declaration */
    [IDX_SVC_DIS_CHAR_SERIAL_NUMBER_STRING]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read}},

    /* Characteristic Value */
    [IDX_SVC_DIS_CHAR_SERIAL_NUMBER_STRING_VAL]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&SERIAL_NUMBER_STRING, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(char_SERIAL_NUMBER_STRING), (uint8_t *)char_SERIAL_NUMBER_STRING}},

    /* Characteristic Declaration */
    [IDX_SVC_DIS_CHAR_FIRMWARE_REVISION_STRING]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read}},

    /* Characteristic Value */
    [IDX_SVC_DIS_CHAR_FIRMWARE_REVISION_STRING_VAL]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&FIRMWARE_REVISION_STRING, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(char_FIRMWARE_REVISION_STRING), (uint8_t *)char_FIRMWARE_REVISION_STRING}},

    /* Characteristic Declaration */
    [IDX_SVC_DIS_CHAR_HARDWARE_REVISION_STRING]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read}},

    /* Characteristic Value */
    [IDX_SVC_DIS_CHAR_HARDWARE_REVISION_STRING_VAL]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&HARDWARE_REVISION_STRING, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(char_HARDWARE_REVISION_STRING), (uint8_t *)char_HARDWARE_REVISION_STRING}},

    /* Characteristic Declaration */
    [IDX_SVC_DIS_CHAR_SOFTWARE_REVISION_STRING]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read}},

    /* Characteristic Value */
    [IDX_SVC_DIS_CHAR_SOFTWARE_REVISION_STRING_VAL]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&SOFTWARE_REVISION_STRING, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(char_SOFTWARE_REVISION_STRING), (uint8_t *)char_SOFTWARE_REVISION_STRING}},

    /* Characteristic Declaration */
    [IDX_SVC_DIS_CHAR_MANUFACTURER_NAME_STRING]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read}},

    /* Characteristic Value */
    [IDX_SVC_DIS_CHAR_MANUFACTURER_NAME_STRING_VAL]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&MANUFACTURER_NAME_STRING, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(char_MANUFACTURER_NAME_STRING), (uint8_t *)char_MANUFACTURER_NAME_STRING}},
};