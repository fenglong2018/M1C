#include "../User_Protocol/User_Protocol.h"

#include "User_BLE.h"
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "esp_system.h"
#define LOG_LOCAL_LEVEL ESP_LOG_WARN
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"
#include "esp_bt_device.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../User_KEY/User_KEY.h"
#include "nvs.h"
#include "User_NVS.h"

#include "User_BLE_FTMS.h"
#include "User_BLE_DEV.h"
#include "User_BLE_FFF0.h"
#include "User_BLE_IPC.h"
#include "User_BLE_battery.h"
#include "User_BLE_alternator.h"
#include "User_sports.h"
#include "User_LED.h"
#include "user_mid_inc.h"

#define GATTS_TABLE_TAG "BLE"

#define PROFILE_NUM                 1
#define PROFILE_APP_IDX             0
#define ESP_APP_ID                  0x55
#define SVC_INST_ID                 0

/* The max length of characteristic value. When the GATT client performs a write or prepare write operation,
*  the data length must be less than GATTS_DEMO_CHAR_VAL_LEN_MAX. 
*/
#define GATTS_DEMO_CHAR_VAL_LEN_MAX 500
#define PREPARE_BUF_MAX_SIZE        1024
#define CHAR_DECLARATION_SIZE       (sizeof(uint8_t))

#define ADV_CONFIG_FLAG             (1 << 0)
#define SCAN_RSP_CONFIG_FLAG        (1 << 1)


static uint8_t ble_connect = 0; 

static uint8_t adv_config_done       = 0;

uint16_t heart_rate_handle_table[HRS_IDX_NB];
uint16_t DIS_handle_table[IDX_SVC_DIS_MAX];
uint16_t FFF0_handle_table[IDX_FFF0_MAX];


typedef struct {
    uint8_t                 *prepare_buf;
    int                     prepare_len;
} prepare_type_env_t;

static prepare_type_env_t prepare_write_env;

void set_user_ble_name(void)
{
    char read_buff[100] = {0};
    esp_err_t err = read_user_ble_name(read_buff,sizeof(read_buff));
    if(err==ESP_OK){
        esp_err_t set_dev_name_ret = esp_ble_gap_set_device_name((const char *)read_buff);
        if (set_dev_name_ret){
            ESP_LOGE(GATTS_TABLE_TAG, "set device name failed, error code = %x", set_dev_name_ret);
        }
    }else{
        ESP_LOGE(GATTS_TABLE_TAG,"set_user_ble_name_err");
    }
}

void set_user_SN(void)
{
    char read_buff[100] = {0};
    esp_err_t err = read_user_SN(read_buff,sizeof(read_buff));
    if(err==ESP_OK){
        esp_ble_gatts_set_attr_value(DIS_handle_table[IDX_SVC_DIS_CHAR_SERIAL_NUMBER_STRING_VAL],strlen(read_buff),(uint8_t*)read_buff);
    }else{
        ESP_LOGE(GATTS_TABLE_TAG,"set_user_SN_err");
    }
}

void set_user_HARDWARE(void)
{
    char read_buff[100] = {0};
    esp_err_t err = read_user_ble_HARDWARE(read_buff,sizeof(read_buff));
    if(err==ESP_OK){
        esp_ble_gatts_set_attr_value(DIS_handle_table[IDX_SVC_DIS_CHAR_HARDWARE_REVISION_STRING_VAL],strlen(read_buff),(uint8_t*)read_buff);
    }else{
        ESP_LOGE(GATTS_TABLE_TAG,"set_user_HARDWARE_err");
    }
}

void set_user_mac(void)
{
    uint8_t mac[6];
    esp_read_mac(mac,ESP_MAC_BT);            
    esp_ble_gatts_set_attr_value(DIS_handle_table[IDX_SVC_DIS_CHAR_SYSTEM_ID_VAL],6,mac);//写入MAC地址到系统ID
}

void set_user_model(void)
{
    char read_buff[100] = {0};
    esp_err_t err = read_user_MODEL(read_buff,sizeof(read_buff));
    if(err==ESP_OK){
        esp_ble_gatts_set_attr_value(DIS_handle_table[IDX_SVC_DIS_CHAR_MODEL_NUMBER_STRING_VAL],strlen(read_buff),(uint8_t*)read_buff);
    }else{
        ESP_LOGE(GATTS_TABLE_TAG,"set_user_model err");
    }
}

// #define CONFIG_SET_RAW_ADV_DATA
#ifdef CONFIG_SET_RAW_ADV_DATA
static uint8_t raw_adv_data[31] = {
        /* flags */
        0x02, 0x01, 0x06,
        /* tx power*/
        0x02, 0x0a, 0xeb,
        /* service uuid */
        0x03, 0x03, 0xFF, 0x00,
        /* device name */
        0x0d, 0x09, 'Y', 'S', 'R', '4', '0', 'S', 'E', '9', '9', '9', '9','9',

};

static uint8_t raw_scan_rsp_data[] = {
        /* flags */
        0x02, 0x01, 0x06,
        /* service uuid */
        0x03, 0x02, 0x26,0x18,

        0x06,0x16,0x26,0x18,0x01,0x10,0x00,
        0x08,0xff,0x7d,0x02,0x01,0x05,0x00,0xff,0xff
};

#else

static uint8_t manufacturer[] = {0x7d,0x02,0x01,0x05,0x00,0xff,0xff};//华为的自定义厂家信息
static uint8_t service_uuid[16] = {
    /* LSB <--------------------------------------------------------------------------------> MSB */
    //first uuid, 16bit, [12],[13] is the value
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x26, 0x18, 0x00, 0x00,
};

static uint8_t HW_service_data[6] = {0x26,0x18,0x01,0x02,0x00};

/* The length of adv data must be less than 31 bytes */
static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp        = false,
    .include_name        = true,
    .include_txpower     = false,
    .min_interval        = 0x0006, //slave connection min interval, Time = min_interval * 1.25 msec
    .max_interval        = 0x0010, //slave connection max interval, Time = max_interval * 1.25 msec
    .appearance          = 0x00,
    .manufacturer_len    = sizeof(manufacturer),    //TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data = manufacturer, //test_manufacturer,
    .service_data_len    = 0,
    .p_service_data      = NULL,
    .service_uuid_len    = 0,
    .p_service_uuid      = NULL,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

