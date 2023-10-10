#include "User_NVS.h"
#include "nvs_flash.h"
#include <string.h>
#include "esp_log.h"
#include "esp_system.h"

#define USER_NVS_TAG "USER_NVS"

static esp_err_t read_user_dev_info(const char* KEY, char *buf,uint32_t buf_len);
static void set_user_mac(void);

static struct User_NVS
{
    uint8_t dev_model_val;
    char *model;
} User_NVS_list = {
    .dev_model_val = DEV_MODEL_M1S,
};

static void update_model(void)
{
    char buf[64] = {0};
    read_user_MODEL(buf, sizeof(buf));

    if (strcmp("M1M", buf) == 0)
    {
        User_NVS_list.dev_model_val = DEV_MODEL_M1M;
        ESP_LOGI(USER_NVS_TAG, "设备型号是M1M");
    }
    else if (strcmp("M2M", buf) == 0)
    {
        User_NVS_list.dev_model_val = DEV_MODEL_M2M;

        ESP_LOGI(USER_NVS_TAG, "设备型号是M1M");
    }
    else if (strcmp("M1S", buf) == 0)
    {
        User_NVS_list.dev_model_val = DEV_MODEL_M1S;

        ESP_LOGI(USER_NVS_TAG, "设备型号是M1S");
    }
    else if (strcmp("E80", buf) == 0)
    {
        User_NVS_list.dev_model_val = DEV_MODEL_E80;

        ESP_LOGI(USER_NVS_TAG, "设备型号是E80");
    }  
    else if (strcmp("E80EV", buf) == 0)
    {
        User_NVS_list.dev_model_val = DEV_MODEL_E80EV;

        ESP_LOGI(USER_NVS_TAG, "设备型号是E80EV");
    }
    else if (strcmp("M1C", buf) == 0)
    {
        User_NVS_list.dev_model_val = DEV_MODEL_M1C;
        ESP_LOGI(USER_NVS_TAG, "设备型号是DEV_MODEL_M1C");
    }
    else if (strcmp("EF25EV", buf) == 0)
    {
        User_NVS_list.dev_model_val = DEV_MODEL_EF25EV;
        ESP_LOGI(USER_NVS_TAG, "设备型号是DEV_MODEL_EF25");
    }
    else if (strcmp("E20EVS", buf) == 0)
    {
        User_NVS_list.dev_model_val = DEV_MODEL_E20EVS;
        ESP_LOGI(USER_NVS_TAG, "设备型号是DEV_MODEL_E20EVS");
    }
    else
    {
        User_NVS_list.dev_model_val = DEV_MODEL_M1S;
        ESP_LOGE(USER_NVS_TAG, "设备型号是未定义的,默认生效M1S");
    }
}

uint8_t get_model(void)
{
    return User_NVS_list.dev_model_val;
}

bool is_EV(void)
{
    bool ret = false;
    switch (User_NVS_list.dev_model_val)
    {
    case DEV_MODEL_M1S:
        ret = false;
        break;
    case DEV_MODEL_M1M:
        ret = true;
        break;
    case DEV_MODEL_M2M:
        ret = true;
        break;
    case DEV_MODEL_E80:
        ret = true;
        break;
    case DEV_MODEL_E80EV:
        ret = true;
        break;
    case DEV_MODEL_M1C:
        ret = true;
        break;
    case DEV_MODEL_EF25EV:
        ret = true;
        break;
    case DEV_MODEL_E20EVS:
        ret = true;
        break;

    default:
        break;
    }
    return ret;
}

void user_NVS_init(void)
{
    esp_err_t ret;
    /* Initialize NVS. */
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_LOGE(USER_NVS_TAG,"ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND");
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );
    set_user_mac();
    update_model();
}

