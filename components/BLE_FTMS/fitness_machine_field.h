/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: fitness_machine_field
 * Author: l00357454
 * Create: 2019-7-22
 */

#ifndef FITNESS_MACHINE_FIELD_H_
#define FITNESS_MACHINE_FIELD_H_
#include "stdint.h"

#define INDOORBIKE (1)
#define HWGYM_CROSSTRAINER (1)

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned long uint32;

/**
 * service和Characteristics的对应关系结构体
 *
 */
typedef struct CharacteristicsAttribute
{
	unsigned char serviceAttributeNum; // service下Characteristics的序列号
	unsigned char serviceUuidLen;	   // service的uuid长度
	unsigned char *serviceUuid;		   // service的uuid
	unsigned char permission;		   // Characteristics permission
	unsigned char uuidLen;			   // Characteristics的uuid
	unsigned char *uuid;			   // Characteristics的uuid
} fitness_machine_attribute_t;

/**
 * fitness_machine_feature的feature定义
 */
typedef enum
{
	average_speed = 0x00000001,					 //平均速度支持     *
	cadence_supported = 0x00000002,				 //支持步频        *
	total_distance = 0x00000004,				 //总距离           *
	inclination_supported = 0x00000008,			 //倾向支持
	elevation_gain = 0x00000010,				 //海拔
	pace_supported = 0x00000020,				 // 步    支持速度
	step_count = 0x00000040,					 //步数               *
	resistance_level = 0x00000080,				 //阻力位         *
	stride_count = 0x00000100,					 //步计数
	expended_energy = 0x00000200,				 //消耗的能源  *
	heart_rate_measurement = 0x00000400,		 //心率测量
	metabolic_equivalent = 0x00000800,			 //代谢当量
	elapsed_time = 0x00001000,					 //运行时间      *
	remaining_time = 0x00002000,				 //剩余时间
	power_measurement = 0x00004000,				 //功率测量       *
	force_on_belt_and_power_output = 0x00008000, //皮带受力及输出功率
	user_data_retention = 0x00010000,			 //用户数据保留
} fitness_machine_feature;

/**
 * target_setting_feature的feature定义
 * 小字节序
 */
typedef enum
{
	speed_target_setting = 0x00000001,
	inclination_target_setting = 0x00000002,
	resistance_target_setting = 0x00000004,
	power_target_setting = 0x00000008,
	heart_rate_target_setting = 0x00000010,
	targeted_expended_energy_configuration = 0x00000020,
	targeted_step_number_configuration = 0x00000040,
	targeted_stride_number_configuration = 0x00000080,
	targeted_distance_configuration = 0x00000100,
	targeted_training_time_configuration = 0x00000200,
	targeted_time_in_two_heart_rate_zones_configuration = 0x00000400,
	targeted_time_in_three_heart_rate_zones_configuration = 0x00000800,
	targeted_time_in_five_heart_rate_zones_configuration = 0x00001000,
	indoor_bike_simulation_parameters = 0x00002000,
	wheel_circumference_configuration = 0x00004000,
	spin_down_control = 0x00008000,
	targeted_cadence_configuration = 0x00010000,
} target_setting_feature;

/**
 *fitness_machine_feature的类型
 */
typedef enum
{
	fitness_machine_features = 0x01,
	target_setting_features = 0x02,
} fitness_machine_feature_properties;

#ifdef HWGYM_TREADMILL //跑步机
/**
 * treadmill 的field定义
 */
typedef enum
{
	instantaneous_speed = 0x0000, // 必须支持，为0代表支持瞬时速度
	treadmill_average_speed = 0x0002,
	treadmill_total_distance = 0x0004,
	inclination_and_ramp_angle_setting = 0x0008,
	treadmill_elevation_gain = 0x0010,
	instantaneous_pace = 0x0020,
	average_pace = 0x0040,
	treadmill_expended_energy = 0x0080,
	heart_rate = 0x0100,
	treadmill_metabolic_equivalent = 0x0200,
	treadmill_elapsed_time = 0x0400,
	treadmill_remaining_time = 0x0800,
	treadmill_force_on_belt_and_power_output = 0x1000,
} treadmill_data_field;

