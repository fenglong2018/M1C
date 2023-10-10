#include "User_pull_motor.h"
#include "../User_Protocol/User_Protocol.h"
#include "../User_dev/include/User_dev_uart2.h"
#include "esp_log.h"
#include "User_BLE_IPC.h"
#define USER_PULL_TAG "USER_PULL"

static enum {
	PULL_INIT,		 // 初始状态
	PULL_RST,		 // 复位电机位置
	PULL_COUNT,		 // 设定conut
	PULL_RPM,		 // 获取rpm
	PULL_BATT,		 // 获取电池电压
	PULL_POWER,		 // 读取发电功率
	PULL_READ_COUNT, // 读取当前conut
	PULL_READ_ERR,	 // 读取报错代码
};

static struct PULL_STRUCT{
	uint16_t zrro_fg;
}pull_list;

static uint8_t close_fg = 0;
static uint8_t rpm_err_cnt = 0;
static uint32_t data_send_fg = 0; //只ll

static int32_t get_data_update(void)
{
	static uint32_t data_update_err_cnt = 0;

	int ret = 0;
	if (data_send_fg == 0)
	{
		ret = 1;
	}
	else
	{
		if (data_update_err_cnt < 10)
		{
			data_update_err_cnt++;
		}
		else
		{
			data_update_err_cnt = 0;
			data_send_fg = 0;
		}
	}

	return ret;
}

/********************************************************************************************************
 * @brief	计算CRC校验码
 * @param 	计算的缓冲区指针
 * @param 	需要计算的长度
 * @return	crc
********************************************************************************************************/
static uint16_t calCrcBitByBit(uint8_t *pSrcBuf, uint8_t len)
{
	uint16_t crcReg;
	uint8_t i, j;

	j = 0;
	crcReg = 0xFFFF;

	while (len-- != 0)
	{
		uint8_t temp;
		temp = pSrcBuf[j];

		for (i = 0x01; i != 0; i = i << 1)
		{
			if ((crcReg & 0x0001) != 0)
			{
				crcReg >>= 1;
				crcReg ^= 0x8408;
			}
			else
			{
				crcReg >>= 1;
			}

			if ((temp & i) != 0)
			{
				crcReg ^= 0x8408;
			}
		}
		j++;
	}

	return crcReg;
}

static void pull_motor_set_rst(void)
{
	uint8_t t[] = {0xfe,0x20,0x2b,0x03,0x00,0x87,0x8c,0xff};
	uart_write_bytes(UART_NUM_2, (const char *) t, sizeof(t));
}

static void pull_motor_write_par(uint8_t cmd,uint16_t par)
{
	uint8_t send_buff[20];
	uint8_t send_buff_len = 0;;
	send_buff[(send_buff_len)++] = FRAME_HEADER;
	send_buff[(send_buff_len)++] = FRAME_WHITE;
	send_buff[(send_buff_len)++] = cmd;
	send_buff[(send_buff_len)++] = par;
	send_buff[(send_buff_len)++] = par>>8;
	uint16_t crc = calCrcBitByBit(&(send_buff[1]),send_buff_len-1);

	if((uint8_t)(crc)>=0XFD)
	{
		send_buff[(send_buff_len)++]=0XFD;
		send_buff[(send_buff_len)++]=(uint8_t)crc-0XFD;
	}else{
		send_buff[(send_buff_len)++]=crc;
	}
	if((uint8_t)(crc>>8)>=0XFD)
	{
		send_buff[(send_buff_len)++]=0XFD;
		send_buff[(send_buff_len)++]=(uint8_t)(crc>>8)-0XFD;
	}else{
		send_buff[(send_buff_len)++]=(crc>>8);
	}
	send_buff[(send_buff_len)++] = FRAME_TAIL;
	uart_write_bytes(UART_NUM_2,(const char *) send_buff,send_buff_len);
}

static void pull_motor_set_count(uint16_t cnt)
{
	pull_motor_write_par(45,cnt);
}

static void send_write_recv(uint8_t cmd,uint8_t order)
{
	uint8_t send_buff[20];
	uint8_t send_buff_len = 0;;
	send_buff[(send_buff_len)++] = FRAME_HEADER;
	send_buff[(send_buff_len)++] = cmd;
	send_buff[(send_buff_len)++] = order;
	uint16_t crc = calCrcBitByBit(&(send_buff[1]),send_buff_len-1);

	if((uint8_t)(crc)>=0XFD)
	{
		send_buff[(send_buff_len)++]=0XFD;
		send_buff[(send_buff_len)++]=(uint8_t)crc-0XFD;
	}else{
		send_buff[(send_buff_len)++]=crc;
	}
	if((uint8_t)(crc>>8)>=0XFD)
	{
		send_buff[(send_buff_len)++]=0XFD;
		send_buff[(send_buff_len)++]=(uint8_t)(crc>>8)-0XFD;
	}else{
		send_buff[(send_buff_len)++]=(crc>>8);
	}
	send_buff[(send_buff_len)++] = FRAME_TAIL;
	uart_write_bytes(UART_NUM_2,(const char *) send_buff,send_buff_len);
}

