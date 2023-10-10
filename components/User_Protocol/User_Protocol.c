#include "User_Protocol.h"
#include <esp_log.h>
static struct PROROCOL_STRUCT
{
	uint32_t timer;
	uint32_t timer_BLE;
	uint16_t rpm;
	uint16_t pwoer;
	uint8_t bpm;
	uint8_t bpm_lock;
	uint8_t bpm_threshold_val;
	uint8_t power_threshold_val;
	uint8_t hiit_mod;
	uint16_t hiit_over_led_timer;		// HIIT模式后燃灯
	bool hiit_over_led_timer_update_fg; // HIIT模式后燃灯
	int16_t res;
	int16_t actual_res;
	uint8_t run_lock;
	uint8_t start_suspend_lock;
	uint8_t batt;
	uint8_t check;
} protocol_list = {
	.batt = 100,
};

void update_check(uint8_t check)
{
	protocol_list.check = check;
}

uint8_t get_check(void)
{
	uint8_t temp = protocol_list.check;
	protocol_list.check = 0;
	return temp;
}

void update_batt(uint8_t batt)
{
	protocol_list.batt = batt;
}

uint8_t get_batt(void)
{
	return protocol_list.batt;
}

void update_hiit_mod(uint8_t mod)
{
	protocol_list.hiit_mod = mod;
}

uint8_t get_hiit_mod(void)
{
	return protocol_list.hiit_mod;
}

void reset_hiit_mod(void)
{
	protocol_list.hiit_mod = 0;
}

void update_hiit_over_led_timer(uint16_t timer)
{
	protocol_list.hiit_over_led_timer_update_fg = true;
	protocol_list.hiit_over_led_timer = timer;
}

/*true查询是否变更，false读取值*/
uint16_t get_hiit_over_led_timer(bool chaxun)
{
	if(chaxun){
		if(protocol_list.hiit_over_led_timer_update_fg==true){
			protocol_list.hiit_over_led_timer_update_fg = false;
			return true;
		}else{
			return false;
		}
	}
	return protocol_list.hiit_over_led_timer;
}

void set_start_suspend_lock(void)
{
	protocol_list.start_suspend_lock = 1;
}

void reset_start_suspend_lock(void)
{
	protocol_list.start_suspend_lock = 0;
}

uint8_t  get_start_suspend_lock(void)
{
	return protocol_list.start_suspend_lock;
}

void update_bpm_lock(uint8_t lock)
{
	protocol_list.bpm_lock = lock;
}

uint8_t get_bpm_lock(void)
{
	return protocol_list.bpm_lock;
}

void update_bpm_threshold_val(uint8_t val)
{
	protocol_list.bpm_threshold_val = val;
}

uint8_t get_bpm_threshold_val(void)
{
	if(protocol_list.bpm_threshold_val==0){
		protocol_list.bpm_threshold_val = 100;
	}
	return protocol_list.bpm_threshold_val;	
}

void update_power_threshold_val(uint16_t power_threshold)
{
	protocol_list.power_threshold_val = power_threshold;
}

uint16_t get_power_threshold_val(void)
{
	if(protocol_list.power_threshold_val==0){
		return 250;
	}
	return protocol_list.power_threshold_val;
}

void update_RPM(uint16_t rpm)
{
	protocol_list.rpm = rpm;
}

uint16_t get_RPM(void)
{
	return protocol_list.rpm;
}

void set_actual_res(uint32_t val)
{
	protocol_list.actual_res = val;
}

uint32_t get_actual_res(void)
{
	return protocol_list.actual_res;
}

void set_res(int16_t res_i)
{
     protocol_list.res = res_i;
}

void IRAM_ATTR set_RES_up(uint32_t res_temp){
    if(0==res_temp)
    {
        res_temp = 1;
    }
    if(protocol_list.res<200){
        protocol_list.res+=res_temp;
    }
    if(protocol_list.res>200){
        protocol_list.res = 200;
    }
}
void IRAM_ATTR set_RES_down(uint32_t res_temp){
	if(0==res_temp)
    {
        res_temp = 1;
    }
    if(protocol_list.res>0){
        protocol_list.res-=res_temp;
    }
    if(protocol_list.res<0){
        protocol_list.res = 0;
    }
}

uint8_t get_RES(void){
	return protocol_list.res/2;
}

/********************************************************************************************************
* @brief	获取时间——时
* @param 	void
* @return	小时数，最大99
********************************************************************************************************/
uint8_t get_timer_H(void){
	return protocol_list.timer/3600;
}
/********************************************************************************************************
* @brief	获取时间——分
* @param 	void
* @return	分钟数，最大60
********************************************************************************************************/
uint8_t get_timer_M(void){
	return protocol_list.timer%3600/60;
}
/********************************************************************************************************
* @brief	获取时间——秒
* @param 	void
* @return	秒数，最大60
********************************************************************************************************/
uint8_t get_timer_S(void){
	return protocol_list.timer%60;
}

uint32_t get_timer(void)
{
	return protocol_list.timer;
}

void set_run_timer_lock(void)
{
	protocol_list.run_lock = 1;
	protocol_list.timer_BLE = 0;//暂停后，将允许再次同步时间
}

void reset_run_timer_lock(void)
{
	protocol_list.run_lock = 0;
}

static void run_timer_cb(void *arg);

static esp_timer_handle_t run_timer_handle = NULL;
static const esp_timer_create_args_t run_timer = {
    .callback = &run_timer_cb,
    .arg = NULL,
    .name ="run_timer",
    .dispatch_method = ESP_TIMER_TASK
};

extern uint8_t get_ble_connect(void);
static IRAM_ATTR void run_timer_cb(void *arg)
{
	if(protocol_list.run_lock==0){//未上锁的时候代表允许计数，上锁可能结束或暂停运动
		protocol_list.timer++;
		//ESP_LOGE("run","timer+= %u",protocol_list.timer);
	}else{
		protocol_list.timer_BLE = 0;
	}
	if(get_ble_connect()==0){//蓝牙断开后，将允许再次同步时间
		protocol_list.timer_BLE = 0;
	}
}

void IRAM_ATTR ble_set_run_timer(uint32_t timer)
{
	// if(protocol_list.timer_BLE==0){
	// 	protocol_list.timer_BLE = timer;
		
	// 	if(protocol_list.timer>timer){//如果当前时间已经大于设置时间了，那么不进行时间变更

	// 	}else{//反之对时间进行变更
			protocol_list.timer = timer;
	// 	}
	// }
}

void run_timer_create(void)
{
    esp_err_t err = esp_timer_create(&run_timer, &run_timer_handle);
    err = esp_timer_start_periodic(run_timer_handle, 1000*1000 );//1秒回调
	protocol_list.timer_BLE = 0;
	if(err==ESP_OK){
		reset_run_timer_lock();
    }else{
        ESP_LOGE("PRO","\r\nrun_timer定时器创建失败\r\n");
    }
}

void run_timer_delete(void)
{
	esp_timer_stop(run_timer_handle); //定时器暂停
    esp_err_t err = esp_timer_delete(run_timer_handle);
	protocol_list.timer = 0;
	protocol_list.timer_BLE = 0;
	if(err==ESP_OK){
		set_run_timer_lock();
    }else{
        ESP_LOGE("PRO","\r\n run_timer定时器删除失败\r\n");
    }
}

uint8_t get_bpm(void)
{
	return protocol_list.bpm;
}

void update_bpm(uint8_t bpm)
{
	protocol_list.bpm = bpm;
}