esp_err_t read_user_dev_info(const char* KEY, char *buf,uint32_t buf_len)
{
    nvs_handle_t my_handle;
    esp_err_t err;
    err = nvs_open("dev_info", NVS_READONLY, &my_handle);
    if (err == ESP_OK){
        // Read restart counter
        err = nvs_get_str(my_handle,KEY,buf,&buf_len);
        if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND){
            ESP_LOGE(USER_NVS_TAG,"dev_infonvs句柄开启失败");
        }else{
            ESP_LOGI(USER_NVS_TAG,"%s=%s",KEY,buf);
        }
    }else{
        ESP_LOGE(USER_NVS_TAG,"BLE_nvs句柄开启失败");
    }
    nvs_close(my_handle);
    return ESP_OK;
}

esp_err_t read_user_ble_HARDWARE(char *buf,uint32_t buf_len)
{
    return read_user_dev_info("HARDWARE",buf,buf_len);
}

esp_err_t read_user_MODEL(char *buf,uint32_t buf_len)
{
    return read_user_dev_info("MODEL",buf,buf_len);
}

esp_err_t read_user_ble_name(char *buf,uint32_t buf_len)
{
    return read_user_dev_info("BLE_NAME",buf,buf_len);
}

esp_err_t read_user_mac(char *buf,uint32_t buf_len)
{
    return read_user_dev_info("MAC",buf,buf_len);
}

esp_err_t read_user_SN(char *buf,uint32_t buf_len)
{
    return read_user_dev_info("SN",buf,buf_len);
}

esp_err_t read_user_LCD_ver(char *buf,uint32_t buf_len)
{
    return read_user_dev_info("LCD",buf,buf_len);
}


static char ASCII_to_dec(char val)
{
    if(val<0x40){
        return val-0x30;
    }else{
        switch (val)
        {
        case 'a':val = 0x0a;break;
        case 'b':val = 0x0b;break;
        case 'c':val = 0x0c;break;
        case 'd':val = 0x0d;break;
        case 'e':val = 0x0e;break;
        case 'f':val = 0x0f;break;
        case 'A':val = 0x0a;break;
        case 'B':val = 0x0b;break;
        case 'C':val = 0x0c;break;
        case 'D':val = 0x0d;break;
        case 'E':val = 0x0e;break;
        case 'F':val = 0x0f;break;
        default:
            break;
        }
    }
    return val;
}

static void set_user_mac(void)
{
    char read_buff[100] = {0};
    esp_err_t err = read_user_mac(read_buff,sizeof(read_buff));
    if(err!=ESP_OK){//读取的mac操作失败，那就不要管了
        return ;
    }
    if(read_buff[0]!=0x00&&strcmp(read_buff,"000000000000")){//检查到MAC
        uint8_t mac[6];
        mac[0] = (ASCII_to_dec(read_buff[0])<<4)|ASCII_to_dec(read_buff[1]);
        mac[1] = (ASCII_to_dec(read_buff[2])<<4)|ASCII_to_dec(read_buff[3]);
        mac[2] = (ASCII_to_dec(read_buff[4])<<4)|ASCII_to_dec(read_buff[5]);
        mac[3] = (ASCII_to_dec(read_buff[6])<<4)|ASCII_to_dec(read_buff[7]);
        mac[4] = (ASCII_to_dec(read_buff[8])<<4)|ASCII_to_dec(read_buff[9]);
        mac[5] = (ASCII_to_dec(read_buff[10])<<4)|ASCII_to_dec(read_buff[11]);
        esp_err_t err =  esp_base_mac_addr_set(mac);
        if(err!=ESP_OK){
            for(int i=0;i<12;i++){
            ESP_LOGE(USER_NVS_TAG,"read_buff[%u] = %x",i,read_buff[i]);
        }       
         for(int i=0;i<6;i++){
            ESP_LOGE(USER_NVS_TAG,"MAC[%u] = %x",i,mac[i]);
        }
        ESP_LOGE(USER_NVS_TAG,"MAC = %s err=%x",read_buff,(uint32_t)err);
        }
    }else{
        ESP_LOGW(USER_NVS_TAG,"mac无需设置");
    }
}