// scan response data
static esp_ble_adv_data_t scan_rsp_data = {
    .set_scan_rsp        = true,
    .include_name        = false,
    .include_txpower     = true,
    .min_interval        = 0x0006,
    .max_interval        = 0x0010,
    .appearance          = 0x00,
    .manufacturer_len    = 0, //TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data = NULL, //&test_manufacturer[0],
    .service_data_len    = sizeof(HW_service_data),
    .p_service_data      = HW_service_data,
    .service_uuid_len    = sizeof(service_uuid),
    .p_service_uuid      = service_uuid,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};
#endif /* CONFIG_SET_RAW_ADV_DATA */

static esp_ble_adv_params_t adv_params = {
    .adv_int_min         = 0x20,
    .adv_int_max         = 0x40,
    .adv_type            = ADV_TYPE_IND,
    .own_addr_type       = BLE_ADDR_TYPE_PUBLIC,
    .channel_map         = ADV_CHNL_ALL,
    .adv_filter_policy   = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

struct gatts_profile_inst {
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_handle;
    esp_gatt_srvc_id_t service_id;
    uint16_t char_handle;
    esp_bt_uuid_t char_uuid;
    esp_gatt_perm_t perm;
    esp_gatt_char_prop_t property;
    uint16_t descr_handle;
    esp_bt_uuid_t descr_uuid;
};

static void gatts_profile_event_handler(esp_gatts_cb_event_t event,
					esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

/* One gatt-based profile one app_id and one gatts_if, this array will store the gatts_if returned by ESP_GATTS_REG_EVT */
static struct gatts_profile_inst heart_rate_profile_tab[PROFILE_NUM] = {
    [PROFILE_APP_IDX] = {
        .gatts_cb = gatts_profile_event_handler,
        .gatts_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
    },
};

static  uint8_t Training_Status_value[2]             = {0x01, 0x01};


static char *esp_key_type_to_str(esp_ble_key_type_t key_type)
{
   char *key_str = NULL;
   switch(key_type) {
    case ESP_LE_KEY_NONE:
        key_str = "ESP_LE_KEY_NONE";
        break;
    case ESP_LE_KEY_PENC:
        key_str = "ESP_LE_KEY_PENC";
        break;
    case ESP_LE_KEY_PID:
        key_str = "ESP_LE_KEY_PID";
        break;
    case ESP_LE_KEY_PCSRK:
        key_str = "ESP_LE_KEY_PCSRK";
        break;
    case ESP_LE_KEY_PLK:
        key_str = "ESP_LE_KEY_PLK";
        break;
    case ESP_LE_KEY_LLK:
        key_str = "ESP_LE_KEY_LLK";
        break;
    case ESP_LE_KEY_LENC:
        key_str = "ESP_LE_KEY_LENC";
        break;
    case ESP_LE_KEY_LID:
        key_str = "ESP_LE_KEY_LID";
        break;
    case ESP_LE_KEY_LCSRK:
        key_str = "ESP_LE_KEY_LCSRK";
        break;
    default:
        key_str = "INVALID BLE KEY TYPE";
        break;

   }

   return key_str;
}

static char *esp_auth_req_to_str(esp_ble_auth_req_t auth_req)
{
   char *auth_str = NULL;
   switch(auth_req) {
    case ESP_LE_AUTH_NO_BOND:
        auth_str = "ESP_LE_AUTH_NO_BOND";
        break;
    case ESP_LE_AUTH_BOND:
        auth_str = "ESP_LE_AUTH_BOND";
        break;
    case ESP_LE_AUTH_REQ_MITM:
        auth_str = "ESP_LE_AUTH_REQ_MITM";
        break;
    case ESP_LE_AUTH_REQ_BOND_MITM:
        auth_str = "ESP_LE_AUTH_REQ_BOND_MITM";
        break;
    case ESP_LE_AUTH_REQ_SC_ONLY:
        auth_str = "ESP_LE_AUTH_REQ_SC_ONLY";
        break;
    case ESP_LE_AUTH_REQ_SC_BOND:
        auth_str = "ESP_LE_AUTH_REQ_SC_BOND";
        break;
    case ESP_LE_AUTH_REQ_SC_MITM:
        auth_str = "ESP_LE_AUTH_REQ_SC_MITM";
        break;
    case ESP_LE_AUTH_REQ_SC_MITM_BOND:
        auth_str = "ESP_LE_AUTH_REQ_SC_MITM_BOND";
        break;
    default:
        auth_str = "INVALID BLE AUTH REQ";
        break;
   }

   return auth_str;
}

static void show_bonded_devices(void)
{
    int dev_num = esp_ble_get_bond_device_num();

    esp_ble_bond_dev_t *dev_list = (esp_ble_bond_dev_t *)malloc(sizeof(esp_ble_bond_dev_t) * dev_num);
    esp_ble_get_bond_device_list(&dev_num, dev_list);
    ESP_LOGI(GATTS_TABLE_TAG, "Bonded devices number : %d\n", dev_num);

    ESP_LOGI(GATTS_TABLE_TAG, "Bonded devices list : %d\n", dev_num);
    for (int i = 0; i < dev_num; i++) {
        esp_log_buffer_hex(GATTS_TABLE_TAG, (void *)dev_list[i].bd_addr, sizeof(esp_bd_addr_t));
    }

    free(dev_list);
}

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event) {
    #ifdef CONFIG_SET_RAW_ADV_DATA
        case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
            adv_config_done &= (~ADV_CONFIG_FLAG);
            if (adv_config_done == 0){
                esp_ble_gap_start_advertising(&adv_params);
            }
            break;
        case ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT:
            adv_config_done &= (~SCAN_RSP_CONFIG_FLAG);
            if (adv_config_done == 0){
               esp_ble_gap_start_advertising(&adv_params);
            }
            break;
    #else
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
            adv_config_done &= (~ADV_CONFIG_FLAG);
            if (adv_config_done == 0){
                esp_ble_gap_start_advertising(&adv_params);
            }
            break;
        case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
            adv_config_done &= (~SCAN_RSP_CONFIG_FLAG);
            if (adv_config_done == 0){
                esp_ble_gap_start_advertising(&adv_params);
            }
            break;
    #endif
        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
            /* advertising start complete event to indicate advertising start successfully or failed */
            if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
                ESP_LOGE(GATTS_TABLE_TAG, "advertising start failed");
            }else{
                ESP_LOGI(GATTS_TABLE_TAG, "advertising start successfully");
            }
            break;
        case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
            if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
                ESP_LOGE(GATTS_TABLE_TAG, "Advertising stop failed");
            }
            else {
                ESP_LOGI(GATTS_TABLE_TAG, "Stop adv successfully\n");
            }
            break;
        case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
            ESP_LOGI(GATTS_TABLE_TAG, "update connection params status = %d, min_int = %d, max_int = %d,conn_int = %d,latency = %d, timeout = %d",
                  param->update_conn_params.status,
                  param->update_conn_params.min_int,
                  param->update_conn_params.max_int,
                  param->update_conn_params.conn_int,
                  param->update_conn_params.latency,
                  param->update_conn_params.timeout);
            break;
        case ESP_GAP_BLE_KEY_EVT:
        //shows the ble key info share with peer device to the user.
        ESP_LOGI(GATTS_TABLE_TAG, "key type = %s", esp_key_type_to_str(param->ble_security.ble_key.key_type));
        
        break;
        case ESP_GAP_BLE_AUTH_CMPL_EVT: {
            
        esp_bd_addr_t bd_addr;
        memcpy(bd_addr, param->ble_security.auth_cmpl.bd_addr, sizeof(esp_bd_addr_t));
        ESP_LOGI(GATTS_TABLE_TAG, "remote BD_ADDR: %08x%04x",\
                (bd_addr[0] << 24) + (bd_addr[1] << 16) + (bd_addr[2] << 8) + bd_addr[3],
                (bd_addr[4] << 8) + bd_addr[5]);
        ESP_LOGI(GATTS_TABLE_TAG, "address type = %d", param->ble_security.auth_cmpl.addr_type);
        ESP_LOGI(GATTS_TABLE_TAG, "pair status = %s",param->ble_security.auth_cmpl.success ? "success" : "fail");
        if(!param->ble_security.auth_cmpl.success) {
            ESP_LOGI(GATTS_TABLE_TAG, "fail reason = 0x%x",param->ble_security.auth_cmpl.fail_reason);
        } else {
            ESP_LOGI(GATTS_TABLE_TAG, "auth mode = %s",esp_auth_req_to_str(param->ble_security.auth_cmpl.auth_mode));
        }
        show_bonded_devices();
        break;
    }

        case ESP_GAP_BLE_SEC_REQ_EVT:{
            esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, true);
            //esp_ble_set_encryption(param->connect.remote_bda, ESP_BLE_SEC_ENCRYPT_MITM);
        }break;
        default:ESP_LOGE(GATTS_TABLE_TAG, "GAPERR = %x",event);
            break;
    }
}

