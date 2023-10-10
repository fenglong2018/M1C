#include "User_encoder.h"
#include "rotary_encoder.h"
#include "esp_log.h"
static uint8_t RES_update_flag = 0;
static uint8_t RES_update_flag_2 = 0;
static uint8_t RES_LOCK = 0;


void IRAM_ATTR set_RES_LOCK(void)
{
    RES_LOCK = 1;
}
void IRAM_ATTR reset_RES_LOCK(void)
{
    RES_LOCK = 0;
}

void IRAM_ATTR RES_update_flag_clean_2(void){
	RES_update_flag_2 = 0;
}

uint8_t IRAM_ATTR get_RES_update_flag_2(void){
	if(RES_update_flag_2==1){
		RES_update_flag_2 = 0;
		return 1;
	}
	return 0;
}

uint8_t IRAM_ATTR get_RES_update_flag(void){
	if(RES_update_flag==1){
		RES_update_flag = 0;
		return 1;
	}
	return 0;
}

void IRAM_ATTR RES_update_flag_clean(void){
	RES_update_flag = 0;
}	


void IRAM_ATTR encoder_task(void *arg)
{
    // Rotary encoder underlying device is represented by a PCNT unit in this example
    uint32_t pcnt_unit = 0;

    // Create rotary encoder instance
    rotary_encoder_config_t config = ROTARY_ENCODER_DEFAULT_CONFIG((rotary_encoder_dev_t)pcnt_unit, 34, 35);
    rotary_encoder_t *encoder = NULL;
    ESP_ERROR_CHECK(rotary_encoder_new_ec11(&config, &encoder));

    // Filter out glitch (1us)
    ESP_ERROR_CHECK(encoder->set_glitch_filter(encoder, 1));

    // Start encoder
    ESP_ERROR_CHECK(encoder->start(encoder));

    // Report counter value
    int32_t old_val = 0;    //旧值存储
    int32_t val = 0;        //新值存储
    while (1) {
        val = encoder->get_counter_value(encoder);
        if(old_val>val){//左旋，阻力减小
            if(RES_LOCK!=true)//不带锁的情况下操作才有意义
            {
                set_RES_down(abs(val-old_val));
                RES_update_flag = true;
            }
            RES_update_flag_2 = true;
        }else if(old_val<val){//右旋，阻力增加
            if(RES_LOCK!=true)//不带锁的情况下操作才有意义
            {
                set_RES_up(abs(val-old_val));
                RES_update_flag = true;
            }
            RES_update_flag_2 = true;
        }
        old_val = val;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void encode_init(void)
{
    xTaskCreatePinnedToCore(encoder_task,"encoder_task",4096,NULL,2,NULL,tskNO_AFFINITY);
}
