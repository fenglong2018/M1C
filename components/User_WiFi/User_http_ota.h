#ifndef _USER_HTTP_OTA_H_
#define _USER_HTTP_OTA_H_

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_https_ota.h"


esp_err_t user_esp_https_ota(const esp_http_client_config_t *config);


#endif