void example_prepare_write_event_env(esp_gatt_if_t gatts_if, prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param)
{
    ESP_LOGI(GATTS_TABLE_TAG, "prepare write, handle = %d, value len = %d", param->write.handle, param->write.len);
    esp_gatt_status_t status = ESP_GATT_OK;
    if (prepare_write_env->prepare_buf == NULL) {
        prepare_write_env->prepare_buf = (uint8_t *)malloc(PREPARE_BUF_MAX_SIZE * sizeof(uint8_t));
        prepare_write_env->prepare_len = 0;
        if (prepare_write_env->prepare_buf == NULL) {
            ESP_LOGE(GATTS_TABLE_TAG, "%s, Gatt_server prep no mem", __func__);
            status = ESP_GATT_NO_RESOURCES;
        }
    } else {
        if(param->write.offset > PREPARE_BUF_MAX_SIZE) {
            status = ESP_GATT_INVALID_OFFSET;
        } else if ((param->write.offset + param->write.len) > PREPARE_BUF_MAX_SIZE) {
            status = ESP_GATT_INVALID_ATTR_LEN;
        }
    }
    /*send response when param->write.need_rsp is true */
    if (param->write.need_rsp){
        esp_gatt_rsp_t *gatt_rsp = (esp_gatt_rsp_t *)malloc(sizeof(esp_gatt_rsp_t));
        if (gatt_rsp != NULL){
            gatt_rsp->attr_value.len = param->write.len;
            gatt_rsp->attr_value.handle = param->write.handle;
            gatt_rsp->attr_value.offset = param->write.offset;
            gatt_rsp->attr_value.auth_req = ESP_GATT_AUTH_REQ_NONE;
            memcpy(gatt_rsp->attr_value.value, param->write.value, param->write.len);
            esp_err_t response_err = esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, status, gatt_rsp);
            if (response_err != ESP_OK){
               ESP_LOGE(GATTS_TABLE_TAG, "Send response error");
            }
            free(gatt_rsp);
        }else{
            ESP_LOGE(GATTS_TABLE_TAG, "%s, malloc failed", __func__);
        }
    }
    if (status != ESP_GATT_OK){
        return;
    }
    memcpy(prepare_write_env->prepare_buf + param->write.offset,
           param->write.value,
           param->write.len);
    prepare_write_env->prepare_len += param->write.len;

}

void example_exec_write_event_env(prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param){
    if (param->exec_write.exec_write_flag == ESP_GATT_PREP_WRITE_EXEC && prepare_write_env->prepare_buf){
        esp_log_buffer_hex(GATTS_TABLE_TAG, prepare_write_env->prepare_buf, prepare_write_env->prepare_len);
        set_wifi_cfg(prepare_write_env->prepare_buf,prepare_write_env->prepare_len);
    }else{
        ESP_LOGI(GATTS_TABLE_TAG,"ESP_GATT_PREP_WRITE_CANCEL");
    }
    if (prepare_write_env->prepare_buf) {
        free(prepare_write_env->prepare_buf);
        prepare_write_env->prepare_buf = NULL;
    }
    prepare_write_env->prepare_len = 0;
}

