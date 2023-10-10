#include "User_BLE_FTMS.h"
#include "esp_log.h"
#include "esp_gatts_api.h"

#define TAG "User_BLE_FTMS"

#define GATTS_DEMO_CHAR_VAL_LEN_MAX 500
#define CHAR_DECLARATION_SIZE       (sizeof(uint8_t))

enum SPORTS_UUID
{
  SPORTS_UUID_BIKE = 0x2AD2,         // 单车UUID
  SPORTS_UUID_TREADMILL = 0x2ACD,    // 跑步机UUID
  SPORTS_UUID_ROWER = 0x2AD1,        // 划船机UUID
  SPORTS_UUID_CROSSTRAINER = 0x2ACE, // 椭圆机UUID
};

/* Service */
static const uint16_t SERVICE_FITNESS_MACHINE_UUID              = 0x1826;
static const uint16_t GATT_UUID_FITNESS_MACHINE_FEATURE         = 0x2ACC;
static const uint8_t  GATT_UUID_FITNESS_EXTENSION_DATA[16]      = {0x59,0x14,0xfb,0x69,0x92,0x52,0x55,0xa3,0xe8,0x11,0x4c,0xc4,0x10,0x2c,0x8d,0xd1};
static const uint16_t GATT_UUID_TRAINING_STATUS                 = 0x2AD3;
static const uint16_t GATT_UUID_SUPPORT_SPEED_RANGE             = 0x2AD4;
static const uint16_t GATT_UUID_SUPPORT_RESISTANCE_LEVEL_RANGE  = 0x2AD6;
static const uint16_t GATT_UUID_SUPPORT_POWER_RANGE             = 0x2AD8;
static const uint16_t GATT_UUID_FITNESS_MACHINE_CONTROL_POINT   = 0x2AD9;
static const uint16_t GATT_UUID_FITNESS_MACHINE_STATUS          = 0x2ADA;
static const uint16_t primary_service_uuid         = ESP_GATT_UUID_PRI_SERVICE;
static const uint16_t character_declaration_uuid   = ESP_GATT_UUID_CHAR_DECLARE;
static const uint16_t character_client_config_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
static const uint8_t char_prop_read                = ESP_GATT_CHAR_PROP_BIT_READ;
static const uint8_t char_prop_read_notify         = ESP_GATT_CHAR_PROP_BIT_READ|ESP_GATT_CHAR_PROP_BIT_NOTIFY;
static const uint8_t char_prop_write               = ESP_GATT_CHAR_PROP_BIT_WRITE;
static const uint8_t char_prop_notify              = ESP_GATT_CHAR_PROP_BIT_NOTIFY;
static const uint8_t char_prop_write_indicate      = ESP_GATT_CHAR_PROP_BIT_WRITE|ESP_GATT_CHAR_PROP_BIT_INDICATE;
static const uint8_t FTMS_measurement_ccc[2]        = {0x00, 0x00};
static const uint8_t char_value[4]                   = {0x11, 0x22, 0x33, 0x44};
static const uint8_t char_FITNESS_MACHINE_FEATURE[8] = {0xa4,0x52,0,0,04,0,0,0};
static const uint8_t char_SUPPORT_SPEED_RANGE[6]     = {0x62,00,0xc9,0x04,0x05,00};
static const uint8_t char_SUPPORT_RESISTANCE_LEVEL_RANGE[6] = {00,00,0xe8,0x03,0x0A,00};
static const uint8_t char_SUPPORT_POWER_RANGE[6]     = {00,00,0xd0,0x05,0x01,00};
static  uint8_t Training_Status_value[2]             = {0x01, 0x01};
static uint16_t sports_type = SPORTS_UUID_BIKE; // 默认为单车类型

