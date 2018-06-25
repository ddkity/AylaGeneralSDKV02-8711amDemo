#ifndef __PROCEDURE_H__
#define __PROCEDURE_H__

/* 1是打开log，0是关闭log */
#define DEBUG_LEVEL (1)
#define RF_PRINTF(level, fmt, arg...)     \
do {\
	if (level > 0) {\
		printf(fmt, ##arg);\
	}\
}while(0)
#define PRINTF(...)      RF_PRINTF(DEBUG_LEVEL, __VA_ARGS__)


/* 串口帧结构定义 */
#define F_HEAD_H  		(0xFF)	/* 帧头 */
#define F_HEAD_L  		 (0xFF)	/* 帧尾 */
#define UARTNOP  			(0)           /*  串口接收错误或空闲	*/
#define UARTSOP_H  		(1)           /*  接收起始位		*/
#define UARTSOP_L  		(2)           /*  接收起始位		*/
#define UARTLEN_H  		(3)           /*  接收长度			*/
#define UARTLEN_L  		(4)           /*  接收长度			*/
#define UARTCMD  		(5)           /*  接收命令			*/
#define UARTDATA 		(6)           /*  接收数据			*/
#define UARTCRC  			(7)           /*  数据校验和 		*/

#define SENDMAXLEN (1088)
#define RECVMAXLEN (1088)


/* 通讯命令 */
#define GET_DEV_INFO_CMD 			(0x01)			//获取设备信息
#define GET_DEV_INFO_CMD_REQ 		(0x02)
#define WIFI_CTRL_DEV_CMD 			( 0x03)			//WIFI模组控制设备
#define WIFI_CTRL_DEV_CMD_REQ 		( 0x04)
#define WIFI_READ_DEV_STA_CMD 		( 0x05)			//WIFI模组读取设备状态
#define WIFI_READ_DEV_STA_CMD_REQ  	(0x06)
#define MCU_UP_PROPERTY_CMD		(0x07)			//MCU主动上报当前状态
#define MCU_UP_PROPERTY_CMD_REQ	( 0x08)
#define HEARTBEAT_CMD 				( 0x09)			//心跳命令
#define HEARTBEAT_CMD_REQ 			( 0x0a)
#define RESET_WIFI_CMD 				( 0x0b)			//重置WIFI模组
#define RESET_WIFI_CMD_REQ 			( 0x0c)
#define WIFI_SNED_STA_CMD 			( 0x0d)			//推送wifi工作状态
#define WIFI_SNED_STA_CMD_REQ 		(0x0e)
#define MCU_RESTART_WIFI_CMD 			( 0x10)			//重启MCU
#define MCU_RESTART_WIFI_CMD_REQ 		( 0x11)
#define WIFI_GET_DEV_ATTR_CMD 		( 0x12)			//获取设备属性列表
#define WIFI_GET_DEV_ATTR_CMD_REQ 	( 0x13)
#define MCU_GET_NTP_TIME_CMD		(0x14)	//获取NTP时间
#define MCU_GET_NTP_TIME_CMD_REQ	(0x15)
#define MCU_GET_WIFI_RSSI_CMD		(0x16)	//获取模组的信号强度
#define MCU_GET_WIFI_RSSI_CMD_REQ	(0x17)

#define ACK_OK (0)
#define ACK_ERR (1)
#define _NULL_ (0)

//属性相关定义
#define TO_DEVICE (0x01)
#define FROM_DEVICE (0x00)

#define TYPE_BOOL (0x00)
#define TYPE_INT (0x01)
#define TYPE_CENTS (0x02)
#define TYPE_UTF8 (0x03)

struct ada_sprop {
	unsigned char dir;
	unsigned char site;
	unsigned char type;
	char *name;
	void *val;
	unsigned short val_len;
	void (*set)(struct ada_sprop *, const void *buf, unsigned int  len);
};

struct clock_info {
	unsigned int TimeStamp;	/* 1970年到现在的秒数 */
	unsigned int year;		/* current year */
	unsigned char month;		/* current month starting from 1 */
	unsigned char  days;		/* current day of month */
	unsigned char  day_of_week;		/* day of the week. Mon = 1, Sun = 7 */
	unsigned char  hour;		/* current hour */
	unsigned char  min;			/* current min */
	unsigned char  sec;			/* current seconds */
};

/* 属性的个数，在初始化的时候先计算属性个数 */
extern unsigned char AttrNum;	


#endif