static void gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    switch (event) {
        case ESP_GATTS_REG_EVT:{
            set_user_ble_name();

    #ifdef CONFIG_SET_RAW_ADV_DATA
            esp_err_t raw_adv_ret = esp_ble_gap_config_adv_data_raw(raw_adv_data, sizeof(raw_adv_data));
            if (raw_adv_ret){
                ESP_LOGE(GATTS_TABLE_TAG, "config raw adv data failed, error code = %x ", raw_adv_ret);
            }
            adv_config_done |= ADV_CONFIG_FLAG;
            esp_err_t raw_scan_ret = esp_ble_gap_config_scan_rsp_data_raw(raw_scan_rsp_data, sizeof(raw_scan_rsp_data));
            if (raw_scan_ret){
                ESP_LOGE(GATTS_TABLE_TAG, "config raw scan rsp data failed, error code = %x", raw_scan_ret);
            }
            adv_config_done |= SCAN_RSP_CONFIG_FLAG;
    #else
            //config adv data
            esp_err_t ret = esp_ble_gap_config_adv_data(&adv_data);
            if (ret){
                ESP_LOGE(GATTS_TABLE_TAG, "config adv data failed, error code = %x", ret);
            }
            adv_config_done |= ADV_CONFIG_FLAG;
            //config scan response data
            ret = esp_ble_gap_config_adv_data(&scan_rsp_data);
            if (ret){
                ESP_LOGE(GATTS_TABLE_TAG, "config scan response data failed, error code = %x", ret);
            }
            adv_config_done |= SCAN_RSP_CONFIG_FLAG;
    #endif
            esp_err_t create_attr_ret;
            create_attr_ret = esp_ble_gatts_create_attr_tab(gatt_db_ftms, gatts_if, HRS_IDX_NB, SVC_INST_ID);
            if (create_attr_ret){
                ESP_LOGE(GATTS_TABLE_TAG, "create attr table failed, error code = %x", create_attr_ret);
            }
            create_attr_ret = esp_ble_gatts_create_attr_tab(gatt_db_DIS, gatts_if, IDX_SVC_DIS_MAX, 1);
            if (create_attr_ret){
                ESP_LOGE(GATTS_TABLE_TAG, "create attr table failed, error code = %x", create_attr_ret);
            }
            create_attr_ret = esp_ble_gatts_create_attr_tab(gatt_db_FFF0, gatts_if, IDX_FFF0_MAX, 2);
            if (create_attr_ret){
                ESP_LOGE(GATTS_TABLE_TAG, "create attr table failed, error code = %x", create_attr_ret);
            }
            create_attr_ret = esp_ble_gatts_create_attr_tab(gatt_db_BATTERY, gatts_if, IDX_BATTERY_MAX, 3);
            if (create_attr_ret){
                ESP_LOGE(GATTS_TABLE_TAG, "create attr table failed, error code = %x", create_attr_ret);
            }else{
                ESP_LOGI(GATTS_TABLE_TAG, "create attr table succeed gatt_db_BATTERY");
            }
            create_attr_ret = esp_ble_gatts_create_attr_tab(gatt_db_EEE0, gatts_if, IDX_EEE0_MAX, 4);
            if (create_attr_ret){
                ESP_LOGE(GATTS_TABLE_TAG, "create attr table failed, error code = %x", create_attr_ret);
            }else{
                ESP_LOGI(GATTS_TABLE_TAG, "create attr table succeed gatt_db_EEE0");
            }
        }
       	    break;
        case ESP_GATTS_READ_EVT:
            ESP_LOGI(GATTS_TABLE_TAG, "GATT_READ_EVT, conn_id %d, trans_id %d, handle %d\n", param->read.conn_id, param->read.trans_id, param->read.handle);
       	    break;
        case ESP_GATTS_WRITE_EVT:
            if (!param->write.is_prep){
                // the data length of gattc write  must be less than GATTS_DEMO_CHAR_VAL_LEN_MAX.
                ESP_LOGI(GATTS_TABLE_TAG, "GATT_WRITE_EVT, handle = %d, value len = %d, value :", param->write.handle, param->write.len);
                esp_log_buffer_hex(GATTS_TABLE_TAG, param->write.value, param->write.len);
                if(param->write.handle==heart_rate_handle_table[IDX_CHAR_VAL_GATT_UUID_FITNESS_EXTENSION_DATA]){
                    decoder_d18d2c10(param->write.value,param->write.len);
                }else if(param->write.handle==heart_rate_handle_table[IDX_CHAR_VAL_GATT_UUID_FITNESS_MACHINE_CONTROL_POINT]){
                    decoder_device(param->write.value,param->write.len);
                }else if(param->write.handle==FFF0_handle_table[IDX_CHAR_FFF2_VAL]){
                    decoder_fff2(param->write.value,param->write.len);
                }else if(param->write.handle==FFF0_handle_table[IDX_CHAR_FFF9_VAL]){
                    set_wifi_cfg(param->write.value,param->write.len);
                }else if(param->write.handle==FFF0_handle_table[IDX_CHAR_FFF8_VAL]){
                    decoder_fff8(param->write.value, param->write.len);
                }else if(param->write.handle==FFF0_handle_table[IDX_CHAR_FFF6_VAL]){
                    decoder_fff6(param->write.value, param->write.len);
                }
                if (param->write.need_rsp){
                    esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
                }
            }else{
                /* handle prepare write */
                example_prepare_write_event_env(gatts_if, &prepare_write_env, param);
            }
      	    break;
        case ESP_GATTS_EXEC_WRITE_EVT: 
            // the length of gattc prepare write data must be less than GATTS_DEMO_CHAR_VAL_LEN_MAX. 
            ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_EXEC_WRITE_EVT");
            example_exec_write_event_env(&prepare_write_env, param);
            break;
        case ESP_GATTS_MTU_EVT:
            ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_MTU_EVT, MTU %d", param->mtu.mtu);
            break;
        case ESP_GATTS_CONF_EVT:
            //ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_CONF_EVT, status = %d, attr_handle %d", param->conf.status, param->conf.handle);
            break;
        case ESP_GATTS_START_EVT:
            ESP_LOGI(GATTS_TABLE_TAG, "SERVICE_START_EVT, status %d, service_handle %d", param->start.status, param->start.service_handle);
            break;
        case ESP_GATTS_CONNECT_EVT:
            ble_connect = true;
            user_mid_beep_short();
            ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_CONNECT_EVT, conn_id = %d", param->connect.conn_id);
            esp_log_buffer_hex(GATTS_TABLE_TAG, param->connect.remote_bda, 6);
            esp_ble_conn_update_params_t conn_params = {0};
            memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
            /* For the iOS system, please refer to Apple official documents about the BLE connection parameters restrictions. */
            conn_params.latency = 0;
            conn_params.max_int = 0x20;    // max_int = 0x20*1.25ms = 40ms
            conn_params.min_int = 0x10;    // min_int = 0x10*1.25ms = 20ms
            conn_params.timeout = 400;    // timeout = 400*10ms = 4000ms
            //start sent the update connection parameters to the peer device.
            esp_ble_gap_update_conn_params(&conn_params);
            //esp_ble_set_encryption(param->connect.remote_bda, ESP_BLE_SEC_ENCRYPT_MITM);
            // esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT,ESP_PWR_LVL_P9);
            break;
        case ESP_GATTS_DISCONNECT_EVT:{
            ble_connect = false;
            reset_hiit_mod();
            ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_DISCONNECT_EVT, reason = 0x%x", param->disconnect.reason);
            esp_ble_gap_start_advertising(&adv_params);
            ble_disconnect_ev();
        }break;
        case ESP_GATTS_CREAT_ATTR_TAB_EVT:{
            if (param->add_attr_tab.status != ESP_GATT_OK){
                ESP_LOGE(GATTS_TABLE_TAG, "create attribute table failed, error code=0x%x", param->add_attr_tab.status);
            }else if(param->add_attr_tab.num_handle ==IDX_FFF0_MAX){
                ESP_LOGI(GATTS_TABLE_TAG, "create attribute table successfully, the number handle = %d\n",param->add_attr_tab.num_handle);
                memcpy(FFF0_handle_table, param->add_attr_tab.handles, sizeof(FFF0_handle_table));
                esp_ble_gatts_start_service(FFF0_handle_table[IDX_SVC_FFF0]);
            }else if(param->add_attr_tab.num_handle ==IDX_SVC_DIS_MAX){
                ESP_LOGI(GATTS_TABLE_TAG, "create attribute table successfully, the number handle = %d\n",param->add_attr_tab.num_handle);
                memcpy(DIS_handle_table, param->add_attr_tab.handles, sizeof(DIS_handle_table));
                esp_ble_gatts_start_service(DIS_handle_table[IDX_SVC_DIS]);
                set_user_mac();
                set_user_SN();
                set_user_HARDWARE();
                set_user_model();
            }
            else if(param->add_attr_tab.svc_uuid.uuid.uuid16==BLE_BATTERY_SERVER_UUID)
            {
                ESP_LOGI(GATTS_TABLE_TAG, "create attribute table successfully, the number handle = %d\n",param->add_attr_tab.num_handle);
                memcpy(BATTERY_handle_table, param->add_attr_tab.handles, sizeof(BATTERY_handle_table));
                esp_ble_gatts_start_service(BATTERY_handle_table[IDX_SVC_BATTERY]);
            }
            else if (param->add_attr_tab.svc_uuid.uuid.uuid16 == BLE_EEE0_SERVER_UUID)
            {
                memcpy(EEE0_handle_table,
                       param->add_attr_tab.handles,
                       sizeof(EEE0_handle_table));
                esp_ble_gatts_start_service(EEE0_handle_table[IDX_SVC_EEE0]);
            }
            else if (param->add_attr_tab.num_handle != HRS_IDX_NB){
                ESP_LOGE(GATTS_TABLE_TAG, "create attribute table abnormally, num_handle (%d) \
                        doesn't equal to HRS_IDX_NB(%d)", param->add_attr_tab.num_handle, HRS_IDX_NB);
            }
            else {
                ESP_LOGI(GATTS_TABLE_TAG, "create attribute table successfully, the number handle = %d\n",param->add_attr_tab.num_handle);
                memcpy(heart_rate_handle_table, param->add_attr_tab.handles, sizeof(heart_rate_handle_table));
                esp_ble_gatts_start_service(heart_rate_handle_table[IDX_SVC]);
            }
            break;
        }
        case ESP_GATTS_STOP_EVT:
        case ESP_GATTS_OPEN_EVT:
        case ESP_GATTS_CANCEL_OPEN_EVT:
        case ESP_GATTS_CLOSE_EVT:
        case ESP_GATTS_LISTEN_EVT:
        case ESP_GATTS_CONGEST_EVT:
        case ESP_GATTS_UNREG_EVT:
        case ESP_GATTS_DELETE_EVT:
        default:
        // ESP_LOGW(GATTS_TABLE_TAG,"gatts_profile_event_handler未处理事件%x",event);
            break;
    }
}


