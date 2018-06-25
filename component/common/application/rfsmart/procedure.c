#include "product.h"
#include "procedure.h"
#include <stdio.h>
#include <string.h>

/* ���Եĸ������ڳ�ʼ����ʱ���ȼ������Ը��� */
unsigned char AttrNum = 0x00;	

/* ���ڷ�������buffer */
unsigned char SendBufferData[SENDMAXLEN] = {0x00};

/* �ϴ����Է���״̬���� */
int AttrSendErrorCode = 0;

/*
Bit0:  0:AP_Mode;  1:STA Mode
Bit1:  0:��·�ɳɹ� 1:��·��ʧ��
Bit2:  0:����ʧ��  1:���Ƴɹ�
Bit3:  0:�豸��ʧ�� 1:�豸�󶨳ɹ�
*/
/* WIFI״̬��ʶλ, */
unsigned char RFLinkWIFIStatus = 0x00;	

unsigned char ResetWIFI_Flag = 0x00;				/* ����WIFIģ�� */
unsigned char MCURestartWIFI_Flag = 0x00;		/* MCU����WIFIģ�� */
unsigned char MCUGetNTPTime_Flag = 0x00;		/* MCU��ȡʱ���־ */
unsigned char MCUGetWIFIRSSI_Flag = 0x00;		/* MCU��ȡWIFI�źŵ�ǿ�� */

/* ��ȡ��ϵͳ����ʱ����Ϣ */
struct clock_info Netclk;

/* WIFI�źŵ�ǿ�� */
int WIFI_RSSI = 0x00;

/* �豸��Ϣ���� */
extern char hardversion[];
extern char softversion[];
extern char host_version[];
extern char oem_model[];
extern char oem_id[];
extern char OEM_AP_SSID_PREFIX[];
extern struct ada_sprop demo_props[];

/*******************************************************************************************
*                    			  start  ҵ����                                          *
*******************************************************************************************/

/* �ϴ��豸��Ϣ */
void GetDevInfoHandler(void)
{
	int cnt = 0;
	int sub = 0;

	while((hardversion[cnt] != '\0'))
	{
		SendBufferData[sub] = hardversion[cnt];
		sub++;
		cnt++;
	}

	SendBufferData[sub] = 0x20;	//SPACE
	sub++;

	cnt = 0;
	while((softversion[cnt] != '\0'))
	{
		SendBufferData[sub] = softversion[cnt];
		sub++;
		cnt++;
	}
	SendBufferData[sub] = 0x20;	//SPACE
	sub++;

	cnt = 0;
	while((host_version[cnt] != '\0'))
	{
		SendBufferData[sub] = host_version[cnt];
		sub++;
		cnt++;
	}
	SendBufferData[sub] = 0x20;	//SPACE
	sub++;

	cnt = 0;
	while((oem_id[cnt] != '\0'))
	{
		SendBufferData[sub] = oem_id[cnt];
		sub++;
		cnt++;
	}
	SendBufferData[sub] = 0x20;	//SPACE
	sub++;

	cnt = 0;
	while((oem_model[cnt] != '\0'))
	{
		SendBufferData[sub] = oem_model[cnt];
		sub++;
		cnt++;
	}
	SendBufferData[sub] = 0x20;	//SPACE
	sub++;

	cnt = 0;
	while((OEM_AP_SSID_PREFIX[cnt] != '\0'))
	{
		SendBufferData[sub] = OEM_AP_SSID_PREFIX[cnt];
		sub++;
		cnt++;
	}
	SendBufferData[sub] = 0x20;	//SPACE
	sub++;

	SendBufferData[sub] = AttrNum;
	sub++;
	
	UartSendFormData(GET_DEV_INFO_CMD_REQ, SendBufferData, sub);
	memset(SendBufferData, 0x00, SENDMAXLEN);
}

/* ��ȡ�����б� */
void GetDevAttrHandler(unsigned char AttrIndex)
{
	int sub = 0;
	int cnt = 0;

	SendBufferData[sub++] = demo_props[AttrIndex].dir;
	SendBufferData[sub++] = demo_props[AttrIndex].site;
	SendBufferData[sub++] = demo_props[AttrIndex].type;
	while(demo_props[AttrIndex].name[cnt] != '\0')
	{
		SendBufferData[ sub] = demo_props[AttrIndex].name[cnt];
		sub++;
		cnt++;
	}
	SendBufferData[sub++] = 0x20;	//SPACE

	UartSendFormData(WIFI_GET_DEV_ATTR_CMD_REQ, SendBufferData, sub);
	memset(SendBufferData, 0x00, SENDMAXLEN);
}

