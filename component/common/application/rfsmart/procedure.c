#include "product.h"
#include "procedure.h"
#include <stdio.h>
#include <string.h>

/* 属性的个数，在初始化的时候先计算属性个数 */
unsigned char AttrNum = 0x00;	

/* 串口发送数据buffer */
unsigned char SendBufferData[SENDMAXLEN] = {0x00};

/* 上传属性返回状态代码 */
int AttrSendErrorCode = 0;

/*
Bit0:  0:AP_Mode;  1:STA Mode
Bit1:  0:联路由成功 1:联路由失败
Bit2:  0:联云失败  1:联云成功
Bit3:  0:设备绑定失败 1:设备绑定成功
*/
/* WIFI状态标识位, */
unsigned char RFLinkWIFIStatus = 0x00;	

unsigned char ResetWIFI_Flag = 0x00;				/* 重置WIFI模组 */
unsigned char MCURestartWIFI_Flag = 0x00;		/* MCU重启WIFI模组 */
unsigned char MCUGetNTPTime_Flag = 0x00;		/* MCU获取时间标志 */
unsigned char MCUGetWIFIRSSI_Flag = 0x00;		/* MCU获取WIFI信号的强度 */

/* 获取的系统网络时间信息 */
struct clock_info Netclk;

/* WIFI信号的强度 */
int WIFI_RSSI = 0x00;

/* 设备信息定义 */
extern char hardversion[];
extern char softversion[];
extern char host_version[];
extern char oem_model[];
extern char oem_id[];
extern char OEM_AP_SSID_PREFIX[];
extern struct ada_sprop demo_props[];

/*******************************************************************************************
*                    			  start  业务处理                                          *
*******************************************************************************************/

/* 上传设备信息 */
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

/* 获取属性列表 */
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

/* 获取所有属性的初始值 */
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

