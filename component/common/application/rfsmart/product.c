#include "product.h"
#include "procedure.h"
#include <stdio.h>
#include <string.h>

#include "device.h"
#include "serial_api.h"
#include "main.h"
#include "timer_api.h"

serial_t	sobj;
gtimer_t RFTimer0;
#define UART_TX    PA_7
#define UART_RX    PA_6

/* 设备信息定义 */
char hardversion[]="1-0-0";
char softversion[]="1-0-3";
char host_version[] = "rfsmart-01";
char oem_model[]="ledevb";
char oem_id[]="54505428";
char OEM_AP_SSID_PREFIX[]="Ayla-";

unsigned char blue_button;
unsigned char blue_led;
unsigned char green_led;
char cmd_buf[1024]; 
int input;
int output;
int decimal_in;
int decimal_out;

char example_bool_flag = 0;
char example_int_flag = 0;
char example_decimal_flag = 0;
char example_string_flag = 0;

void userhandler(void)
{
	if(example_bool_flag == 0x01)
	{
		SendAttrToCloud("Blue_button", &blue_button, sizeof(blue_button), TYPE_BOOL);
		example_bool_flag = 0x00;
	}

	if(example_int_flag == 0x01)
	{
		SendAttrToCloud("output", &output, sizeof(output), TYPE_INT);
		example_int_flag = 0x00;
	}

	if(example_decimal_flag == 0x01)
	{
		SendAttrToCloud("decimal_out", &decimal_out, sizeof(decimal_out), TYPE_CENTS);
		example_decimal_flag = 0x00;
	}

	if(example_string_flag == 0x01)
	{
		SendAttrToCloud("log", &cmd_buf, strlen(cmd_buf), TYPE_UTF8);
		example_string_flag = 0x00;
	}
}

void boolproperty_set(struct ada_sprop *sprop, const void *buf, unsigned int len)
{
	//user handler
	PRINTF("[%s] set to [%d], len = %d\n",  sprop->name, *(unsigned char *)sprop->val, len);
	if(0 == memcmp("Blue_LED", sprop->name, strlen(sprop->name)))
	{
		blue_button = blue_led;
	}else if(0 == memcmp("Green_LED", sprop->name, strlen(sprop->name)))
	{
		blue_button = !green_led;
	}
	example_bool_flag = 0x01;
}

void intproperty_set(struct ada_sprop *sprop, const void *buf, unsigned int len)
{
	//user handler
	PRINTF("[%s] set to [%d], len = %d\n",  sprop->name, *(int *)sprop->val, len);
	output = input;
	example_int_flag = 0x01;
}

void decimalproperty_set(struct ada_sprop *sprop, const void *buf, unsigned int len)
{
	//user handler
	PRINTF("[%s] set to [%d], len = %d\n",  sprop->name, *(int *)sprop->val, len);
	decimal_out = decimal_in;
	example_decimal_flag = 0x01;
}

void string_cmdproperty_set(struct ada_sprop *sprop, const void *buf, unsigned int len)
{
	//user handler
	PRINTF("[%s] set to [%s], len = %d\n",  sprop->name, (char *)sprop->val, len);
	example_string_flag = 0x01;
}

/* 所有属性的定义:属性方向，属性位置(一定要按顺序从0开始排列)，属性类型，属性名，属性变量，属性大小，属性对应的回调函数 */
struct ada_sprop demo_props[] = {
	/* bool properties */
	{FROM_DEVICE, 0, TYPE_BOOL, "Blue_button", &blue_button, sizeof(blue_button), NULL},
	{TO_DEVICE, 1, TYPE_BOOL, "Blue_LED", &blue_led, sizeof(blue_led), boolproperty_set},
	{TO_DEVICE, 2, TYPE_BOOL, "Green_LED", &green_led, sizeof(green_led), boolproperty_set},

	/* string properties */
	{TO_DEVICE, 3, TYPE_UTF8, "cmd", cmd_buf, sizeof(cmd_buf), string_cmdproperty_set},
	{FROM_DEVICE, 4, TYPE_UTF8, "log", cmd_buf, sizeof(cmd_buf), NULL},

	/* integer properties */
	{TO_DEVICE, 5, TYPE_INT, "input", &input, sizeof(input), intproperty_set},
	{FROM_DEVICE, 6, TYPE_INT, "output", &output, sizeof(output), NULL},