/* ��ȡ�������Եĳ�ʼֵ */
void GetDEVAllAttrVal(unsigned char AttrIndex)
{
	int sub = 0;
	unsigned char AttrType;
	unsigned short AttrLen; 
	
	SendBufferData[sub++] = demo_props[AttrIndex].dir;
	SendBufferData[sub++] = demo_props[AttrIndex].type;
	SendBufferData[sub++] = demo_props[AttrIndex].site;
	AttrType = demo_props[AttrIndex].type;
	
	if(AttrType == TYPE_BOOL){
		SendBufferData[sub++] = 0x00;
		SendBufferData[sub++] = 0x01;
		SendBufferData[sub++] = *(unsigned char *)demo_props[AttrIndex].val;
	}else if(AttrType == TYPE_INT){
		SendBufferData[sub++] = 0x00;
		SendBufferData[sub++] = 0x04;
		SendBufferData[sub++] = (*(int *)demo_props[AttrIndex].val) >> 24;
		SendBufferData[sub++] = (*(int *)demo_props[AttrIndex].val) >> 16;
		SendBufferData[sub++] = (*(int *)demo_props[AttrIndex].val) >> 8;
		SendBufferData[sub++] = (*(int *)demo_props[AttrIndex].val) >> 0;
	}else if(AttrType == TYPE_CENTS){
		SendBufferData[sub++] = 0x00;
		SendBufferData[sub++] = 0x04;
		SendBufferData[sub++] = (*(int *)demo_props[AttrIndex].val) >> 24;
		SendBufferData[sub++] = (*(int *)demo_props[AttrIndex].val) >> 16;
		SendBufferData[sub++] = (*(int *)demo_props[AttrIndex].val) >> 8;
		SendBufferData[sub++] = (*(int *)demo_props[AttrIndex].val) >> 0;
	}else if(AttrType == TYPE_UTF8){
		AttrLen = strlen(demo_props[AttrIndex].val);
		SendBufferData[sub++] = (AttrLen >> 8) & 0xFF;
		SendBufferData[sub++] = (AttrLen >> 0) & 0xFF;
		memcpy(&SendBufferData[sub], (char *)demo_props[AttrIndex].val, AttrLen);
		sub = sub + AttrLen;
	}

	UartSendFormData(WIFI_READ_DEV_STA_CMD_REQ, SendBufferData, sub);
	memset(SendBufferData, 0x00, SENDMAXLEN);
}

void SetToAttrValCallback(struct ada_sprop *sprop, const void *buf, unsigned int  len, void (*set)(struct ada_sprop *, const void *buf, unsigned int  len))
{
	set(sprop, buf, len);
}

