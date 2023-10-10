#include "User_pull_motor_rpm_dev.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"
#include "soc/rtc.h"
#include "esp_log.h"
#include "User_Protocol.h"

/*typedef*/
typedef struct
{
    uint32_t capture_signal;
    uint32_t previous_cap_value;
    uint32_t current_cap_value;
    mcpwm_capture_signal_t sel_cap_signal;
} capture_t;

/*def*/
static const char *TAG = "RPM_TAG";
#define CAP0_INT_EN BIT(27) // Capture 0 interrupt bit
#define CAP1_INT_EN BIT(28) // Capture 1 interrupt bit
#define CAP2_INT_EN BIT(29) // Capture 2 interrupt bit

/*val*/
static xQueueHandle cap_queue;
extern mcpwm_dev_t MCPWM0;
static uint8_t rpm_close_fg = false;

void rpm_set_close(void)
{
    rpm_close_fg = true;
}

/*捕获中断句柄*/
static void IRAM_ATTR isr_handler(void)
{
    static DRAM_ATTR uint32_t current_cap_value = 0;
    static DRAM_ATTR uint32_t previous_cap_value = 0;

    uint32_t mcpwm_intr_status = 0;
    capture_t evt;
    mcpwm_intr_status = MCPWM0.int_st.val; // Read interrupt status
    // calculate the interval in the ISR,
    // so that the interval will be always correct even when cap_queue is not handled in time and overflow.
    if (mcpwm_intr_status & CAP0_INT_EN) // Check for interrupt on rising edge on CAP0 signal
    {
        // if(mcpwm_capture_signal_get_edge(MCPWM_UNIT_0,MCPWM_SELECT_CAP0))
        if (gpio_get_level(GPIO_RPM_CAP_IN) == true)
        {
            current_cap_value = mcpwm_capture_signal_get_value(MCPWM_UNIT_0, MCPWM_SELECT_CAP0); // get capture signal counter value
            evt.capture_signal = (current_cap_value - previous_cap_value) / (rtc_clk_apb_freq_get() / 1000000);
            evt.previous_cap_value = previous_cap_value;
            evt.current_cap_value = current_cap_value;
            evt.sel_cap_signal = MCPWM_SELECT_CAP0;

            /*如果旧的值比新的还大，那么不要传出去直接舍弃,这个情况属于数据溢出了*/
            if (previous_cap_value < current_cap_value)
            {
                xQueueSendFromISR(cap_queue, &evt, NULL);
            }
            previous_cap_value = current_cap_value;
        }
    }
    MCPWM0.int_clr.val = mcpwm_intr_status;
}

static IRAM_ATTR void rpm_ft_task(uint8_t g_rpm)
{
    static uint32_t rpm_fp[10] = {0};
    static uint8_t rpm_fp_i = 0;
    static uint32_t rpm_val = 0;
    rpm_fp[rpm_fp_i] = g_rpm;
    rpm_fp_i++;
    if (rpm_fp_i >= 10)
    {
        rpm_fp_i = 0;
    }
    rpm_val = 0;
    for (size_t i = 0; i < 10; i++)
    {
        rpm_val += rpm_fp[i];
    }
    update_RPM(rpm_val / 10);
}

static IRAM_ATTR void rpm_task(void *arg)
{
    capture_t evt;
    cap_queue = xQueueCreate(1, sizeof(capture_t)); // comment if you don't want to use capture module

    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_CAP_0, 15);

    mcpwm_capture_enable(MCPWM_UNIT_0, MCPWM_SELECT_CAP0, MCPWM_POS_EDGE, 0); // capture signal on rising edge, prescale = 0 i.e. 800,000,000 counts is equal to one second
    // enable interrupt, so each this a rising edge occurs interrupt is triggered
    MCPWM0.int_ena.val = CAP0_INT_EN;                                              // Enable interrupt on  CAP0, CAP1 and CAP2 signal
    mcpwm_isr_register(MCPWM_UNIT_0, isr_handler, NULL, ESP_INTR_FLAG_IRAM, NULL); // Set ISR Handler
    uint32_t rpm_over_timer = 50;                                                  //超时时间，取决于时间片大小
    uint32_t rpm = 0;
    while (1)
    {
        if(rpm_close_fg)
        {
            mcpwm_capture_disable(MCPWM_UNIT_0,MCPWM_SELECT_CAP0);
            break;
        }
        if (xQueueReceive(cap_queue, &evt, 100 / portTICK_PERIOD_MS))
        {
            if (evt.sel_cap_signal == MCPWM_SELECT_CAP0 && evt.capture_signal != 0) //查询是否是选定捕获中断传来的消息
            {
                rpm = 60 * 1000 * 1000 / evt.capture_signal;
                rpm_over_timer = 0;
                rpm = rpm + 0.5f;
                if (rpm <= 150)
                {
                    update_RPM(rpm);
                }
            }
        }
        else
        {
            if (rpm_over_timer < 50) // 50表示5秒后，rpm超时，会else里归零
            {
                rpm_over_timer++;
            }
            else
            {
                rpm = 0;
                update_RPM(rpm);
            }
        }
    }
    if (cap_queue != NULL)
    {
        free(cap_queue);
        cap_queue = NULL;
    }
    vTaskDelete(NULL);
}

void user_pull_motor_rpm_dev_init(void)
{
    xTaskCreate(rpm_task, "rpm_task", 4096, NULL, 15, NULL);
}
