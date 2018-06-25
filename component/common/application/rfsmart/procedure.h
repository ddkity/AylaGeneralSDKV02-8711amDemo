#ifndef __PROCEDURE_H__
#define __PROCEDURE_H__

/* 1�Ǵ�log��0�ǹر�log */
#define DEBUG_LEVEL (1)
#define RF_PRINTF(level, fmt, arg...)     \
do {\
	if (level > 0) {\
		printf(fmt, ##arg);\
	}\
}while(0)
#define PRINTF(...)      RF_PRINTF(DEBUG_LEVEL, __VA_ARGS__)


/* ����֡�ṹ���� */
#define F_HEAD_H  		(0xFF)	/* ֡ͷ */
#define F_HEAD_L  		 (0xFF)	/* ֡β */
#define UARTNOP  			(0)           /*  ���ڽ��մ�������	*/
#define UARTSOP_H  		(1)           /*  ������ʼλ		*/
#define UARTSOP_L  		(2)           /*  ������ʼλ		*/
#define UARTLEN_H  		(3)           /*  ���ճ���			*/
#define UARTLEN_L  		(4)           /*  ���ճ���			*/
#define UARTCMD  		(5)           /*  ��������			*/
#define UARTDATA 		(6)           /*  ��������			*/
#define UARTCRC  			(7)           /*  ����У��� 		*/

#define SENDMAXLEN (1088)
#define RECVMAXLEN (1088)


/* ͨѶ���� */
#define GET_DEV_INFO_CMD 			(0x01)			//��ȡ�豸��Ϣ
#define GET_DEV_INFO_CMD_REQ 		(0x02)
#define WIFI_CTRL_DEV_CMD 			( 0x03)			//WIFIģ������豸
#define WIFI_CTRL_DEV_CMD_REQ 		( 0x04)
#define WIFI_READ_DEV_STA_CMD 		( 0x05)			//WIFIģ���ȡ�豸״̬
#define WIFI_READ_DEV_STA_CMD_REQ  	(0x06)
#define MCU_UP_PROPERTY_CMD		(0x07)			//MCU�����ϱ���ǰ״̬
#define MCU_UP_PROPERTY_CMD_REQ	( 0x08)
#define HEARTBEAT_CMD 				( 0x09)			//��������
#define HEARTBEAT_CMD_REQ 			( 0x0a)
#define RESET_WIFI_CMD 				( 0x0b)			//����WIFIģ��
#define RESET_WIFI_CMD_REQ 			( 0x0c)
#define WIFI_SNED_STA_CMD 			( 0x0d)			//����wifi����״̬
#define WIFI_SNED_STA_CMD_REQ 		(0x0e)
#define MCU_RESTART_WIFI_CMD 			( 0x10)			//����MCU
#define MCU_RESTART_WIFI_CMD_REQ 		( 0x11)
#define WIFI_GET_DEV_ATTR_CMD 		( 0x12)			//��ȡ�豸�����б�
#define WIFI_GET_DEV_ATTR_CMD_REQ 	( 0x13)
#define MCU_GET_NTP_TIME_CMD		(0x14)	//��ȡNTPʱ��
#define MCU_GET_NTP_TIME_CMD_REQ	(0x15)
#define MCU_GET_WIFI_RSSI_CMD		(0x16)	//��ȡģ����ź�ǿ��
#define MCU_GET_WIFI_RSSI_CMD_REQ	(0x17)

#define ACK_OK (0)
#define ACK_ERR (1)
#define _NULL_ (0)

//������ض���
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
	unsigned int TimeStamp;	/* 1970�굽���ڵ����� */
	unsigned int year;		/* current year */
	unsigned char month;		/* current month starting from 1 */
	unsigned char  days;		/* current day of month */
	unsigned char  day_of_week;		/* day of the week. Mon = 1, Sun = 7 */
	unsigned char  hour;		/* current hour */
	unsigned char  min;			/* current min */
	unsigned char  sec;			/* current seconds */
};

/* ���Եĸ������ڳ�ʼ����ʱ���ȼ������Ը��� */
extern unsigned char AttrNum;	


#endif

