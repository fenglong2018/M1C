idf_component_register(SRCS "User_BLE_alternator.c" "User_BLE_battery.c" "User_BLE_IPC.c" "User_BLE_FFF0.c" "User_BLE_DEV.c" "User_BLE.c" "User_BLE_FTMS.c"    
                    INCLUDE_DIRS .
                    REQUIRES console nvs_flash bt User_NVS
                    PRIV_REQUIRES BLE_FTMS User_sports User_LED User_mid)