/*解析协议时需要将FD+XX替换成原本的字节*/
static void data_FD_reply(uint8_t *data,uint8_t len ,uint8_t *temp,uint8_t *temp_len)
{
	uint8_t k = 0;
	for (int i = 0; i < len; i++){
		if(data[i]==0xfd){
			k=1;
		}else if(k==1){
			k = 0;
			if(data[i]==0x02){
				temp[*temp_len] = 0xff;
			}else if(data[i]==0x01){
				temp[*temp_len] = 0xfe;
			}else if(data[i]==0x00){
				temp[*temp_len] = 0xfd;
			}
			temp_len[0]++;
		}else{
			temp[*temp_len] = data[i];
			temp_len[0]++;
		}	
	}
}

static uint8_t err = 0;
static uint8_t res_err = 0;
static uint8_t power_err = 0;

uint8_t is_res_err(void)
{
	static uint8_t send = 0;
	if (err == 1 && send == 0)
	{
		send = 1;
		res_err = 1;
		return 1;
	}
	return 0;
}

uint8_t is_power_err(void)
{
	static uint8_t send = 0;
	if (err == 3 && send == 0)
	{
		send = 1;
		power_err = 1;
		return 1;
	}
	return 0;
}

int has_res_err(void)
{
	return res_err;
}

int has_power_err(void)
{
	return power_err;
}

static void pull_motor_read_data(uint8_t *buf)
{
	switch (buf[3])
	{
	case FRAME_GET_RPM:
		rpm_err_cnt = 0;
		update_RPM(buf[4] | (buf[5] << 8));
		break;
	case FRAME_GET_BATT:
	{
#define MIN_BATT_V (7300)
#define MAX_BATT_V (8300)

		uint32_t batt_voltage = buf[4] | (buf[5] << 8);
		uint32_t batt = 0;
		if (batt_voltage < MIN_BATT_V) // 读取到的电池电压小于7400毫伏，电池电量百分比直接为0
		{
			batt = 0;
		}
		else
		{
			batt = (batt_voltage - MIN_BATT_V) / 10;
		}
		if(batt>100)
		{
			batt = 100;
		}
		update_batt(batt);
		// ESP_LOGI("mo", "batt=%u v = %u", batt, batt_voltage);
	}
	break;
	case FRAME_GET_GE_POWER:
	{
		// uint32_t ge_power = buf[4] | (buf[5] << 8);
	}
	break;
	case FRAME_GET_ZERO: pull_list.zrro_fg = buf[4];break;
	case FRAME_GET_COUNT:	
	{
		size_t count = buf[4] | (buf[5] << 8);
		set_actual_res((uint32_t)count);
	}break;

	case FRAME_GET_ERR:
	{
		err = buf[4];
		// ESP_LOGI("mo", "err = =%u", res_err);
	}break;
	
	case 68://0x44 开始校准
	{
		uint8_t err = buf[4];
		ESP_LOGE("mo", "err = =%u", err);
	}break;

	case 0://0x44 开始校准
	{
		uint8_t send_buf[3] = {0};
		send_buf[0] = 0XFF;
		send_buf[1] = buf[4];
		send_buf[2] = buf[5];
		user_ipc_send_queue(BLE_IPC_SIID_CHECK_INFO,
		send_buf,3);
		size_t count = buf[4] | (buf[5] << 8);
		ESP_LOGE("mo", "ver = =%u", count);
	}break;

	case 69://设备校准状态
	{
		uint8_t send_buf[2] = {0};
		send_buf[0] = 69;
		send_buf[1] = buf[4];
		user_ipc_send_queue(BLE_IPC_SIID_CHECK_INFO,
		send_buf,2);
		ESP_LOGE("mo", "设备校准状态 %u", send_buf[1]);
	}break;
	case 70://读取最大校准值
	{
		uint8_t send_buf[3] = {0};
		send_buf[0] = 70;
		send_buf[1] = buf[4];
		send_buf[2] = buf[5];
		user_ipc_send_queue(BLE_IPC_SIID_CHECK_INFO,
		send_buf,3);
		size_t count = buf[4] | (buf[5] << 8);
		ESP_LOGE("mo", "max_check = =%u", count);
	}break;
	case 71://读取最小校准值
	{
		uint8_t send_buf[3] = {0};
		send_buf[0] = 71;
		send_buf[1] = buf[4];
		send_buf[2] = buf[5];
		user_ipc_send_queue(BLE_IPC_SIID_CHECK_INFO,
		send_buf,3);
		size_t count = buf[4] | (buf[5] << 8);
		ESP_LOGE("mo", "max_check = =%u", count);
	}break;
	default:
		break;
	}
}