/* WIFIģ������豸 */
void SetToAttrVal(unsigned char *Data, unsigned short Length)
{
	unsigned char AttrType;
	unsigned char AttrIndex;
	unsigned short AttrDataLen;

	unsigned char BoolVal;
	int IntVal;
	int DecimalVal;

	AttrType = Data[0];		/* �������� */
	AttrIndex = Data[1];	/* ����λ�� */
	AttrDataLen =Length - 2;	/* ��ȥ���ݵ����ͺ�����λ�� */

	if(AttrType == TYPE_BOOL){
		*(unsigned char *)demo_props[AttrIndex].val = Data[2];
		BoolVal =  Data[2];
		SetToAttrValCallback(&demo_props[AttrIndex], &BoolVal, AttrDataLen, demo_props[AttrIndex].set);
		
	}else if(AttrType == TYPE_INT){
		*(int *)demo_props[AttrIndex].val =  (Data[2] << 24) | (Data[3] << 16) | (Data[4] << 8) | (Data[5] << 0);
		IntVal = (Data[2] << 24) | (Data[3] << 16) | (Data[4] << 8) | (Data[5] << 0);
		SetToAttrValCallback(&demo_props[AttrIndex], &IntVal, AttrDataLen, demo_props[AttrIndex].set);
		
	}else if(AttrType == TYPE_CENTS){
		*(int *)demo_props[AttrIndex].val =  ((Data[2] << 24) | (Data[3] << 16) | (Data[4] << 8) | (Data[5] << 0));
		DecimalVal = ((Data[2] << 24) | (Data[3] << 16) | (Data[4] << 8) | (Data[5] << 0));
		SetToAttrValCallback(&demo_props[AttrIndex], &DecimalVal, AttrDataLen, demo_props[AttrIndex].set);
		
	}else if(AttrType == TYPE_UTF8){
		if(AttrDataLen > demo_props[AttrIndex].val_len){
			//PRINTF("AttrDataLen = %d, demo_props[AttrIndex].val_len = %d\n", AttrDataLen, demo_props[AttrIndex].val_len);
			memset((char *)demo_props[AttrIndex].val, 0x00, demo_props[AttrIndex].val_len);
			memcpy((char *)demo_props[AttrIndex].val, &Data[2], demo_props[AttrIndex].val_len);
			SetToAttrValCallback(&demo_props[AttrIndex], (char *)demo_props[AttrIndex].val, demo_props[AttrIndex].val_len, demo_props[AttrIndex].set);
		}
		else{
			//PRINTF("AttrDataLen = %d, demo_props[AttrIndex].val_len = %d\n", AttrDataLen, demo_props[AttrIndex].val_len);
			memset((char *)demo_props[AttrIndex].val, 0x00, demo_props[AttrIndex].val_len);
			memcpy((char *)demo_props[AttrIndex].val, &Data[2], AttrDataLen);
			SetToAttrValCallback(&demo_props[AttrIndex], (char *)demo_props[AttrIndex].val, AttrDataLen, demo_props[AttrIndex].set);
		}
	}

	UartSendFormData(WIFI_CTRL_DEV_CMD_REQ, &AttrIndex, 1);
}

/***************************************
	���ô˽ӿڷ������Ե��ƶ�
	name: Ҫ���͵�������
	value:Ҫ���͵�����ֵ
	ValueLen:����ռ�õ��ֽ���char ռ1��intռ4
	AttrType:��������:TYPE_BOOL��TYPE_INT��TYPE_CENTS��TYPE_UTF8
	����ֵ : ����0��ʾ�ϴ��ɹ�����������ֵΪʧ�� 
****************************************/
unsigned char SendAttrToCloud( char *name, void *Value, unsigned short ValueLen, unsigned char AttrType)
{
	int cnt = 0;
	int i = 0;
	unsigned char AttrIndex = 0;

	for(i = 0; i < AttrNum; i++)
	{
		if(0 == memcmp(name, demo_props[i].name, strlen(demo_props[i].name)))
			break;
	}
	if(i >= AttrNum){
		PRINTF("Attr Name Error.\n");
		return -19;
	}
	
	AttrIndex = i;		/* ��ʱ��i�����������ڵ����� */

	SendBufferData[cnt++] = demo_props[AttrIndex].dir;	/* ���Է��� */
	SendBufferData[cnt++] = AttrType;	/* �������� */
	SendBufferData[cnt++] = AttrIndex;	/* �������� */
	SendBufferData[cnt++] = (ValueLen >> 8) & 0xFF;	/* ����ֵ�ĳ��� */
	SendBufferData[cnt++] = (ValueLen >> 0) & 0xFF;

	if(AttrType == TYPE_BOOL){
		SendBufferData[cnt++] = *(unsigned char *)Value;
		PRINTF("Send Attr [%s] to cloud, Value:[%d]\n", demo_props[AttrIndex].name, *(unsigned char *)Value);
	}else if(AttrType == TYPE_INT){
		SendBufferData[cnt++] = (*( int *)Value >> 24) & 0xFF;
		SendBufferData[cnt++] = (*( int *)Value >> 16) & 0xFF;
		SendBufferData[cnt++] = (*( int *)Value >> 8) & 0xFF;
		SendBufferData[cnt++] = (*( int *)Value >> 0) & 0xFF;
		PRINTF("Send Attr [%s] to cloud, Value:[%d]\n", demo_props[AttrIndex].name, *(int *)Value);
	}else if(AttrType == TYPE_CENTS){
		SendBufferData[cnt++] = (*( int *)Value >> 24) & 0xFF;
		SendBufferData[cnt++] = (*( int *)Value >> 16) & 0xFF;
		SendBufferData[cnt++] = (*( int *)Value >> 8) & 0xFF;
		SendBufferData[cnt++] = (*( int *)Value >> 0) & 0xFF;
		PRINTF("Send Attr [%s] to cloud, Value:[%d]\n", demo_props[AttrIndex].name, *(int *)Value);
	}else if(AttrType == TYPE_UTF8){
		memcpy(&SendBufferData[cnt], (char *)Value, ValueLen);
		cnt = cnt + ValueLen;
		PRINTF("Send Attr [%s] to cloud, Value:[%s]\n", demo_props[AttrIndex].name,  (char *)Value);
	}

	UartSendFormData(MCU_UP_PROPERTY_CMD_REQ, SendBufferData, cnt);
	memset(SendBufferData, 0x00, SENDMAXLEN);

	return AttrSendErrorCode;
}