	/* decimal properties */
	{TO_DEVICE, 7, TYPE_CENTS, "decimal_in", &decimal_in, sizeof(decimal_in), decimalproperty_set},
	{FROM_DEVICE, 8, TYPE_CENTS, "decimal_out", &decimal_out, sizeof(decimal_out), NULL},
};

/*******************************************************************************************
*                    			  start  串口和定时器的初始胡                             *
*******************************************************************************************/

/* 串口接收中断数据接收接口 */
void RFUartIrq(uint32_t id, SerialIrq event)
{
	unsigned char RecvCharTemp = 0;
	
	if(event == RxIrq)
	{
		RecvCharTemp= serial_getc(&sobj);
		UartHandler(RecvCharTemp);
	}
}

/* 定时器中断接口，100ms中断一次 */
void RFTimer0Handler(unsigned int TimeOut)
{
	UartErrorRecvTimeout();
}

/* 串口和定时器等外设的初始化，设备信息和属性列表初始化 */
void UartInit(void)
{
	AttrNum = sizeof(demo_props)/sizeof(demo_props[0]);	/* 计算属性个数 */

	/* 1、初始化串口 */
	serial_init(&sobj,UART_TX,UART_RX);
	serial_baud(&sobj,9600);
	serial_format(&sobj, 8, ParityNone, 1);
	serial_irq_handler(&sobj, RFUartIrq, (uint32_t)&sobj);
	serial_irq_set(&sobj, RxIrq, 1);
	serial_irq_set(&sobj, TxIrq, 1);

	/* 2、定时器初始化	100ms进入一次定时器中断 */
	gtimer_init(&RFTimer0, TIMER0);
	gtimer_start_periodical(&RFTimer0, 100000, (void*)RFTimer0Handler, NULL);

	/* 3、所有属性值的初始化 */
	blue_button = 0;
	blue_led = 1;
	green_led = 0;
	memcpy(cmd_buf, "hello world", sizeof("hello world")); 
	input = 65534;
	output = -7895;
	decimal_in = 4512;	/* 小数的100倍 */
	decimal_out = 9886; /* 小数的100倍 */
}

/* 1ms 延时函数实现 */
void MCUDelayMs(unsigned int ms)
{
	int i = 0;
	
	for(i = 0; i < ms; i++){
		HalDelayUs(1000);
	}
}

/************************************* 
*	串口发送数据 
CMDTem:			发送的命令
SendFormData:	发送的真正的数据，Data域
Len:			SendFormData的长度
*************************************/
void UartSendFormData(unsigned char CMDTem, const unsigned char *SendFormData, unsigned short Len)
{
	static unsigned char SendDataTem[SENDMAXLEN + 6] = {0};	/* 这里需要把变量定义为static，否则单片机栈空间不够大的话会导致栈溢出 */
	unsigned short SendDataLen = 0;
	unsigned char CRCTem = 0;
	unsigned short i;
	
	SendDataTem[SendDataLen++] = F_HEAD_H;					//包头
	SendDataTem[SendDataLen++] = F_HEAD_L;					//包头

	Len = Len + 6;
	SendDataTem[SendDataLen++] = (Len >> 8) & 0xFF;		//包长高8位
	CRCTem += SendDataTem[SendDataLen - 1];

	SendDataTem[SendDataLen++] = Len & 0xFF;		//包长低8位
	CRCTem += SendDataTem[SendDataLen - 1];
	
	SendDataTem[SendDataLen++] = CMDTem;					//功能码
	CRCTem += SendDataTem[SendDataLen - 1];
	
	for(i = 0; i < Len - 6; i++){
		SendDataTem[SendDataLen++] = SendFormData[i];		//数据
		CRCTem += SendDataTem[SendDataLen - 1];
	}

	SendDataTem[SendDataLen++] = CRCTem;					//校验和
	/* 开始发送数据 */
	if(Len < 64){
		PRINTF("Send Data:");
		for(i = 0; i < Len; i++)
		{
			PRINTF("%x ", SendDataTem[i]);
		}
		PRINTF("\n");
	}else{
		PRINTF("Send %d Data.\n", Len);
	}


	MCUDelayMs(20);		/* 加个延时，防止有数据下发的时候马上回传到wifi模组导致模组丢数据 */
	for(i = 0; i < Len; i++)
	{
		/* 串口发送数据接口 */
		serial_putc(&sobj, SendDataTem[i]);
	}
	memset(SendDataTem, 0x00, SENDMAXLEN + 6);
}
/*******************************************************************************************
*                    			  end  串口和定时器的初始胡                                *
*******************************************************************************************/

