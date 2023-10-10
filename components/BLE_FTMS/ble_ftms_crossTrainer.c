#include "ble_ftms_crossTrainer.h"
#include "User_BLE_IPC.h"
#include "esp_log.h"

static unsigned int g_byteOrder = 0x00;
#define HW_LITTLE_ENDIAN 0x01
#define HW_BIG_ENDIAN 0x02
#define HW_BLUETOOTH_ENDIAN HW_LITTLE_ENDIAN //蓝牙是小端序

#define INDOOR_BIKE_DATA_NO (11)
#define ROWER_DATA_NO (INDOOR_BIKE_DATA_NO+1)
#define CROSS_TRAINER_DATA_NO (INDOOR_BIKE_DATA_NO+2)
#define POWER_RANGE_NO (INDOOR_BIKE_DATA_NO+3)
static const unsigned int g_pduSize = 20;			 //BLE默认一包发20

static inline void GetByteOrder(){

	if (g_byteOrder){
		return;
	}

	unsigned short hostData = 0x1234;
	unsigned char* orderData = (unsigned char*)&hostData;
	if (orderData[0] == 0x34){
		g_byteOrder = HW_LITTLE_ENDIAN;
	} else {
		g_byteOrder = HW_BIG_ENDIAN;
	}
	return;
}

static inline void CopyShortOrIntToBT(char* hostData, int hostLen, char* btData, int btDataLen)
{
	int i;

	GetByteOrder();

	if (g_byteOrder == HW_BLUETOOTH_ENDIAN){
		for (i = 0; i < btDataLen && i < hostLen; i++){
			btData[i] = hostData[i];
		}
	} else {
		for (i = 0; i < btDataLen && i < hostLen; i++){
			btData[i] = hostData[hostLen-i-1];
		}
	}

	return;
}