/* ����WIFIģ�鷢�͵�״̬ */
void GetWIFIStatus(unsigned char WIFIStatus){
	RFLinkWIFIStatus = WIFIStatus;
	PRINTF("RFLinkWIFIStatus = %x\n", RFLinkWIFIStatus);
	UartSendFormData(WIFI_SNED_STA_CMD_REQ, NULL, 0);
}

/********************************************************
	*���ô˽ӿڻ�ȡwifi��״̬��Ϣ,ֱ�ӷ���wifi��״̬
	*Bit0:  0:AP_Mode;  1:STA Mode
	*Bit1:  0:��·�ɳɹ� 1:��·��ʧ��
	*Bit2:  0:����ʧ��  1:���Ƴɹ�
	*Bit3:  0:�豸��ʧ�� 1:�豸�󶨳ɹ�
********************************************************/
unsigned char MCU_GetWIFIStatus(void)
{
	return RFLinkWIFIStatus;
}

/*************************
	MCU����WIFIģ��
	����0Ϊ�ɹ���1Ϊʧ��
*************************/
unsigned char MCU_ResetWIFI(void)
{
	unsigned char ret = ACK_OK;

	ResetWIFI_Flag = 0x00;
	UartSendFormData(RESET_WIFI_CMD_REQ, NULL, 0);
	while(ResetWIFI_Flag != 0x01);
	return ret;
}

/*****************************
	MCU����WIFIģ��
	����0Ϊ�ɹ���1Ϊʧ��
*****************************/
unsigned char MCU_RestartWIFI(void)
{
	unsigned char ret = ACK_OK;

	MCURestartWIFI_Flag = 0x00;
	UartSendFormData(MCU_RESTART_WIFI_CMD_REQ, _NULL_, 0);
	while(MCURestartWIFI_Flag != 0x01);
	return ret;
}

/************************************************* 
	����zone : 0Ϊ��ȡutcʱ�䣬 Ϊ1��ȡ����ʱ�� 
	netclktime ��ȡʱ��ṹ��ķ���ֵ 
	��ȡ�ɹ�����0��ʧ�ܷ���-1
*************************************************/
unsigned char MCU_GetNTPTime(struct clock_info *netclktime, unsigned char zone)
{
	unsigned char ret = ACK_OK;
	
	MCUGetNTPTime_Flag = 0x00;
	UartSendFormData(MCU_GET_NTP_TIME_CMD_REQ, &zone, 1);
	while(MCUGetNTPTime_Flag != 0x01);
	
	netclktime->year = Netclk.year;
	netclktime->month= Netclk.month;
	netclktime->days = Netclk.days;
	netclktime->day_of_week = Netclk.day_of_week;
	netclktime->hour = Netclk.hour;
	netclktime->min= Netclk.min;
	netclktime->sec = Netclk.sec;
	netclktime->TimeStamp = Netclk.TimeStamp;
	
	return ret;
}

/* ***********************************************
	1����ȡ�ź�ǿ�� 
	�ź�ǿ�ȷ�Χ��0��-200,��Խ�ӽ�0�ź�ǿ��Խ��
	2���ɹ������ź�ǿ�ȣ�ʧ�ܷ���1
*************************************************/
int Get_WIFIRSSI(void)
{
	MCUGetWIFIRSSI_Flag = 0x00;
	UartSendFormData(MCU_GET_WIFI_RSSI_CMD_REQ, NULL, 0);
	while(MCUGetWIFIRSSI_Flag != 0x01);
	
	return WIFI_RSSI;
}


