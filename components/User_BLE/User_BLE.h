#ifndef _USER_BLE_H_
#define _USER_BLE_H_

#include <stdint.h>
#include <stdbool.h>
#include "../BLE_FTMS/fitness_machine_field.h"
#include <string.h>
#include "../User_WiFi/User_WiFi.h"
#include "User_BLE_IPC.h"


#define BLE_SEND_STATUS_SUSPEND 0X01
#define BLE_SEND_STATUS_START   0X02
#define BLE_SEND_STATUS_STOP    0X03
#define BLE_SEND_STATUS_RST     0X00
#define BLE_SEND_STATUS_RES     0X04
void set_user_ble_name(void);
void ble_init(void);
void tese(void);
uint8_t get_ble_connect(void);
uint8_t get_ble_connect_set_timer(void);
void decoder_d18d2c10(uint8_t *buf,uint8_t len);
void decoder_device(uint8_t *buf,uint8_t len);
void ble_send_status(uint8_t status,uint8_t res_level);
void decoder_fff2(uint8_t *buf,uint8_t len);
void decoder_fff6(uint8_t *buf, uint8_t len);
void decoder_fff8(uint8_t *buf, uint8_t len);
void ble_disconnect_ev(void);
void update_Training_Status(uint8_t state);
void ble_adv(void);
void update_wifi_state(const uint8_t* buf,uint8_t buf_size);
#endif