int CrossTrainerDataAccessNotify(CROSS_TRAINER_DATA_T* data){

	if (!data){ESP_LOGE("tsg", "s1");
		return HW_ERR; // null pointer
	}

	unsigned char crossTrainerData[CROSS_TRAINER_DATA_MAXLEN + 1] = {0};
	unsigned int flag = data->Flags;
	unsigned int sndFlag = 0;
	unsigned int dataLen = 0;
	unsigned int deltaLen = 0;
	unsigned int optionLen = 0;

	if (!(flag & CROSS_TRAINER_INSTANTANEOUS_SPEED)){   //Instantaneous_Speed必须有
		return HW_ERR;
	}

	dataLen += 3; // 预留Flag位置 24bits

	for (int i = CROSS_TRAINER_FIELD_LOOP_INIT; i <= CROSS_TRAINER_FIELD_LOOP_END ; i++ ){
		switch(flag & (0x00000001 << i)){
			case CROSS_TRAINER_INSTANTANEOUS_SPEED:
				CopyShortOrIntToBT((char*) &(data->Instantaneous_Speed), sizeof(unsigned short),(char*)(crossTrainerData + dataLen), sizeof(unsigned short));
				dataLen += sizeof(unsigned short);
				optionLen = deltaLen = sizeof(unsigned short);
				break;
			case CROSS_TRAINER_AVERAGE_SPEED:
				CopyShortOrIntToBT((char*)&(data->Average_Speed), sizeof(unsigned short), (char*) (crossTrainerData + dataLen), sizeof(unsigned short));
				dataLen += sizeof(unsigned short);
				deltaLen = sizeof(unsigned short);
				sndFlag |= CROSS_TRAINER_AVERAGE_SPEED;
				break;
			case CROSS_TRAINER_TOTAL_DISTANCE:
				CopyShortOrIntToBT((char*)&(data->Total_Distance), sizeof(unsigned int), (char*) (crossTrainerData + dataLen), 3);
				dataLen += 3;
				deltaLen = 3;
				sndFlag |= CROSS_TRAINER_TOTAL_DISTANCE;
				break;
			case CROSS_TRAINER_STEP_COUNT:
				CopyShortOrIntToBT((char*)&(data->Step_Per_Minute), sizeof(unsigned short), (char*) (crossTrainerData + dataLen), sizeof(unsigned short));
				dataLen += sizeof(unsigned short);
				CopyShortOrIntToBT((char*)&(data->Average_Step_Rate), sizeof(unsigned short), (char*) (crossTrainerData + dataLen), sizeof(unsigned short));
				dataLen += sizeof(unsigned short);
				deltaLen = sizeof(unsigned short) * 2;
				sndFlag |= CROSS_TRAINER_STEP_COUNT;
				break;
			case CROSS_TRAINER_STRIDE_COUNT:
				CopyShortOrIntToBT((char*)&(data->Stride_Count), sizeof(unsigned short), (char*) (crossTrainerData + dataLen), sizeof(unsigned short));
				dataLen += sizeof(unsigned short);
				deltaLen = sizeof(unsigned short);
				sndFlag |= CROSS_TRAINER_STRIDE_COUNT;
				break;
			case CROSS_TRAINER_ELEVATION_GAIN:
				CopyShortOrIntToBT((char*)&(data->Positive_Elevation_Gain), sizeof(unsigned short), (char*) (crossTrainerData + dataLen), sizeof(unsigned short));
				dataLen += sizeof(unsigned short);
				CopyShortOrIntToBT((char*)&(data->Negative_Elevation_Gain), sizeof(unsigned short), (char*) (crossTrainerData + dataLen), sizeof(unsigned short));
				dataLen += sizeof(unsigned short);
				deltaLen = sizeof(unsigned short) * 2;
				sndFlag |= CROSS_TRAINER_ELEVATION_GAIN;
				break;
			case CROSS_TRAINER_INCLINATION_RAMP_ANGLE_SETTING:
				CopyShortOrIntToBT((char*)&(data->Inclination), sizeof(unsigned short), (char*) (crossTrainerData + dataLen), sizeof(unsigned short));
				dataLen += sizeof(unsigned short);
				CopyShortOrIntToBT((char*)&(data->Ramp_Angle_Setting), sizeof(unsigned short), (char*) (crossTrainerData + dataLen), sizeof(unsigned short));
				dataLen += sizeof(unsigned short);
				deltaLen = sizeof(unsigned short) * 2;
				sndFlag |= CROSS_TRAINER_INCLINATION_RAMP_ANGLE_SETTING;
				break;
			case CROSS_TRAINER_RESISTANCE_LEVEL:
				CopyShortOrIntToBT((char*)&(data->Resistance_Level), sizeof(unsigned short), (char*) (crossTrainerData + dataLen), sizeof(unsigned short));
				dataLen += sizeof(unsigned short);
				deltaLen = sizeof(unsigned short);
				sndFlag |= CROSS_TRAINER_RESISTANCE_LEVEL;
				break;
			case CROSS_TRAINER_INSTANTANEOUS_POWER:
				CopyShortOrIntToBT((char*)&(data->Instantaneous_Power), sizeof(unsigned short), (char*) (crossTrainerData + dataLen), sizeof(unsigned short));
				dataLen += sizeof(unsigned short);
				deltaLen = sizeof(unsigned short);
				sndFlag |= CROSS_TRAINER_INSTANTANEOUS_POWER;
				break;
			case CROSS_TRAINER_AVERAGE_POWER:
				CopyShortOrIntToBT((char*)&(data->Average_Power), sizeof(unsigned short), (char*) (crossTrainerData + dataLen), sizeof(unsigned short));
				dataLen += sizeof(unsigned short);
				deltaLen = sizeof(unsigned short);
				sndFlag |= CROSS_TRAINER_AVERAGE_POWER;
				break;
			case CROSS_TRAINER_EXPENDED_ENERGY:
				CopyShortOrIntToBT((char*)&(data->Total_Energy), sizeof(unsigned short),(char*) ( crossTrainerData + dataLen), sizeof(unsigned short));
				dataLen += sizeof(unsigned short);
				CopyShortOrIntToBT((char*)&(data->Energy_Per_Hour), sizeof(unsigned short), (char*) (crossTrainerData + dataLen), sizeof(unsigned short));
				dataLen += sizeof(unsigned short);
				crossTrainerData[dataLen] = data->Energy_Per_Minute;
				dataLen += 1;
				deltaLen = sizeof(unsigned short)* 2 + 1;
				sndFlag |= CROSS_TRAINER_EXPENDED_ENERGY;
				break;
			case CROSS_TRAINER_HEART_RATE:
				crossTrainerData[dataLen] = data->Heart_Rate;
				dataLen += 1;
				deltaLen = 1;
				sndFlag |= CROSS_TRAINER_HEART_RATE;
				break;
			case CROSS_TRAINER_METABOLIC_EQUIVALENT:
				crossTrainerData[dataLen] = data->Metabolic_Equivalent;
				dataLen += 1;
				deltaLen = 1;
				sndFlag |= CROSS_TRAINER_METABOLIC_EQUIVALENT;
				break;
			case CROSS_TRAINER_ELAPSED_TIME:
				CopyShortOrIntToBT((char*)&(data->Elapsed_Time), sizeof(unsigned short), (char*) (crossTrainerData + dataLen), sizeof(unsigned short));
				dataLen += sizeof(unsigned short);
				deltaLen = sizeof(unsigned short);
				sndFlag |= CROSS_TRAINER_ELAPSED_TIME;
				break;
			case CROSS_TRAINER_REMAINING_TIME:
				CopyShortOrIntToBT((char*)&(data->Remaining_Time), sizeof(unsigned short), (char*) (crossTrainerData + dataLen), sizeof(unsigned short));
				dataLen += sizeof(unsigned short);
				deltaLen = sizeof(unsigned short);
				sndFlag |= CROSS_TRAINER_REMAINING_TIME;
				break;
			case CROSS_TRAINER_MOVEMENT_DIRECTION:
				sndFlag |= CROSS_TRAINER_MOVEMENT_DIRECTION;
				break;
			default:
				break;
		}

		if (dataLen > g_pduSize){
			sndFlag |= 0x00000001;  //more data
			sndFlag = sndFlag & (~(0x00000001 << i));
			dataLen = dataLen - deltaLen;
			i--;

			CopyShortOrIntToBT((char*)&(sndFlag), sizeof(unsigned int), (char*) (crossTrainerData + optionLen), 3);
			dataLen = dataLen - optionLen;
			user_ipc_send_queue(BLE_IPC_SIID_INDOOR_BIKE,crossTrainerData+ optionLen,dataLen);
      		// E30S_send(crossTrainerData + optionLen,dataLen);
			// SendUartDataToApp(crossTrainerData + optionLen,dataLen,CROSS_TRAINER_DATA_NO);

			// 下一个PDU
			sndFlag = 0;
			dataLen = 3; // 预留Flag位置 24bits
			CopyShortOrIntToBT((char*)&(data->Instantaneous_Speed), sizeof(unsigned short), (char*) (crossTrainerData + dataLen), sizeof(unsigned short));
			dataLen += sizeof(unsigned short);
			deltaLen = sizeof(unsigned short);
		}
	}

	CopyShortOrIntToBT((char*)&sndFlag, sizeof(unsigned int), (char*) (crossTrainerData), 3);
	user_ipc_send_queue(BLE_IPC_SIID_INDOOR_BIKE,crossTrainerData,dataLen);
//   E30S_send(crossTrainerData,dataLen);
	
	return HW_OK;
}