void pull_motor_decode(uint8_t *data,uint32_t len)
{
	data_send_fg = 0;
	uint8_t temp[20] = {0};
	uint8_t temp_len = 0;
	/*将FD字节还原成原本的含义*/
	data_FD_reply(data, len, temp, &temp_len);

	if (temp[0] == FRAME_HEADER)
	{
		uint16_t crc = calCrcBitByBit(&(temp[1]), temp_len - 4);
		uint16_t crc_temp = ((temp[temp_len - 2]) << 8) | temp[temp_len - 3];
		if (crc == crc_temp)
		{
			switch (temp[1])
			{
			case 0x00:
				switch (temp[2])
				{
				case FRAME_WHITE:
					break;
				case FRAME_READ:
					pull_motor_read_data(temp);
					break;

				default:
					break;
				}
				break;
			case 0x02:
				break;
			default:
				break;
			}
		}
		else
		{
			ESP_LOGE("mo", "crc校验错误");
			for (int i = 0; i < len; i++)
			{
				ESP_LOGE("mo", "data[%u]=%x\n", i, data[i]);
			}
		}
	}
}

static void decode_Task(void *par)
{
	uint8_t pull_sw = PULL_INIT;
	while (1)
    {
		while (close_fg)
		{
			pull_motor_write_par(49,0x01);
			vTaskDelay(50 / portTICK_PERIOD_MS);
		}
		switch (get_check())
		{
		case 0xff:
			send_write_recv(FRAME_READ, 0); // 读取电池电压
			vTaskDelay(100 / portTICK_PERIOD_MS);
			break;
		case 68:
			pull_motor_write_par(68,0x01);
			vTaskDelay(100 / portTICK_PERIOD_MS);
			break;
		case 69:
			send_write_recv(FRAME_READ, 69); // 读取电池电压
			vTaskDelay(100 / portTICK_PERIOD_MS);
			break;
		case 70:
			send_write_recv(FRAME_READ, 70); // 读取电池电压
			vTaskDelay(100 / portTICK_PERIOD_MS);
			break;
		case 71:
			send_write_recv(FRAME_READ, 71); // 读取电池电压
			vTaskDelay(100 / portTICK_PERIOD_MS);
			break;

		default:
			break;
		}
		if (get_data_update())
		{
			data_send_fg = 1; // 置位发送标志
			switch (pull_sw)
			{
			case PULL_INIT:
				pull_motor_set_rst();
				pull_sw = PULL_RPM;
				break;
			case PULL_RST:
			{
				if (pull_list.zrro_fg == 1)
				{
					pull_sw = PULL_RPM;
				}
				else
				{
					send_write_recv(FRAME_READ, 48); // 读取0点状态，0非零，1零点
				}
			}
			break;
			case PULL_RPM:
			{
				send_write_recv(FRAME_READ, 74); // 读取rpm
				if (rpm_err_cnt < 5)
				{
					rpm_err_cnt++;
				}
				else
				{
					update_RPM(0);
				}
				pull_sw = PULL_COUNT;
			}
			break;
			case PULL_COUNT:
			{
				pull_motor_set_count(get_RES());
				pull_sw = PULL_BATT;
			}
			break;
			case PULL_BATT:
			{
				send_write_recv(FRAME_READ, 77); // 读取电池电压
				pull_sw = PULL_POWER;
			}
			break;
			case PULL_POWER:
			{
				send_write_recv(FRAME_READ, FRAME_GET_GE_POWER); // 读取发电功率
				pull_sw = PULL_READ_COUNT;
			}
			break;
			case PULL_READ_COUNT:
			{
				send_write_recv(FRAME_READ, FRAME_GET_COUNT); // 读取发电功率
				pull_sw = PULL_READ_ERR;
			}
			break;
			case PULL_READ_ERR: // 读取报错信息
			{
				send_write_recv(FRAME_READ, FRAME_GET_ERR);
				pull_sw = PULL_RPM;
			}
			break;
			default:
				ESP_LOGE(USER_PULL_TAG, "USER_PULL_err");
				break;
			}
		}
		vTaskDelay(50 / portTICK_PERIOD_MS);
	}
}

void User_pull_motor_close(void)
{
	close_fg = 1;
}

void User_pull_motor(void)
{
	xTaskCreatePinnedToCore(decode_Task, "decode_Task", 2048, NULL, 3, NULL, tskNO_AFFINITY);
}