static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{

    /* If event is register event, store the gatts_if for each profile */
    if (event == ESP_GATTS_REG_EVT) {
        if (param->reg.status == ESP_GATT_OK) {
            heart_rate_profile_tab[PROFILE_APP_IDX].gatts_if = gatts_if;
        } else {
            ESP_LOGE(GATTS_TABLE_TAG, "reg app failed, app_id %04x, status %d",
                    param->reg.app_id,
                    param->reg.status);
            return;
        }
    }
    do {
        int idx;
        for (idx = 0; idx < PROFILE_NUM; idx++) {
            /* ESP_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb function */
            if (gatts_if == ESP_GATT_IF_NONE || gatts_if == heart_rate_profile_tab[idx].gatts_if) {
                if (heart_rate_profile_tab[idx].gatts_cb) {
                    heart_rate_profile_tab[idx].gatts_cb(event, gatts_if, param);
                }
            }
        }
    } while (0);
}



void ble_send_status(uint8_t status,uint8_t res_level)
{
    if(get_ble_connect()){
        uint8_t buf[2] ;
        uint8_t len ;
        switch (status)
        {
        case BLE_SEND_STATUS_RST:
            buf[0] = 0x01;buf[1] = 0x00;
            len = 2;
            break;
        case BLE_SEND_STATUS_SUSPEND:
            buf[0] = 0x02;buf[1] = 0x02;
            len = 2;
            break;
        case BLE_SEND_STATUS_START:
            buf[0] = 0x04;buf[1] = 0x00;
            len = 2;
        break;
        case BLE_SEND_STATUS_STOP:
            buf[0] = 0x02;buf[1] = 0x01;
            len = 2;
        break;
        case BLE_SEND_STATUS_RES:
            buf[0] = 0x07;buf[1] = res_level;
            len = 2;
        break;

        default:
            break;
        }
        esp_ble_gatts_send_indicate(heart_rate_profile_tab[0].gatts_if, heart_rate_profile_tab[0].conn_id,
        heart_rate_handle_table[IDX_CHAR_VAL_GATT_UUID_FITNESS_MACHINE_STATUS],len, buf, false);
    }
}

uint8_t get_ble_connect(void)
{
    return ble_connect;
}

uint8_t get_ble_connect_set_timer(void)
{
    static uint8_t timer  =0;
    if(ble_connect==1){
        timer ++;
        if(timer>=3){//大于等于，防止数值溢出
            ble_connect = 3;
            timer = 0;
        }
    }else if(ble_connect==0){
        timer = 0;
    }
    return ble_connect;
}

