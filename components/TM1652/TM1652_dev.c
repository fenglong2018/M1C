#include "TM1652_dev.h"

void TM1652_send_data(const uint8_t *data_p,uint32_t len)
{
    uart_write_bytes(UART_NUM_1, data_p, len);
}

void TM1652_clear(void)
{
    static const uint8_t clear_buf[] = {0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    TM1652_send_data(clear_buf,sizeof(clear_buf));
}

void TM1652_all_show(void)
{
    static const uint8_t clear_buf[] = {0x08, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    TM1652_send_data(clear_buf,sizeof(clear_buf));
}

void TM1652_init(void)
{
    static const uint8_t init_cmd[] = {0x18, 0xFE};//默认初始化命令
    TM1652_send_data(init_cmd,sizeof(init_cmd));
}