/*******************************************************************************************
*                    			  end  ҵ����                                            *
*******************************************************************************************/



/*******************************************************************************************
*                    			  start  �����շ��Ĵ���                                    *
*******************************************************************************************/
/* ���ڽ�����ر��� */
unsigned char UartStatus = UARTNOP;			/* ����״̬ */
unsigned char UartRxOkFlag = 0;		/* ���ڽ������һ֡���ݵı�־��ʹ���������֮������ */
unsigned char RecvBuffer[RECVMAXLEN] = {0x00};	/* recv from mcu*/
unsigned short UartRecvSumLen = 0;			/* ���ڽ��յ��ܵ����ݳ��ȣ�ÿ����һ�ֽ�����1 */
unsigned char RecvCRC = 0;					/* ���յ���CRC����*/
unsigned char CalCRC = 0;					/* ���������CRC����*/
unsigned short UartRecvDataLen = 0;			/* ���ݰ����������ݰ��ĳ��ȣ�������ͷ�������ȡ� �����ȡ� ����λ�� ����λ�������롢���ݡ�У��͡���β�� */
unsigned int RecvUartTimeOutCnt = 0;	/* �����жϽ��ճ�ʱ��־����ֹ���ʹ������ݵ�ʱ��Ӱ����һ֡�Ľ��� */

/* 	���մ������ݴ�����
	CMD:����
	Data:����������
	Len:���������ݳ���
*/
void ProtocalUartData(unsigned char CMD, unsigned char *Data, unsigned short Length)
{	
	switch(CMD)
	{
		/* ��ȡ�豸����Ϣ */
		case GET_DEV_INFO_CMD:
		{
			GetDevInfoHandler();
			break;
		}

		/* ��ȡ�豸�����б� */
		case WIFI_GET_DEV_ATTR_CMD:
		{
			GetDevAttrHandler(Data[0]);
			break;
		}

		/* WIFIģ���ȡ�豸�������Ե�״̬ */
		case  WIFI_READ_DEV_STA_CMD :
		{
			GetDEVAllAttrVal(Data[0]);
			break;
		}

		/* WIFIģ������豸 */
		case  WIFI_CTRL_DEV_CMD :	
		{	
			SetToAttrVal(Data, Length);
			break;
		}

		/* MCU�����ϱ���ǰ״̬ */
		case MCU_UP_PROPERTY_CMD :
		{
			AttrSendErrorCode = (Data[0] << 24) | (Data[1] << 16) | (Data[2] << 8) | (Data[3] << 0);
			if(AttrSendErrorCode != 0)
			{
				PRINTF("Send [%s] Attr to Cloud failed.error code : [%d]\n", demo_props[Data[4]].name, AttrSendErrorCode);
			}else{
				PRINTF("Send [%s] Attr to Cloud OK.\n", demo_props[Data[4]].name);
			}
			break;
		}

		/* ����wifi����״̬ */
		case WIFI_SNED_STA_CMD :
		{
			GetWIFIStatus(Data[0]);
			break;
		}

		case RESET_WIFI_CMD: 			//����WIFIģ��
		{
			ResetWIFI_Flag = 0x01;
			break;
		}

		case MCU_RESTART_WIFI_CMD :		//����WIFIģ��
		{
			MCURestartWIFI_Flag = 0x01;
			break;
		}

		case MCU_GET_NTP_TIME_CMD:
		{
			Netclk.year = (Data[0] << 8) | Data[1];
			Netclk.month = Data[2];
			Netclk.days= Data[3];
			Netclk.day_of_week= Data[4];
			Netclk.hour= Data[5];
			Netclk.min= Data[6];
			Netclk.sec= Data[7];
			
			Netclk.TimeStamp = (Data[8] << 24) | (Data[9] << 16) | (Data[10] << 8) | (Data[11] << 0);
			MCUGetNTPTime_Flag = 0x01;
			break;
		}

		case MCU_GET_WIFI_RSSI_CMD:
		{
			WIFI_RSSI = (Data[0] << 24) | (Data[1] << 16) | (Data[2] << 8) | (Data[3] << 0) ;
			MCUGetWIFIRSSI_Flag = 0x01;
			break;
		}

		default:
			break;
	}
}