/*自定义特征值解析心率*/
static inline void decoder_d18d2c10_bpm(uint8_t *buf,uint8_t *copy_len)
{
    update_bpm(buf[*copy_len]);
    ESP_LOGI(GATTS_TABLE_TAG,"bpm = %u",buf[*copy_len]);
    *copy_len +=1;
}
/*自定义特征值解析卡路里*/
static inline void decoder_d18d2c10_kcal(uint8_t *buf,uint8_t *copy_len)
{
    uint16_t kcal = buf[*copy_len]|(buf[*copy_len+1]<<8);
    user_sports_send_queue(USER_SPORTS_IPC_CAL, kcal);
    ESP_LOGI(GATTS_TABLE_TAG,"kcal = %u",kcal);
    *copy_len +=2;
}
/*自定义特征值解析时长*/
static inline void decoder_d18d2c10_time(uint8_t *buf,uint8_t *copy_len)
{
    uint16_t time = buf[*copy_len]|(buf[*copy_len+1]<<8);
    ble_set_run_timer(time);
    ESP_LOGI(GATTS_TABLE_TAG,"time = %u",time);
    *copy_len +=2;
}
/*自定义特征值解析RGB颜色*/
static inline void decoder_d18d2c10_RGB(uint8_t *buf,uint8_t *copy_len)
{
    uint8_t RGB[3];
    RGB[0] = buf[*copy_len];
    RGB[1] = buf[*copy_len+1];
    RGB[2] = buf[*copy_len+2];
    user_rgb_send_queue(RGB[0],RGB[1],RGB[2]);
    *copy_len +=3;
}

/*自定义特征值解析距离*/
static inline void decoder_d18d2c10_Distance(uint8_t *buf, uint8_t *copy_len)
{
    uint32_t dir_temp = buf[*copy_len] |
                        buf[*copy_len + 1] << 8 |
                        buf[*copy_len + 2] << 16;
    user_sports_send_queue(USER_SPORTS_IPC_DISTANCE, dir_temp);
    *copy_len += 3;
    ESP_LOGI(GATTS_TABLE_TAG,"Distance = %u",dir_temp);
}
/*
@par1 buf 传入的数组指针
@par2 len 传入的数组长度
*/
void decoder_d18d2c10(uint8_t *buf,uint8_t len)
{
    uint8_t copy_val = buf[0];
    uint8_t copy_len = 2;//flag占用2字节
    // ESP_LOGI(GATTS_TABLE_TAG,"decoder_d18d2c10");
    
    // ESP_LOGI(GATTS_TABLE_TAG, "GATT_WRITE_EVT val = %x",copy_val);
    for (int i = 0; i < 8; i++)
    {
        if(copy_val&(0x01<<i)){
            // ESP_LOGI(GATTS_TABLE_TAG,"i = %u",i);
            switch (i)
            {
                case 0:{}break;//bit0 控制解锁码
                case 1:{decoder_d18d2c10_bpm(buf,&copy_len);} break;//bit1 控制心率
                case 2:{decoder_d18d2c10_kcal(buf,&copy_len);}break;//bit2 控制总卡路里
                case 5:{decoder_d18d2c10_time(buf,&copy_len);}break;//bit2 控制总卡路里
                case 6:{decoder_d18d2c10_RGB(buf,&copy_len);}break;//bit2 控制总卡路里
                case 7:{decoder_d18d2c10_Distance(buf,&copy_len);}break;//bit2 控制总卡路里
                default:break;                  
            }
        }
    }
}

/*
@par1 buf 传入的数组指针
@par2 len 传入的数组长度
解析控制状态
*/
void decoder_device(uint8_t *buf,uint8_t len)
{ 
    switch (buf[0])
    {
        case 0x00:{//Request Control 请求控制
            ESP_LOGI(GATTS_TABLE_TAG,"dev state Request Control");
            uint8_t buf[3] = {0x80,0x00,0x01};
            len = 3;
            esp_ble_gatts_send_indicate(heart_rate_profile_tab[0].gatts_if, heart_rate_profile_tab[0].conn_id,
            heart_rate_handle_table[IDX_CHAR_VAL_GATT_UUID_FITNESS_MACHINE_CONTROL_POINT],len, buf, true);
        }break;
        case 0x01:{//Reset 请求重置控制
            ESP_LOGI(GATTS_TABLE_TAG,"dev state reset");
            uint8_t buf[3] = {0x80,0x01,0x01};
            len = 3;
            esp_ble_gatts_send_indicate(heart_rate_profile_tab[0].gatts_if, heart_rate_profile_tab[0].conn_id,
            heart_rate_handle_table[IDX_CHAR_VAL_GATT_UUID_FITNESS_MACHINE_CONTROL_POINT],len, buf, true);
        }break;
        case 0x07:{//启动或恢复
            ESP_LOGI(GATTS_TABLE_TAG,"dev state run");
            uint8_t buf[3] = {0x80,0x07,0x01};
            len = 3;
            esp_ble_gatts_send_indicate(heart_rate_profile_tab[0].gatts_if, heart_rate_profile_tab[0].conn_id,
            heart_rate_handle_table[IDX_CHAR_VAL_GATT_UUID_FITNESS_MACHINE_CONTROL_POINT],len, buf, true);
            xEventGroupSetBits(KEY_Event_Handle,KEY1_RUN_EVENT);/* 触发一个短按事件 */
        }break;
        case 0x08:{//停止或暂停
            ESP_LOGI(GATTS_TABLE_TAG,"dev state sp or st");
            if(buf[1]==0x02){
                ESP_LOGI(GATTS_TABLE_TAG,"dev state is sp");
                uint8_t buf[3] = {0x80,0x08,0x01};
                len = 3;
                esp_ble_gatts_send_indicate(heart_rate_profile_tab[0].gatts_if, heart_rate_profile_tab[0].conn_id,
                heart_rate_handle_table[IDX_CHAR_VAL_GATT_UUID_FITNESS_MACHINE_CONTROL_POINT],len, buf, true);
                xEventGroupSetBits(KEY_Event_Handle,KEY1_SUSPEND_EVENT);/* 触发一个长按事件 */
            }else if(buf[1]==0x01){
                uint8_t buf[3] = {0x80,0x08,0x01};
                len = 3;
                esp_ble_gatts_send_indicate(heart_rate_profile_tab[0].gatts_if, heart_rate_profile_tab[0].conn_id,
                heart_rate_handle_table[IDX_CHAR_VAL_GATT_UUID_FITNESS_MACHINE_CONTROL_POINT],len, buf, true);
                xEventGroupSetBits(KEY_Event_Handle,KEY1_LONG_PRESS_EVENT);/* 触发一个长按事件 */
                ESP_LOGI(GATTS_TABLE_TAG,"dev state is stopp");
            }
        }break;
        case 0x09:{//自启停控制
            if(buf[1]==0x01){
                set_start_suspend_lock();
                ESP_LOGI(GATTS_TABLE_TAG,"set_start_suspend_lock");
            }else{
                reset_start_suspend_lock();
                ESP_LOGI(GATTS_TABLE_TAG,"reset_start_suspend_lock");
            }
            uint8_t buf[3] = {0x80,0x09,0x01};
            len = 3;
            esp_ble_gatts_send_indicate(heart_rate_profile_tab[0].gatts_if, heart_rate_profile_tab[0].conn_id,
            heart_rate_handle_table[IDX_CHAR_VAL_GATT_UUID_FITNESS_MACHINE_CONTROL_POINT],len, buf, true);
        }break;
        case 0x04:{//设置阻力
            ESP_LOGI(GATTS_TABLE_TAG,"阻力变更%u",buf[1]);
            set_res(buf[1]*2);
            uint8_t re_buf[3] = {0x80, 0x04, 0x01};
            esp_ble_gatts_send_indicate(heart_rate_profile_tab[0].gatts_if,
                                        heart_rate_profile_tab[0].conn_id,
                                        heart_rate_handle_table[IDX_CHAR_VAL_GATT_UUID_FITNESS_MACHINE_CONTROL_POINT],
                                        sizeof(re_buf), re_buf, true);
        }break;
        case 0x16 :{
            user_sports_send_queue(USER_SPORTS_IPC_SETTING_CAL, buf[1]);
            uint8_t re_buf[3] = {0x80, 0x16, 0x01};
            esp_ble_gatts_send_indicate(heart_rate_profile_tab[0].gatts_if,
                                        heart_rate_profile_tab[0].conn_id,
                                        heart_rate_handle_table[IDX_CHAR_VAL_GATT_UUID_FITNESS_MACHINE_CONTROL_POINT],
                                        sizeof(re_buf), re_buf, true);
        }
        break;

        default:
        {
            uint8_t re_buf[3] = {0x80, 0x16, 0x02};
            re_buf[1] = buf[0];
            esp_ble_gatts_send_indicate(heart_rate_profile_tab[0].gatts_if,
                                        heart_rate_profile_tab[0].conn_id,
                                        heart_rate_handle_table[IDX_CHAR_VAL_GATT_UUID_FITNESS_MACHINE_CONTROL_POINT], sizeof(re_buf), re_buf, true);
            ESP_LOGI(GATTS_TABLE_TAG, "未知代码 dev state,%x", buf[0]);
        }
        break;
        }
}

