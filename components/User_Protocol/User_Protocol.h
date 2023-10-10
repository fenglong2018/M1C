#ifndef _USER_PROTOCOL_H_
#define _USER_PROTOCOL_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include <stdio.h>

/*串口功能接口*/
uint16_t get_RPM(void);

void update_bpm(uint8_t bpm);
uint8_t get_bpm(void);
void update_bpm_threshold_val(uint8_t val);
uint8_t get_bpm_threshold_val(void);
uint8_t get_bpm_lock(void);
void update_bpm_lock(uint8_t lock);

void update_RPM(uint16_t rpm);

uint16_t get_power(void);
void update_power_threshold_val(uint16_t power_threshold);
uint16_t get_power_threshold_val(void);

uint8_t get_timer_H(void);
uint8_t get_timer_M(void);
uint8_t get_timer_S(void);
uint8_t get_timer_out(void);
uint32_t get_timer(void);
void set_run_timer_lock(void);
void reset_run_timer_lock(void);

uint32_t get_actual_res(void);
void set_actual_res(uint32_t val);
uint8_t get_RES(void);
void set_RES_up(uint32_t res_temp);
void set_RES_down(uint32_t res_temp);
/*设置协议内的阻力值，会直接更改阻力值*/
void set_res(int16_t res_i);

void send_protocol_order(uint8_t order);
void send_protocol_res(uint8_t res);
void send_protocol_recv(uint8_t order);
void send_protocol_recv_gui(uint8_t gui);
uint8_t get_sleep_key(void);
void set_prorocol_gui(uint8_t val);
void protocol_init(void);
uint8_t get_res_update_fg(void);
void update_RES(int8_t RES_TEMP);
uint8_t get_bpm_fg(void);
uint8_t get_LED_stall(void);
void set_run_state(void);
void reset_run_state(void);
uint8_t get_run_state(void);
void reset_suspend_state(void);
uint8_t get_suspend_state(void);
void set_suspend_state(void);
uint8_t get_bpm_threshold(void);


/*定时器组*/
void run_timer_create(void);
void run_timer_delete(void);
void ble_set_run_timer(uint32_t timer);

#define DEVICE_STATE_RUN     0X01
#define DEVICE_STATE_SUSPEND 0X02
#define DEVICE_STATE_STOP    0X03

/*设备状态更新*/
uint8_t get_device_state(void);
void update_device_state(uint8_t state);

/*更新HIIT间歇训练模式*/
void update_hiit_mod(uint8_t mod);
uint8_t get_hiit_mod(void);
void reset_hiit_mod(void);
void update_hiit_over_led_timer(uint16_t timer);
uint16_t get_hiit_over_led_timer(bool chaxun);

uint32_t get_clk(void);

uint8_t  get_start_suspend_lock(void);
void set_start_suspend_lock(void);
void reset_start_suspend_lock(void);
void protocol_deinit(void);

bool get_sports_state(void);
void set_sports_state(void);
void reset_sports_state(void);
void update_batt(uint8_t batt);
uint8_t get_batt(void);

void update_check(uint8_t check);
uint8_t get_check(void);

#endif