/* ������ر����ĳ�ʼ�� */
void UartVariableInlt(void)
{
	UartRecvSumLen = 0;
	UartRecvDataLen = 0;
	CalCRC = 0;
	RecvCRC = 0;
	UartStatus = UARTNOP;
	UartRxOkFlag = 0;
	memset(RecvBuffer, 0x00, RECVMAXLEN);
	RecvUartTimeOutCnt = 0x00;
}

/* ���ڽ������ݴ��� */
void UartHandler(unsigned char Data)
{
	int  i;
	
	switch(UartStatus)
	{
		case UARTNOP:
		{
			if(UartRxOkFlag){
				break;
			}else{
				UartStatus = UARTSOP_H;
			}
		}

		case UARTSOP_H:
		{
			if(Data == F_HEAD_H)
			{
				RecvBuffer[UartRecvSumLen++] = Data;
				UartStatus = UARTSOP_L;
				RecvUartTimeOutCnt = 0x01;	/* ��ʼ�д������ݽ��� */
			}
			break;
		}

		case UARTSOP_L:
		{
			if(Data == F_HEAD_L){
				RecvBuffer[UartRecvSumLen++] = Data;
				UartStatus = UARTLEN_H;
			}
			break;
		}

		case UARTLEN_H:
		{
			RecvBuffer[UartRecvSumLen++] = Data;
			CalCRC += Data;
			UartRecvDataLen |= (Data << 8);
			UartStatus = UARTLEN_L;
			break;
		}

		case UARTLEN_L:
		{
			RecvBuffer[UartRecvSumLen++] = Data;
			CalCRC += Data;
			UartRecvDataLen |= (Data << 0);
			UartStatus = UARTCMD;
			break;
		}

		case UARTCMD:
		{
			RecvBuffer[UartRecvSumLen++] = Data;
			CalCRC += Data;
			UartStatus = UARTDATA;
			break;
		}

		case UARTDATA:
		{
			if(UartRecvDataLen > 6)		/*��ΪUartDataLen�ĳ��ȳ��˰�������֮�⻹����������6���ֽڵ�֡ͷ���ȵȵ���Ϣ*/
			{
				RecvBuffer[UartRecvSumLen++] = Data;
				CalCRC += Data;
				UartRecvDataLen--;
				break;
			}
			else
			{
				UartStatus = UARTCRC;
			}
		}

		case UARTCRC:
		{
			RecvBuffer[UartRecvSumLen++] = Data;
			RecvCRC = Data;

			if(RecvCRC == CalCRC){
				UartRxOkFlag = 0x01;	/* ������ɱ�־ */
				
				/* ҵ���� */
				if(UartRecvSumLen < 64){
					PRINTF("Recv Data:");	
					for(i = 0; i < UartRecvSumLen; i++)
					{
						PRINTF("%x ", RecvBuffer[i]);
					}
					PRINTF("\n");
				}else{
					PRINTF("Recv %d Data.\n", UartRecvSumLen);
				}

				ProtocalUartData(RecvBuffer[4],& RecvBuffer[5], UartRecvSumLen - 6);
			}else{
				PRINTF("\n===Recv CRC Error.===Expect CRC is: 0x%x, But recv is0x%x\n", CalCRC, RecvCRC);
				PRINTF("Recv Data:");	
				for(i = 0; i < UartRecvSumLen; i++)
				{
					PRINTF("%x ", RecvBuffer[i]);
				}
				PRINTF("\n");
			}
			UartVariableInlt();
			break;
		}
		
		default:
			break;
	}
}

/* �ڶ�ʱ��������ã����ճ�ʱ */
void UartErrorRecvTimeout(void)
{	
	if(RecvUartTimeOutCnt != 0)	
	{		
		RecvUartTimeOutCnt++;	
		if(RecvUartTimeOutCnt > 15)	//���ճ�ʱ1.5��
		{
			UartVariableInlt();
			PRINTF("Error: Uart Recv Data Time Out.\n");
		}
	}
}
/*******************************************************************************************
*                    			  end  �����շ��Ĵ���                                      *
*******************************************************************************************/