/**
 *
 */
typedef struct
{
	unsigned short flags;
	unsigned short instantaneousSpeed; // 必须支持,单位：0.01km/h
	unsigned short averageSpeed;
	unsigned int totalDistance; // 必须支持，单位:米
	signed short inclination;
	signed short rampAngle;
	unsigned short positiveElevationGain;
	unsigned short negativeElevationGain;
	unsigned short totalEnergy;	   // 必须支持，单位kcal
	unsigned short energyPerHour;  // 必须填写，不支持填0
	unsigned char energyPerMinute; // 必须填写，不支持填0
	unsigned char heartRate;
	unsigned short elapsedTime; // 必须支持
} treadmill_Data_t;

#define TREADMILL_DATA_LEN 11 // data max 18

typedef struct
{
	unsigned int len; // data max 18
	unsigned int data[TREADMILL_DATA_LEN];
} treadmill_data_t;
#endif

#define HW_ERR 1
#define HW_OK 0

#ifdef INDOORBIKE //单车
/**
 * 室内单车数据定义：Field
 */
typedef enum
{
	INDOOR_BIKE_INSTANTANEOUS_SPEED = 0x0001,	// 瞬时速率    BIT0  ++++++++++++++
	INDOOR_BIKE_AVERAGE_SPEED = 0x0002,			//平均速 BIT1       +++
	INDOOR_BIKE_INSTANTANEOUS_CADENCE = 0x0004, //瞬时节奏           ++++++++++++++
	INDOOR_BIKE_AVERAGE_CADENCE = 0x0008,		//平均节奏           //+++
	INDOOR_BIKE_TOTAL_DISTANCE = 0x0010,		//总距离               ++++++++++++++
	INDOOR_BIKE_RESISTANCE_LEVEL = 0x0020,		//阻力位            ++++++++++++++
	INDOOR_BIKE_INSTANTANEOUS_POWER = 0x0040,	// 瞬时功率               ++++++++++++++
	INDOOR_BIKE_AVERAGE_POWER = 0x0080,			//平均功率             +++++++++++++++
	INDOOR_BIKE_EXPENDED_ENERGY = 0x0100,		//消耗能源          ++++++++++++++
	INDOOR_BIKE_HEART_RATE = 0x0200,			//心率
	INDOOR_BIKE_METABOLIC_EQUIVALENT = 0x0400,	//代谢当量
	INDOOR_BIKE_ELAPSED_TIME = 0x0800,			// 运行时间             ++++++++++++++
	INDOOR_BIKE_REMAINING_TIME = 0x1000,		//  剩余时间 BIT12
} INDOOR_BIKE_FIELD;

enum
{
	INDOOR_BIKE_FIELD_LOOP_INIT = 0,
	INDOOR_BIKE_FIELD_LOOP_END = 12
};

/**
 * 室内单车数据定义：Data
 */
