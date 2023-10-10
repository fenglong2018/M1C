#include "ble_ftms_inDoorBike.h"
#include "User_BLE_IPC.h"
#include "esp_log.h"

#ifdef INDOORBIKE  //单车

static unsigned int g_byteOrder = 0x00;
#define HW_LITTLE_ENDIAN 0x01
#define HW_BIG_ENDIAN 0x02
#define HW_BLUETOOTH_ENDIAN HW_LITTLE_ENDIAN //蓝牙是小端序

#define INDOOR_BIKE_DATA_NO (11)
#define ROWER_DATA_NO (INDOOR_BIKE_DATA_NO+1)
#define CROSS_TRAINER_DATA_NO (INDOOR_BIKE_DATA_NO+2)
#define POWER_RANGE_NO (INDOOR_BIKE_DATA_NO+3)
static unsigned int g_pduSize = 20;			 //BLE默认一包发20

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

int IndoorBikeDataAccessNotify(INDOOR_BIKE_DATA_T* data)
{
	if (!data){
		return HW_ERR; // null pointer  空指针
	}
//	GetPDUSize();
	unsigned char indoorBikeData[INDOOR_BIKE_DATA_MAXLEN + 1] = {0};
	unsigned short flag 	= data->Flags;
	unsigned short sndFlag 	= 0;
	unsigned int dataLen 	= 0;
	unsigned int deltaLen 	= 0;
	unsigned int optionLen = 0;

	if (!(flag & INDOOR_BIKE_INSTANTANEOUS_SPEED)){   	//Instantaneous_Speed必须有
		return HW_ERR;
	}
	dataLen += sizeof(unsigned short); 					// 预留Flag位置
	for (int i = INDOOR_BIKE_FIELD_LOOP_INIT; i <= INDOOR_BIKE_FIELD_LOOP_END ; i++ ){
		switch(flag & (0x0001 << i)){
			case INDOOR_BIKE_INSTANTANEOUS_SPEED:
				CopyShortOrIntToBT((char*)&(data->Instantaneous_Speed), sizeof(unsigned short), (char*) (indoorBikeData + dataLen), sizeof(unsigned short));
				dataLen 	+= sizeof(unsigned short);
				optionLen 	= deltaLen = sizeof(unsigned short);
				break;
			case INDOOR_BIKE_AVERAGE_SPEED:
				CopyShortOrIntToBT((char*)&(data->Average_Speed), sizeof(unsigned short), (char*) (indoorBikeData + dataLen), sizeof(unsigned short));
				dataLen 	+= sizeof(unsigned short);
				deltaLen 	= sizeof(unsigned short);
				sndFlag 	|= INDOOR_BIKE_AVERAGE_SPEED;
				break;
			case INDOOR_BIKE_INSTANTANEOUS_CADENCE:
				CopyShortOrIntToBT((char*)&(data->Instantaneous_Cadence), sizeof(unsigned short), (char*) (indoorBikeData + dataLen), sizeof(unsigned short));
				dataLen 	+= sizeof(unsigned short);
				deltaLen 	= sizeof(unsigned short);
				sndFlag 	|= INDOOR_BIKE_INSTANTANEOUS_CADENCE;
				break;
			case INDOOR_BIKE_AVERAGE_CADENCE:
				CopyShortOrIntToBT((char*)&(data->Average_Cadence), sizeof(unsigned short),(char*) ( indoorBikeData + dataLen), sizeof(unsigned short));
				dataLen 	+= sizeof(unsigned short);
				deltaLen 	= sizeof(unsigned short);
				sndFlag 	|= INDOOR_BIKE_AVERAGE_CADENCE;
				break;
			case INDOOR_BIKE_TOTAL_DISTANCE:
				CopyShortOrIntToBT((char*)&(data->Total_Distance), sizeof(unsigned int), (char*) (indoorBikeData + dataLen), 3);
				dataLen 	+= 3;
				deltaLen 	= 3;
				sndFlag 	|= INDOOR_BIKE_TOTAL_DISTANCE;
				break;
			case INDOOR_BIKE_RESISTANCE_LEVEL:
				CopyShortOrIntToBT((char*)&(data->Resistance_Level), sizeof(unsigned short),(char*) ( indoorBikeData + dataLen), sizeof(unsigned short));
				dataLen 	+= sizeof(unsigned short);
				deltaLen 	= sizeof(unsigned short);
				sndFlag 	|= INDOOR_BIKE_RESISTANCE_LEVEL;
				break;
			case INDOOR_BIKE_INSTANTANEOUS_POWER:
				CopyShortOrIntToBT((char*)&(data->Instantaneous_Power), sizeof(unsigned short), (char*) (indoorBikeData + dataLen), sizeof(unsigned short));
				dataLen 	+= sizeof(unsigned short);
				deltaLen 	= sizeof(unsigned short);
				sndFlag 	|= INDOOR_BIKE_INSTANTANEOUS_POWER;
				break;
			case INDOOR_BIKE_AVERAGE_POWER:
				CopyShortOrIntToBT((char*)&(data->Average_Power), sizeof(unsigned short), (char*) (indoorBikeData + dataLen), sizeof(unsigned short));
				dataLen 	+= sizeof(unsigned short);
				deltaLen 	= sizeof(unsigned short);
				sndFlag 	|= INDOOR_BIKE_AVERAGE_POWER;
				break;
			case INDOOR_BIKE_EXPENDED_ENERGY:
				CopyShortOrIntToBT((char*)&(data->Total_Energy), sizeof(unsigned short), (char*) (indoorBikeData + dataLen), sizeof(unsigned short));
				dataLen 	+= sizeof(unsigned short);
				CopyShortOrIntToBT((char*)&(data->Energy_Per_Hour), sizeof(unsigned short), (char*) (indoorBikeData + dataLen), sizeof(unsigned short));
				dataLen 	+= sizeof(unsigned short);
				indoorBikeData[dataLen] = data->Energy_Per_Minute;
				dataLen 	+= 1;
				deltaLen = sizeof(unsigned short)* 2 + 1;
				sndFlag 	|= INDOOR_BIKE_EXPENDED_ENERGY;
				break;
			case INDOOR_BIKE_HEART_RATE:
				indoorBikeData[dataLen] = data->Heart_Rate;
				dataLen 	+= 1;
				deltaLen 	= 1;
				sndFlag 	|= INDOOR_BIKE_HEART_RATE;
				break;
			case INDOOR_BIKE_METABOLIC_EQUIVALENT:
				indoorBikeData[dataLen] = data->Metabolic_Equivalent;
				dataLen 	+= 1;
				deltaLen 	= 1;
				sndFlag 	|= INDOOR_BIKE_METABOLIC_EQUIVALENT;
				break;
			case INDOOR_BIKE_ELAPSED_TIME:
				CopyShortOrIntToBT((char*)&(data->Elapsed_Time), sizeof(unsigned short), (char*) (indoorBikeData + dataLen), sizeof(unsigned short));
				dataLen 	+= sizeof(unsigned short);
				deltaLen 	= sizeof(unsigned short);
				sndFlag 	|= INDOOR_BIKE_ELAPSED_TIME;
				break;
			case INDOOR_BIKE_REMAINING_TIME:
				CopyShortOrIntToBT((char*)&(data->Remaining_Time), sizeof(unsigned short), (char*) (indoorBikeData + dataLen), sizeof(unsigned short));
				dataLen 	+= sizeof(unsigned short);
				deltaLen 	= sizeof(unsigned short);
				sndFlag 	|= INDOOR_BIKE_REMAINING_TIME;
				break;
			default:
				break;
		}

		if (dataLen > g_pduSize){
			sndFlag |= 0x0001;  //more data
			sndFlag = sndFlag & (~(0x0001 << i));
			dataLen = dataLen - deltaLen;
			i--;
			CopyShortOrIntToBT((char*)&(sndFlag), sizeof(unsigned short), (char*) (indoorBikeData + optionLen), sizeof(unsigned short));
			dataLen = dataLen - optionLen;
			user_ipc_send_queue(BLE_IPC_SIID_INDOOR_BIKE,indoorBikeData + optionLen,dataLen);
			// 下一个PDU
			sndFlag = 0;
			dataLen = sizeof(unsigned short);
			CopyShortOrIntToBT((char*)&(data->Instantaneous_Speed), sizeof(unsigned short), (char*) (indoorBikeData + dataLen), sizeof(unsigned short));
			dataLen += sizeof(unsigned short);
			deltaLen = sizeof(unsigned short);
		}
	}
	CopyShortOrIntToBT((char*)&sndFlag, sizeof(unsigned short), (char*) (indoorBikeData), sizeof(unsigned short));
	user_ipc_send_queue(BLE_IPC_SIID_INDOOR_BIKE,indoorBikeData,dataLen);
	return HW_OK;
}
#endif