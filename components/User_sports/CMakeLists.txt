idf_component_register(
    SRCS 
    "User_CAL_power_EF25EV.c" 
    "User_CAL_power_E80.c"
    "User_CAL_power_M1S.c"
    "User_CAL_power_M1M.c" 
    "User_CAL_power_M2M.c" 
    "User_CAL_power.c" 
    "User_sports.c"
                    INCLUDE_DIRS .
                    PRIV_REQUIRES User_Protocol User_BLE BLE_FTMS User_NVS)