typedef struct
{
	unsigned short Flags;				  // Please refer to INDOOR_BIKE_FIELD
	unsigned short Instantaneous_Speed;	  // INDOOR_BIKE_INSTANTANEOUS_SPEED			0.01 Kilometer per hour
	unsigned short Average_Speed;		  // INDOOR_BIKE_AVERAGE_SPEED				0.01 Kilometer per hour
	unsigned short Instantaneous_Cadence; // INDOOR_BIKE_INSTANTANEOUS_CADENCE		0.5 /min
	unsigned short Average_Cadence;		  // INDOOR_BIKE_AVERAGE_CADENCE				0.5 /min
	uint32 Total_Distance;				  // INDOOR_BIKE_TOTAL_DISTANCE	use 24bits	M
	signed short Resistance_Level;		  // INDOOR_BIKE_RESISTANCE_LEVEL				Unitless
	signed short Instantaneous_Power;	  // INDOOR_BIKE_INSTANTANEOUS_POWER			Watts
	signed short Average_Power;			  // INDOOR_BIKE_AVERAGE_POWER			 	Watts
	unsigned short Total_Energy;		  // INDOOR_BIKE_EXPENDED_ENERGY				Kilo Calorie
	unsigned short Energy_Per_Hour;		  // INDOOR_BIKE_EXPENDED_ENERGY				Kilo Calorie
	unsigned char Energy_Per_Minute;	  // INDOOR_BIKE_EXPENDED_ENERGY				Kilo Calorie
	unsigned char Heart_Rate;			  // INDOOR_BIKE_HEART_RATE					Beats/minute
	unsigned char Metabolic_Equivalent;	  // INDOOR_BIKE_METABOLIC_EQUIVALENT			0.1 Metabolic Equivalent
	unsigned short Elapsed_Time;		  // INDOOR_BIKE_ELAPSED_TIME					Second
	unsigned short Remaining_Time;		  // INDOOR_BIKE_REMAINING_TIME				Second
} INDOOR_BIKE_DATA_T;
struct INDOOR_BIKE_DATA_event
{
	unsigned short Flags;				  // Please refer to INDOOR_BIKE_FIELD
	unsigned short Instantaneous_Speed;	  // INDOOR_BIKE_INSTANTANEOUS_SPEED			0.01 Kilometer per hour
	unsigned short Average_Speed;		  // INDOOR_BIKE_AVERAGE_SPEED				0.01 Kilometer per hour
	unsigned short Instantaneous_Cadence; // INDOOR_BIKE_INSTANTANEOUS_CADENCE		0.5 /min
	unsigned short Average_Cadence;		  // INDOOR_BIKE_AVERAGE_CADENCE				0.5 /min
	uint32 Total_Distance;				  // INDOOR_BIKE_TOTAL_DISTANCE	use 24bits	M
	signed short Resistance_Level;		  // INDOOR_BIKE_RESISTANCE_LEVEL				Unitless
	signed short Instantaneous_Power;	  // INDOOR_BIKE_INSTANTANEOUS_POWER			Watts
	signed short Average_Power;			  // INDOOR_BIKE_AVERAGE_POWER			 	Watts
	unsigned short Total_Energy;		  // INDOOR_BIKE_EXPENDED_ENERGY				Kilo Calorie
	unsigned short Energy_Per_Hour;		  // INDOOR_BIKE_EXPENDED_ENERGY				Kilo Calorie
	unsigned char Energy_Per_Minute;	  // INDOOR_BIKE_EXPENDED_ENERGY				Kilo Calorie
	unsigned char Heart_Rate;			  // INDOOR_BIKE_HEART_RATE					Beats/minute
	unsigned char Metabolic_Equivalent;	  // INDOOR_BIKE_METABOLIC_EQUIVALENT			0.1 Metabolic Equivalent
	unsigned short Elapsed_Time;		  // INDOOR_BIKE_ELAPSED_TIME					Second
	unsigned short Remaining_Time;		  // INDOOR_BIKE_REMAINING_TIME				Second
};
#define INDOOR_BIKE_DATA_MAXLEN sizeof(INDOOR_BIKE_DATA_T)

typedef struct
{
	unsigned short len; // the effective length of the following data
	unsigned char data[INDOOR_BIKE_DATA_MAXLEN];
} INDOOR_BIKE_DATA_BUFF_T;
#endif

#ifdef HWGYM_ROWER //划船机
/**
 * 划船机数据定义：Field
 */
typedef enum
{
	ROWER_STROKE_RATE_STROKE_COUNT = 0x0001,
	ROWER_AVERAGE_STROKE_RATE = 0x0002,
	ROWER_TOTAL_DISTANCE = 0x0004,
	ROWER_INSTANTANEOUS_PACE = 0x0008,
	ROWER_AVERAGE_PACE = 0x0010,
	ROWER_INSTANTANEOUS_POWER = 0x0020,
	ROWER_AVERAGE_POWER = 0x0040,
	ROWER_RESISTANCE_LEVEL = 0x0080,
	ROWER_EXPENDED_ENERGY = 0x0100,
	ROWER_HEART_RATE = 0x0200,
	ROWER_METABOLIC_EQUIVALENT = 0x0400,
	ROWER_ELAPSED_TIME = 0x0800,
	ROWER_REMAINING_TIME = 0x1000,
} ROWER_FIELD;

enum
{
	ROWER_FIELD_LOOP_INIT = 0,
	ROWER_FIELD_LOOP_END = 12
};

