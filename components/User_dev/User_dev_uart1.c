#include "User_dev_uart1.h"
#include "esp_log.h"

#define TXD_PIN (GPIO_NUM_22)

static xQueueHandle uart1_queue = NULL;

typedef struct uart1_queue
{
    uint8_t *data;
    uint32_t len;
}uart1_queue_t;

void User_dev_uart1_send_queue(uint8_t *data_p,uint32_t len)
{
    if(uart1_queue!=NULL)
    {
        uart1_queue_t uart1_queue_temp;
        uart1_queue_temp.data = data_p;
        uart1_queue_temp.len = len;
        xQueueSend(uart1_queue, &uart1_queue_temp, 0);
    }
}

static void tx_task(void *arg)
{
    uart1_queue_t r_uart1_queue;
    static uint8_t send_buf[50];
    while (1)
    {
        int ret = xQueueReceive(uart1_queue, &r_uart1_queue, portMAX_DELAY);
        if(ret)
        {
            
            // memcpy(send_buf, r_uart1_queue.data, 7);
            

            // ESP_LOGE("tx_task", "data[0]=%x", r_uart1_queue.data[0]);
            // ESP_LOGI("tx_task", "data[0]=%x",r_uart1_queue.data[0]);
            // ESP_LOGI("tx_task", "data[1]=%x",r_uart1_queue.data[1]);
            // ESP_LOGI("tx_task", "data[2]=%x",r_uart1_queue.data[2]);
            // ESP_LOGI("tx_task", "data[3]=%x",r_uart1_queue.data[3]);
            // ESP_LOGI("tx_task", "data[4]=%x",r_uart1_queue.data[4]);

            uart_write_bytes(UART_NUM_1, r_uart1_queue.data, sizeof(r_uart1_queue.len));
        }
    }
}

void User_dev_uart1_init(void) {
    static const uart_config_t uart_config = {
        .baud_rate = 19200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_ODD,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_REF_TICK,
    };
    // We won't use a buffer for sending data.
    uart_driver_install(UART_NUM_1, 1024, 1024, 0, NULL, 0);
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, TXD_PIN, -1, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart1_queue = xQueueCreate(10, sizeof(uart1_queue_t)); 
    // xTaskCreate(tx_task, "uart_tx_task", 1024*2, NULL, 5, NULL);
}
