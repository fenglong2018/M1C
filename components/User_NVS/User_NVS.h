#ifndef _USER_NVS_H_
#define _USER_NVS_H_

#include <stdint.h>
#include <stdbool.h>
#include <esp_err.h>

#define DEV_MODEL_M1S (0)
#define DEV_MODEL_M1M (1)
#define DEV_MODEL_M2M (2)
#define DEV_MODEL_E80 (3)
#define DEV_MODEL_E80EV (4)
#define DEV_MODEL_M1C (5)
#define DEV_MODEL_EF25EV (6)
#define DEV_MODEL_E20EVS (7)


/**************************************
 * @brief 初始化用户NVS
 *
 * @param[in] void
 *
 * @return void
 ***************************************/
void user_NVS_init(void);
/**
 * @brief      get user_ble_name
 *
 *
 * @param[in]     buf     存储数据缓冲区的指针
 * @param[in]     buf_len        最大可以传递多少字节的长度
 *
 * @return
 *             - ESP_OK if the value was retrieved successfully
 *             - ESP_ERR_NVS_NOT_FOUND if the requested key doesn't exist
 *             - ESP_ERR_NVS_INVALID_HANDLE if handle has been closed or is NULL
 *             - ESP_ERR_NVS_INVALID_NAME if key name doesn't satisfy constraints
 *             - ESP_ERR_NVS_INVALID_LENGTH if length is not sufficient to store data
 */
esp_err_t read_user_ble_name(char *buf,uint32_t buf_len);
esp_err_t read_user_SN(char *buf,uint32_t buf_len);
esp_err_t read_user_ble_HARDWARE(char *buf,uint32_t buf_len);
esp_err_t read_user_LCD_ver(char *buf,uint32_t buf_len);
esp_err_t read_user_MODEL(char *buf, uint32_t buf_len);
uint8_t get_model(void);
/**************************************
 * @brief 识别是否为自发电设备
 *
 * @param[in] void
 *
 * @return bool
 * @retval true :设备是自发电类型
 * @retval false:设备是插电类型
 ***************************************/
bool is_EV(void);

#endif