IRAM_ATTR void decoder_fff8(uint8_t *buf,uint8_t len)
{
    switch (buf[0])
    {
    case 0x01:
    {
        uint32_t rpm = (buf[3] << 8) | buf[4];
        uint32_t target_power = (buf[1] << 8) | buf[2];
        uint32_t target_res = 0;
        if (get_ble_connect())
        {
            uint8_t tx_buf[6];
            target_res = get_CAL_target_res(target_power,rpm);
            tx_buf[0] = 02;
            tx_buf[1] = buf[1];
            tx_buf[2] = buf[2];
            tx_buf[3] = buf[3];
            tx_buf[4] = buf[4];
            tx_buf[5] = target_res;
            esp_ble_gatts_send_indicate(
                heart_rate_profile_tab[0].gatts_if,
                heart_rate_profile_tab[0].conn_id,
                FFF0_handle_table[IDX_CHAR_FFF8_VAL],
                sizeof(tx_buf), tx_buf, false);
            ESP_LOGI("decoder_fff8", "power = %u", target_power);
            ESP_LOGI("decoder_fff8", "rpm = %u", rpm);
            ESP_LOGI("decoder_fff8", "target_res = %u", target_res);
        }
    }
    break;

    default:
        break;
    }
}

IRAM_ATTR void decoder_fff6(uint8_t *buf, uint8_t len)
{
    update_check(buf[0]);
}

/*解析自定义FFF2特征值下的指令*/
void decoder_fff2(uint8_t *buf,uint8_t len)
{
    if(buf[0]==0xf5){
        switch (buf[1])
        {
        case 0xf1:
            switch (buf[2])
            {
            case 0x24:xEventGroupSetBits(KEY_Event_Handle,KEY1_SWITCH_TO_PWR);ESP_LOGD(GATTS_TABLE_TAG,"go pwr");break;//直接跳转心率界面
            case 0x25:xEventGroupSetBits(KEY_Event_Handle,KEY1_SWITCH_TO_BPM);ESP_LOGD(GATTS_TABLE_TAG,"go bpm");break;//直接跳转心率界面
            case 0x26:xEventGroupSetBits(KEY_Event_Handle,KEY1_SWITCH_TO_RPM);ESP_LOGD(GATTS_TABLE_TAG,"go rpm");break;//直接跳转踏频界面
            case 0x28:update_bpm_lock(buf[3]);ESP_LOGD(GATTS_TABLE_TAG,"rebpm");break;//心率界面的上锁解锁
            case 0x32:update_bpm_threshold_val(buf[3]);break;/*更新心率阈值*/
            case 0x36:update_power_threshold_val(buf[3]|(buf[4]<<8));break;/*更新功率阈值*/
            case 0x37:update_hiit_mod(buf[3]);break;/*更新HIIT模式控制*/
            case 0x38:{
                uint16_t temp = buf[4]|(buf[5]<<8);//更新后燃灯持续时间，单位为秒
                update_hiit_over_led_timer(temp);
            }break;/*更新HIIT后燃灯时间*/
            default:
                ESP_LOGE(GATTS_TABLE_TAG,"decoder_fff2 未识别命令 = %x",buf[2]);
                break;
            }
            break;
        
        default:
            break;
        }
    }else{
        ESP_LOGI(GATTS_TABLE_TAG,"decoder_fff2 err");
    }
}

void ble_disconnect_ev(void)
{
    update_bpm_lock(0);
    reset_start_suspend_lock();
    user_rgb_send_queue(0xff,0xff,0xff);
}