/* WIFI模组控制设备 */
void SetToAttrVal(unsigned char *Data, unsigned short Length)
{
	unsigned char AttrType;
	unsigned char AttrIndex;
	unsigned short AttrDataLen;

	unsigned char BoolVal;
	int IntVal;
	int DecimalVal;

	AttrType = Data[0];		/* 属性类型 */
	AttrIndex = Data[1];	/* 属性位置 */
	AttrDataLen =Length - 2;	/* 除去数据的类型和属性位置 */

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
	调用此接口发送属性到云端
	name: 要发送的属性名
	value:要发送的属性值
	ValueLen:属性占用的字节数char 占1，int占4
	AttrType:属性类型:TYPE_BOOL、TYPE_INT、TYPE_CENTS、TYPE_UTF8
	返回值 : 返回0表示上传成功，返回其他值为失败 
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
	
	AttrIndex = i;		/* 此时的i就是属性所在的索引 */

	SendBufferData[cnt++] = demo_props[AttrIndex].dir;	/* 属性方向 */
	SendBufferData[cnt++] = AttrType;	/* 属性类型 */
	SendBufferData[cnt++] = AttrIndex;	/* 属性索引 */
	SendBufferData[cnt++] = (ValueLen >> 8) & 0xFF;	/* 属性值的长度 */
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

/* 接收WIFI模块发送的状态 */
void GetWIFIStatus(unsigned char WIFIStatus){
	RFLinkWIFIStatus = WIFIStatus;
	PRINTF("RFLinkWIFIStatus = %x\n", RFLinkWIFIStatus);
	UartSendFormData(WIFI_SNED_STA_CMD_REQ, NULL, 0);
}

/********************************************************
	*调用此接口获取wifi的状态信息,直接返回wifi的状态
	*Bit0:  0:AP_Mode;  1:STA Mode
	*Bit1:  0:联路由成功 1:联路由失败
	*Bit2:  0:联云失败  1:联云成功
	*Bit3:  0:设备绑定失败 1:设备绑定成功
********************************************************/
unsigned char MCU_GetWIFIStatus(void)
{
	return RFLinkWIFIStatus;
}

/*************************
	MCU重置WIFI模组
	返回0为成功，1为失败
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
	MCU重启WIFI模组
	返回0为成功，1为失败
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
	参数zone : 0为获取utc时间， 为1获取本地时间 
	netclktime 获取时间结构体的返回值 
	获取成功返回0，失败返回-1
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
	1、获取信号强度 
	信号强度范围从0到-200,，越接近0信号强度越好
	2、成功返回信号强度，失败返回1
*************************************************/
int Get_WIFIRSSI(void)
{
	MCUGetWIFIRSSI_Flag = 0x00;
	UartSendFormData(MCU_GET_WIFI_RSSI_CMD_REQ, NULL, 0);
	while(MCUGetWIFIRSSI_Flag != 0x01);
	
	return WIFI_RSSI;
}


/*******************************************************************************************
*                    			  end  业务处理                                            *
*******************************************************************************************/



/*******************************************************************************************
*                    			  start  串口收发的处理                                    *
*******************************************************************************************/
/* 串口接收相关变量 */
unsigned char UartStatus = UARTNOP;			/* 接收状态 */
unsigned char UartRxOkFlag = 0;		/* 串口接收完成一帧数据的标志，使用完成数据之后清零 */
unsigned char RecvBuffer[RECVMAXLEN] = {0x00};	/* recv from mcu*/
unsigned short UartRecvSumLen = 0;			/* 串口接收的总的数据长度，每接收一字节增加1 */
unsigned char RecvCRC = 0;					/* 接收到的CRC数据*/
unsigned char CalCRC = 0;					/* 计算出来的CRC数据*/
unsigned short UartRecvDataLen = 0;			/* 数据包中整个数据包的长度（包括包头、包长度、 包长度、 保留位、 保留位、功能码、数据、校验和、包尾） */
unsigned int RecvUartTimeOutCnt = 0;	/* 串口中断接收超时标志，防止发送错误数据的时候影响下一帧的接收 */

/* 	接收串口数据处理函数
	CMD:命令
	Data:真正的数据
	Len:真正的数据长度
*/
void ProtocalUartData(unsigned char CMD, unsigned char *Data, unsigned short Length)
{	
	switch(CMD)
	{
		/* 获取设备的信息 */
		case GET_DEV_INFO_CMD:
		{
			GetDevInfoHandler();
			break;
		}

		/* 获取设备属性列表 */
		case WIFI_GET_DEV_ATTR_CMD:
		{
			GetDevAttrHandler(Data[0]);
			break;
		}

		/* WIFI模组读取设备所有属性的状态 */
		case  WIFI_READ_DEV_STA_CMD :
		{
			GetDEVAllAttrVal(Data[0]);
			break;
		}

		/* WIFI模组控制设备 */
		case  WIFI_CTRL_DEV_CMD :	
		{	
			SetToAttrVal(Data, Length);
			break;
		}

		/* MCU主动上报当前状态 */
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

		/* 推送wifi工作状态 */
		case WIFI_SNED_STA_CMD :
		{
			GetWIFIStatus(Data[0]);
			break;
		}

		case RESET_WIFI_CMD: 			//重置WIFI模组
		{
			ResetWIFI_Flag = 0x01;
			break;
		}

		case MCU_RESTART_WIFI_CMD :		//重启WIFI模组
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

/* 串口相关变量的初始化 */
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

/* 串口接收数据处理 */
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
				RecvUartTimeOutCnt = 0x01;	/* 表开始有串口数据接收 */
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
			if(UartRecvDataLen > 6)		/*因为UartDataLen的长度除了包括数据之外还包含了其他6个字节的帧头长度等等信息*/
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
				UartRxOkFlag = 0x01;	/* 接收完成标志 */
				
				/* 业务处理 */
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

/* 在定时器里面调用，接收超时 */
void UartErrorRecvTimeout(void)
{	
	if(RecvUartTimeOutCnt != 0)	
	{		
		RecvUartTimeOutCnt++;	
		if(RecvUartTimeOutCnt > 15)	//接收超时1.5秒
		{
			UartVariableInlt();
			PRINTF("Error: Uart Recv Data Time Out.\n");
		}
	}
}
/*******************************************************************************************
*                    			  end  串口收发的处理                                      *
*******************************************************************************************/


