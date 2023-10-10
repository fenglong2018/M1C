idf_component_register(SRCS 
                "User_dev_ADC.c" 
                "User_dev_Vspi.c"
                "User_dev_uart2.c" 
                "User_dev_uart1.c"
                "User_dev_pwm.c"
                INCLUDE_DIRS "include"
                PRIV_REQUIRES esp_adc_cal)