/**
 * 划船机数据定义：Data
 */
typedef struct
{
	unsigned short Flags;				// Please refer to ROWER_FIELD
	unsigned char Stroke_Rate;			// ROWER_STROKE_RATE_STROKE_COUNT  	0.5 stroke/min  //桨频
	unsigned short Stroke_Count;		// ROWER_STROKE_RATE_STROKE_COUNT	1 Unitless
	unsigned char Average_Stroke_Rate;	// ROWER_AVERAGE_STROKE_RATE		0.5 /min
	uint32 Total_Distance;				// ROWER_TOTAL_DISTANCE	use 24bits	M
	unsigned short Instantaneous_Pace;	// ROWER_INSTANTANEOUS_PACE			Second
	unsigned short Average_Pace;		// ROWER_AVERAGE_PACE				Second
	signed short Instantaneous_Power;	// ROWER_INSTANTANEOUS_POWER		Watts
	signed short Average_Power;			// ROWER_AVERAGE_POWER			 	Watts
	signed short Resistance_Level;		// ROWER_RESISTANCE_LEVEL			Unitless
	unsigned short Total_Energy;		// ROWER_EXPENDED_ENERGY			Kilo Calorie
	unsigned short Energy_Per_Hour;		// ROWER_EXPENDED_ENERGY			Kilo Calorie
	unsigned char Energy_Per_Minute;	// ROWER_EXPENDED_ENERGY			Kilo Calorie
	unsigned char Heart_Rate;			// ROWER_HEART_RATE					Beats/minute
	unsigned char Metabolic_Equivalent; // ROWER_METABOLIC_EQUIVALENT		0.1 Metabolic Equivalent
	unsigned short Elapsed_Time;		// ROWER_ELAPSED_TIME				Second
	unsigned short Remaining_Time;		// ROWER_REMAINING_TIME				Second
	//	unsigned short  rpm;     //划船机  速度
	//	unsigned char   DIR;     //划船机  收拉桨方向
} ROWER_DATA_T;
/*
划船机 收拉桨 方向
*/
typedef enum
{
	ROWER_INVALID = 0,			//无效
	ROWER_PULLING_THE_OARS = 1, //拉桨
	ROWER_The_OARS = 2,			// 收桨
} ROWER_DIR_FIELD;
#define ROWER_DATA_MAXLEN sizeof(ROWER_DATA_T)

typedef struct
{
	unsigned short len; // the effective length of the following data
	unsigned char data[ROWER_DATA_MAXLEN];
} ROWER_DATA_BUFF_T;

#endif
#ifdef HWGYM_CROSSTRAINER //交叉训练器
/**
 * 椭圆机数据定义: Field
 */
typedef enum
{
	CROSS_TRAINER_INSTANTANEOUS_SPEED = 0x000001, // *
	CROSS_TRAINER_AVERAGE_SPEED = 0x000002,		  //*
	CROSS_TRAINER_TOTAL_DISTANCE = 0x000004,	  //*
	CROSS_TRAINER_STEP_COUNT = 0x000008,		  //*
	CROSS_TRAINER_STRIDE_COUNT = 0x000010,		  //*
	CROSS_TRAINER_ELEVATION_GAIN = 0x000020,
	CROSS_TRAINER_INCLINATION_RAMP_ANGLE_SETTING = 0x000040,
	CROSS_TRAINER_RESISTANCE_LEVEL = 0x000080,	  //*
	CROSS_TRAINER_INSTANTANEOUS_POWER = 0x000100, //*
	CROSS_TRAINER_AVERAGE_POWER = 0x000200,		  //*
	CROSS_TRAINER_EXPENDED_ENERGY = 0x000400,	  //*
	CROSS_TRAINER_HEART_RATE = 0x000800,
	CROSS_TRAINER_METABOLIC_EQUIVALENT = 0x001000,
	CROSS_TRAINER_ELAPSED_TIME = 0x002000, //*
	CROSS_TRAINER_REMAINING_TIME = 0x004000,
	CROSS_TRAINER_MOVEMENT_DIRECTION = 0x008000,
} CROSS_TRAINER_FIELD;