/* Full Database Description - Used to add attributes into the database */
esp_gatts_attr_db_t gatt_db_ftms[HRS_IDX_NB] =
{
    // Service Declaration
    [IDX_SVC]        =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid, ESP_GATT_PERM_READ,
      sizeof(uint16_t), sizeof(SERVICE_FITNESS_MACHINE_UUID), (uint8_t *)&SERVICE_FITNESS_MACHINE_UUID}},

    /* Characteristic Declaration */
    [IDX_CHAR_GATT_UUID_FITNESS_MACHINE_FEATURE]     =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read}},

    /* Characteristic Value */
    [IDX_CHAR_GATT_UUID_FITNESS_MACHINE_FEATURE_VAL] =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATT_UUID_FITNESS_MACHINE_FEATURE, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(char_FITNESS_MACHINE_FEATURE), (uint8_t *)char_FITNESS_MACHINE_FEATURE}},

    /* 运动数据特征值表 */
    [IDX_CHAR_GATT_UUID_SPORTS_DATA]     =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_notify}},

    /* Characteristic Value */
    [IDX_CHAR_GATT_UUID_SPORTS_DATA_VAL] =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&sports_type, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(char_value), (uint8_t *)char_value}},

    /* Client Characteristic Configuration Descriptor */
    [IDX_CHAR_GATT_UUID_SPORTS_DATA_CFG]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      sizeof(uint16_t), sizeof(FTMS_measurement_ccc), (uint8_t *)FTMS_measurement_ccc}},

    /* Characteristic Declaration */
    [IDX_CHAR_GATT_UUID_TRAINING_STATUS]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_notify}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_GATT_UUID_TRAINING_STATUS]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATT_UUID_TRAINING_STATUS, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(Training_Status_value), (uint8_t *)Training_Status_value}},

    /* Client Characteristic Configuration Descriptor */
    [IDX_CHAR_CFG_GATT_UUID_TRAINING_STATUS]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      sizeof(uint16_t), sizeof(FTMS_measurement_ccc), (uint8_t *)FTMS_measurement_ccc}},
  

    /* Characteristic Declaration */
    [IDX_CHAR_GATT_UUID_SUPPORT_SPEED_RANGE]     =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_GATT_UUID_SUPPORT_SPEED_RANGE] =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATT_UUID_SUPPORT_SPEED_RANGE, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(char_SUPPORT_SPEED_RANGE), (uint8_t *)char_SUPPORT_SPEED_RANGE}},

    /* Characteristic Declaration */
    [IDX_CHAR_GATT_UUID_SUPPORT_RESISTANCE_LEVEL_RANGE]     =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_GATT_UUID_SUPPORT_RESISTANCE_LEVEL_RANGE] =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATT_UUID_SUPPORT_RESISTANCE_LEVEL_RANGE, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(char_SUPPORT_RESISTANCE_LEVEL_RANGE), (uint8_t *)char_SUPPORT_RESISTANCE_LEVEL_RANGE}},

    /* Characteristic Declaration */
    [IDX_CHAR_GATT_UUID_SUPPORT_POWER_RANGE]     =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_GATT_UUID_SUPPORT_POWER_RANGE] =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATT_UUID_SUPPORT_POWER_RANGE, ESP_GATT_PERM_READ,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(char_SUPPORT_POWER_RANGE), (uint8_t *)char_SUPPORT_POWER_RANGE}},


    /* Characteristic Declaration */
    [IDX_CHAR_GATT_UUID_FITNESS_MACHINE_CONTROL_POINT]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_write_indicate}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_GATT_UUID_FITNESS_MACHINE_CONTROL_POINT]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATT_UUID_FITNESS_MACHINE_CONTROL_POINT, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(char_value), (uint8_t *)char_value}},

    /* Client Characteristic Configuration Descriptor */
    [IDX_CHAR_CFG_GATT_UUID_FITNESS_MACHINE_CONTROL_POINT]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      sizeof(uint16_t), sizeof(FTMS_measurement_ccc), (uint8_t *)FTMS_measurement_ccc}},

    /* Characteristic Declaration */
    [IDX_CHAR_GATT_UUID_FITNESS_MACHINE_STATUS]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_notify}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_GATT_UUID_FITNESS_MACHINE_STATUS]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATT_UUID_FITNESS_MACHINE_STATUS, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(char_value), (uint8_t *)char_value}},

          /* Client Characteristic Configuration Descriptor */
    [IDX_CHAR_CFG_GATT_UUID_FITNESS_MACHINE_STATUS]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      sizeof(uint16_t), sizeof(FTMS_measurement_ccc), (uint8_t *)FTMS_measurement_ccc}},

    /* Characteristic Declaration */
    [IDX_CHAR_GATT_UUID_FITNESS_EXTENSION_DATA]     =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_write}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_GATT_UUID_FITNESS_EXTENSION_DATA] =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_128, (uint8_t *)&GATT_UUID_FITNESS_EXTENSION_DATA, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(char_FITNESS_MACHINE_FEATURE), (uint8_t *)char_FITNESS_MACHINE_FEATURE}},
};

void set_ftms_sports_type(FTMS_SPORTS_TYPE_t type)
{
  switch (type)
  {
  case SPORTS_TYPE_BIKE:
    sports_type = SPORTS_UUID_BIKE;
    break;
  case SPORTS_TYPE_CROSSTRAINER:
    sports_type = SPORTS_UUID_CROSSTRAINER;
    break;
  case SPORTS_TYPE_ROWER:
    sports_type = SPORTS_UUID_ROWER;
    break;
  case SPORTS_TYPE_TREADMILL:
    sports_type = SPORTS_UUID_TREADMILL;
    break;

  default:
    sports_type = SPORTS_UUID_BIKE;
    break;
  }
}