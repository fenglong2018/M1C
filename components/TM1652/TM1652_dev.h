#ifndef _TM1652_DEV_H_
#define _TM1652_DEV_H_

#include <stdint.h>
#include "User_dev_uart1.h"

void TM1652_send_data(const uint8_t *data_p, uint32_t len);
void TM1652_clear(void);
void TM1652_all_show(void);
void TM1652_init(void);

#endif