enum
{
	CROSS_TRAINER_FIELD_LOOP_INIT = 0,
	CROSS_TRAINER_FIELD_LOOP_END = 15
};

/**
 * 椭圆机数据定义: Data
 */

typedef struct
{
	uint32 Flags;							// Please refer to CROSS_TRAINER_FIELD  use 24bits
	unsigned short Instantaneous_Speed;		// CROSS_TRAINER_INSTANTANEOUS_SPEED  				0.01 KM/Hour
	unsigned short Average_Speed;			// CROSS_TRAINER_AVERAGE_SPEED						0.01 KM/Hour
	uint32 Total_Distance;					// CROSS_TRAINER_TOTAL_DISTANCE	use 24bits			M
	unsigned short Step_Per_Minute;			// CROSS_TRAINER_STEP_COUNT							Step/Min
	unsigned short Average_Step_Rate;		// CROSS_TRAINER_STEP_COUNT							Step/Min
	unsigned short Stride_Count;			// CROSS_TRAINER_STRIDE_COUNT						0.1 Unitless
	unsigned short Positive_Elevation_Gain; // CROSS_TRAINER_ELEVATION_GAIN	M   海拔
	unsigned short Negative_Elevation_Gain; // CROSS_TRAINER_ELEVATION_GAIN	M   海拔
	signed short Inclination;				// CROSS_TRAINER_INCLINATION_RAMP_ANGLE_SETTING		0.1 %   倾角
	signed short Ramp_Angle_Setting;		// CROSS_TRAINER_INCLINATION_RAMP_ANGLE_SETTING		0.1 Degree
	signed short Resistance_Level;			// CROSS_TRAINER_RESISTANCE_LEVEL					0.1 Unitless
	signed short Instantaneous_Power;		// CROSS_TRAINER_INSTANTANEOUS_POWER				Watts
	signed short Average_Power;				// CROSS_TRAINER_AVERAGE_POWER						Watts
	unsigned short Total_Energy;			// CROSS_TRAINER_EXPENDED_ENERGY					Kilo Calorie
	unsigned short Energy_Per_Hour;			// CROSS_TRAINER_EXPENDED_ENERGY					Kilo Calorie
	unsigned char Energy_Per_Minute;		// CROSS_TRAINER_EXPENDED_ENERGY					Kilo Calorie
	unsigned char Heart_Rate;				// CROSS_TRAINER_HEART_RATE							Beats/minute
	unsigned char Metabolic_Equivalent;		// CROSS_TRAINER_METABOLIC_EQUIVALENT				0.1 Metabolic Equivalent
	unsigned short Elapsed_Time;			// CROSS_TRAINER_ELAPSED_TIME						Second
	unsigned short Remaining_Time;			// CROSS_TRAINER_REMAINING_TIME						Second
} CROSS_TRAINER_DATA_T;

#define CROSS_TRAINER_DATA_MAXLEN sizeof(CROSS_TRAINER_DATA_T)

typedef struct
{
	unsigned short len; // the effective length of the following data
	unsigned char data[CROSS_TRAINER_DATA_MAXLEN];
} CROSS_TRAINER_BUFF_T;

#endif

typedef enum
{
	other = 0x00, // 其他未知状态
	idle = 0x01,  // 空闲,必须支持
	warming_Up = 0x02,
	low_intensity_interval = 0x03,
	high_intensity_interval = 0x04,
	recovery_interval = 0x05,
	isometric = 0x06,
	heart_rate_control = 0x07,
	fitness_test = 0x08,
	speed_outside_of_control_region_low = 0x09,
	speed_outside_of_control_region_high = 0x0A,
	cool_down = 0x0B,
	watt_control = 0x0C,
	quick_start = 0x0D,	 // 快速开始,必须支持
	pre_workout = 0x0E,	 // 开启前读秒时状态,必须支持
	post_workout = 0x0F, // 结束后读秒时状态,必须支持
} training_status;

// typedef struct {
//     unsigned int len;
//     unsigned char flags;
//     unsigned char training_status;
// }training_status_t;

// typedef struct {
//     unsigned short minSpeed;
//     unsigned short maxSpeed;
//     unsigned short minIncrement;
// }speed_range_t;