void update_Training_Status(uint8_t state)
{
    Training_Status_value[1] = state;
    esp_ble_gatts_set_attr_value(heart_rate_handle_table[IDX_CHAR_VAL_GATT_UUID_TRAINING_STATUS],2,Training_Status_value);
    if(ble_connect){
        esp_ble_gatts_send_indicate(heart_rate_profile_tab[0].gatts_if, heart_rate_profile_tab[0].conn_id,
        heart_rate_handle_table[IDX_CHAR_VAL_GATT_UUID_TRAINING_STATUS],2, Training_Status_value, false);
    }
    ESP_LOGI(GATTS_TABLE_TAG,"Training_Status_value %x",state);
}


void update_wifi_state(const uint8_t* buf,uint8_t buf_size)
{
    if(get_ble_connect()){
        printf("buf_size = %u\r\n",buf_size);
        esp_ble_gatts_send_indicate(heart_rate_profile_tab[0].gatts_if, heart_rate_profile_tab[0].conn_id,
        FFF0_handle_table[IDX_CHAR_FFF9_VAL],buf_size, (uint8_t*)buf, false);
    }
}

void ble_adv(void)
{
    esp_ble_gap_start_advertising(&adv_params);
}

static void ble_task(void *arg)
{
    User_BLE_IPC_t r_queue;
    user_BLE_queue_init();
    while (1)
    {
        if (BLE_queue == NULL)
        {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        else
        {
            int ret = xQueueReceive(BLE_queue, &r_queue, 1000 / portTICK_PERIOD_MS);
            if (ret)
            {
                switch (r_queue.ssid)
                {
                case BLE_IPC_SIID_INDOOR_BIKE:
                    if (get_ble_connect())
                    {
                        esp_ble_gatts_send_indicate(
                            heart_rate_profile_tab[0].gatts_if,
                            heart_rate_profile_tab[0].conn_id,
                            heart_rate_handle_table[IDX_CHAR_GATT_UUID_SPORTS_DATA_VAL],
                            r_queue.len, r_queue.buf, false);
                    }
                    break;

                case BLE_IPC_SIID_CHECK_INFO:
                {
                    if (get_ble_connect())
                    {
                        esp_ble_gatts_send_indicate(
                            heart_rate_profile_tab[0].gatts_if,
                            heart_rate_profile_tab[0].conn_id,
                            FFF0_handle_table[IDX_CHAR_FFF6_VAL],
                            r_queue.len, r_queue.buf, false);
                    }
                }
                break;
                default:
                    break;
                }
            }
            if (get_ble_connect())
            {
                uint8_t batt = get_batt();
                esp_ble_gatts_send_indicate(
                    heart_rate_profile_tab[0].gatts_if,
                    heart_rate_profile_tab[0].conn_id,
                    BATTERY_handle_table[IDX_CHAR_BATTERY_VAL],
                    1, &batt, false);
                esp_ble_gatts_set_attr_value(BATTERY_handle_table[IDX_CHAR_BATTERY_VAL],
                                             1, &batt);
            }
        }
    }
}

void ble_init(void)
{
    esp_err_t ret;
    
    switch (get_model())
    {
    case DEV_MODEL_M1S:
        set_ftms_sports_type(SPORTS_TYPE_BIKE);
        break;
    case DEV_MODEL_M1M:
        set_ftms_sports_type(SPORTS_TYPE_BIKE);
        break;
    case DEV_MODEL_M1C:
        set_ftms_sports_type(SPORTS_TYPE_BIKE);
        break;
    case DEV_MODEL_M2M:
        set_ftms_sports_type(SPORTS_TYPE_BIKE);
        break;
    case DEV_MODEL_E80:
        set_ftms_sports_type(SPORTS_TYPE_CROSSTRAINER);
        break;
    case DEV_MODEL_E80EV:
        set_ftms_sports_type(SPORTS_TYPE_CROSSTRAINER);
        break;
    case DEV_MODEL_EF25EV:
        set_ftms_sports_type(SPORTS_TYPE_CROSSTRAINER);
        break;

    default:
        set_ftms_sports_type(SPORTS_TYPE_BIKE);
        break;
    }

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(GATTS_TABLE_TAG, "%s enable controller failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(GATTS_TABLE_TAG, "%s enable controller failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(GATTS_TABLE_TAG, "%s init bluetooth failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(GATTS_TABLE_TAG, "%s enable bluetooth failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_ble_gatts_register_callback(gatts_event_handler);
    if (ret){
        ESP_LOGE(GATTS_TABLE_TAG, "gatts register error, error code = %x", ret);
        return;
    }

    ret = esp_ble_gap_register_callback(gap_event_handler);
    if (ret){
        ESP_LOGE(GATTS_TABLE_TAG, "gap register error, error code = %x", ret);
        return;
    }

    ret = esp_ble_gatts_app_register(ESP_APP_ID);
    if (ret){
        ESP_LOGE(GATTS_TABLE_TAG, "gatts app register error, error code = %x", ret);
        return;
    }

    esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(500);
    if (local_mtu_ret){
        ESP_LOGE(GATTS_TABLE_TAG, "set local  MTU failed, error code = %x", local_mtu_ret);
    }
    
    /* set the security iocap & auth_req & key size & init key response key parameters to the stack*/
    esp_ble_auth_req_t auth_req = ESP_LE_AUTH_REQ_SC_MITM_BOND;     //bonding with peer device after authentication
    esp_ble_io_cap_t iocap = ESP_IO_CAP_NONE;           //set the IO capability to No output No input
    uint8_t key_size = 16;      //the key size should be 7~16 bytes
    uint8_t init_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    uint8_t rsp_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    //set static passkey
    uint32_t passkey = 123456;
    uint8_t auth_option = ESP_BLE_ONLY_ACCEPT_SPECIFIED_AUTH_DISABLE;
    uint8_t oob_support = ESP_BLE_OOB_DISABLE;
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_STATIC_PASSKEY, &passkey, sizeof(uint32_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_MAX_KEY_SIZE, &key_size, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_ONLY_ACCEPT_SPECIFIED_SEC_AUTH, &auth_option, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_OOB_SUPPORT, &oob_support, sizeof(uint8_t));
    /* If your BLE device acts as a Slave, the init_key means you hope which types of key of the master should distribute to you,
    and the response key means which key you can distribute to the master;
    If your BLE device acts as a master, the response key means you hope which types of key of the slave should distribute to you,
    and the init key means which key you can distribute to the slave. */
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_INIT_KEY, &init_key, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_RSP_KEY, &rsp_key, sizeof(uint8_t));
    xTaskCreatePinnedToCore(ble_task,"ble_task",2048,NULL,3,NULL,tskNO_AFFINITY);
}
