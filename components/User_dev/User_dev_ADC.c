#include "User_dev_ADC.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "esp_log.h"



#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   128          //Multisampling

static esp_adc_cal_characteristics_t *adc_chars;
static const adc_bits_width_t width = ADC_WIDTH_BIT_12;
static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_unit_t unit = ADC_UNIT_1;

static const char *TAG = "user_ADC";

static void check_efuse(void)
{
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        ESP_LOGI(TAG,"eFuse Two Point: Supported\n");
    } else {
        ESP_LOGW(TAG,"Cannot retrieve eFuse Two Point calibration values. Default calibration values will be used.\n");
    }
}

static void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        ESP_LOGI(TAG,"Characterized using Two Point Value\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        ESP_LOGI(TAG,"Characterized using eFuse Vref\n");
    } else {
        ESP_LOGI(TAG,"Characterized using Default Vref\n");
    }
}



uint32_t motor_ad = 0;


uint32_t get_motor_ad(void)
{
    return motor_ad;
}


static uint32_t IRAM_ATTR _get_ad(adc1_channel_t channel)
{
    uint32_t adc_reading = 0;
    //Multisampling
    for (int i = 0; i < NO_OF_SAMPLES; i++) {
        if (unit == ADC_UNIT_1) {
            adc_reading += adc1_get_raw((adc1_channel_t)channel);
        }
    }
    adc_reading /= NO_OF_SAMPLES;
    return adc_reading;
}




void IRAM_ATTR adc_task(void *arg)
{
    while (true)
    {
        uint32_t adc_reading = 0;
        uint32_t voltage = 0;
    
        //Convert adc_reading to voltage in mV
        adc_reading = _get_ad(ADC_CHANNEL_0);
        voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
        // ESP_LOGI(TAG,"Raw0: %d\tVoltage: %dmV\n",adc_reading, voltage);
        motor_ad = voltage;
        vTaskDelay(30 / portTICK_PERIOD_MS);
    }
}


void adc_init(void)
{
    check_efuse();
    adc1_config_width(width);
    adc1_config_channel_atten(ADC_CHANNEL_0, atten);
    //Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, width, DEFAULT_VREF, adc_chars);
    print_char_val_type(val_type);
    xTaskCreate(adc_task, "adc_task", 2048, NULL, 1, NULL);
}