// typedef struct {
//     signed short minInclination;
//     signed short maxInclination;
//     unsigned short minIncrement;
// }inclination_range_t;

// typedef struct {
//     signed short minLevel;
//     signed short maxLevel;
//     unsigned short minIncrement;
// }resistance_level_range_t;

// typedef struct {
//     unsigned char minHeartRate;
//     unsigned char maxHeartRate;
//     unsigned char minIncrement;
// }heart_rate_range_t;

/**
 * Fitness Machine Control Point的行为枚举定义
 */
typedef enum
{
	request_control = 0x00, // 请求控制权限
	reset = 0x01,
	set_target_speed = 0x02,			//设定目标的速度
	set_target_inclination = 0x03,		//设定目标的倾向
	set_target_resistance_level = 0x04, //设置 阻力档位
	start = 0x07,
	stop = 0x08,
	start_and_stop_control = 0X09, //  YESOUL 增加,跑步机以外的产品，设备自启停控制开关   00：打开;01:关闭
	resume = 0X15,				   // 华为扩展
	pause = 0x16,				   // 华为扩展
	response_code = 0x80,
} fitness_machine_control_point_operation_code;

/**
 * Fitness Machine Control Point 的命令
 */
typedef struct
{
	unsigned char opCode;
	unsigned char parameterLength; //参数长度
} fitness_machine_control_point_t;

typedef union
{
	unsigned short targetSpeed;		//目标飞行速度
	signed short targetInclination; //目标倾向
	char targetResistanceLevel;		//目标阻力
	char response_state;
} control_point_target_union;
/**
 * Fitness Machine Control Point response的resultCode枚举定义
 */
typedef enum
{
	success = 0x01,				  //成功
	op_code_not_supported = 0x02, //不支持操作
	invalid_parameter = 0x03,	  //参数错误
	operation_failed = 0x04,	  //操作失败
	control_not_permitted = 0x05, //不允许控制
} fitness_machine_control_point_response_resultCode;

/**
 * Fitness Machine Control Point 的执行结果答复
 */
typedef struct
{
	unsigned char opCode;
	unsigned char resultCode;
	int parameter;
} fitness_machine_control_point_response_t;

/**
 * Fitness Machine status的行为枚举定义
 */
typedef enum
{
	pause_by_user = 0x00,					// 用户暂停
	status_reset = 0x01,					// 参数：无      状态重置
	stop_by_user = 0x02,					// 用户停止 参数：无
	stop_by_safety_key = 0x03,				// 参数：无
	start_by_user = 0x04,					// 参数：无   由用户启动或恢复健身器
	target_speed_changed = 0x05,			// 参数：uint16
	target_incline_changed = 0x06,			// 参数：sint16
	Target_resistance_level_changed = 0x07, // 参数：uint8
	resume_by_user = 0x16,					// 用户resume
	control_permission_lost = 0xFF,			// Fitness Machine发送该指令后，不再接收Fitness Machine Control Point
} fitness_machine_status_operation_code;

//#define STATUS_DATA_LEN    		2   // data max 2
// typedef struct {
//	unsigned char code;
//    unsigned int len; // data max 18
//    unsigned char data[STATUS_DATA_LEN];
//}fitness_machine_status_t;

// typedef enum
//{
//	unLock_Code =0x01,
//	extension_heart_rate=0x02,
//	total_energy=0x04,
//	dynamic_enerty=0x08,
//	extension_step_count=0x10,
// }fitness_extension_data_flag;

/**
 *
 */
typedef struct
{
	unsigned short flags;
	char unLockCode[6];
	unsigned char heartRate;
	unsigned short totalEnergy;
	unsigned short dynamicEnerty;
	unsigned short steps;
	unsigned short remaining_time;
	char RGBColorTable[3];
} huawei_fitness_extension_data_t;

typedef enum
{
	verify_failed = 0x00,
	verify_success = 0xFF,
} fitness_verify_code;
typedef enum
{
	open_control = 0x00,  //打开
	close_control = 0xFF, //关闭
} start_stop_control_code;

#endif /* FITNESS_MACHINE_FIELD_H_ */
