#include <8052.h>
#include <stdio.h>

#define __STC89C52

#ifdef __STC89C52
// for STC 89C52RD
__sfr AUXR  = 0x8E;
#endif

typedef unsigned char uchar;
typedef unsigned char BYTE;

BYTE  flag;

unsigned char const __data hexs[] = "0123456789ABCDEF";

#define __nop__    __asm  nop __endasm

void init_uart()
{
#ifdef __STC89C52
	AUXR = 0;
#endif

	SCON  = 0x50;  // SCON mode 1, 8bit enable ucvr
//	TMOD |= 0x20;  // timer1, mode 2, 8bit reload

	TMOD |= 0x21;

	PCON |= 0x00;  // SMOD =1 

	TH1   = 0xFD;  // 9600
	TL1   = 0xFD;  //9600 
	TR1   = 1;

	TR0   = 1;
	ET0   = 1;
	IE   |= 0x90;  // enable serial interrupt 
	EA    = 1;
}

// for SDCC, printf
void putchar(char c) 
{
	SBUF = c;
	while(!TI) {
	};
	TI = 0;
}

// UART interrupt handler
void serial() __interrupt 4 __using 3
{
	BYTE c;
	
	if (RI) {
		c = SBUF;
		RI = 0;

		if (c == 't') 
			flag = 0x55;
		else
			flag = 0;
	} 
}

// Timer 0 handler
BYTE t0count =0;
void timer0() __interrupt 1 __using 2
{

	TR0 = 0;
	TL0 = 0;
	TH0 = 0x4C;
	TR0 = 1;
	
	t0count++;
	
	if( 20 == t0count) {
		flag = 0x55;
		t0count = 0;
	}
}

// -------------------   DS18B20  --------------------------
#define DQ P3_7

BYTE TPH, TPL;

void DelayXus(BYTE n);
void DS18B20_Reset();
void DS18B20_WriteByte(BYTE dat);
BYTE DS18B20_ReadByte();

unsigned int ReadTemp()
{
	unsigned int v;
	
	DS18B20_Reset();                //设备复位
	DS18B20_WriteByte(0xCC);        //跳过ROM命令
	DS18B20_WriteByte(0x44);        //开始转换命令
	while (!DQ);                    //等待转换完成
	
	DS18B20_Reset();                //设备复位
	DS18B20_WriteByte(0xCC);        //跳过ROM命令
	DS18B20_WriteByte(0xBE);        //读暂存存储器命令
	TPL = DS18B20_ReadByte();       //读温度低字节
	TPH = DS18B20_ReadByte();       //读温度高字节

	v = TPH*256+TPL;
	return v;
}

/**************************************
延时X*10微秒(STC90C52RC@12M)
不同的工作环境,需要调整此函数
当改用1T的MCU时,请调整此延时函数
**************************************/
void DelayX0us(BYTE n)
{
	while (n--)
	{
		__nop__;
		__nop__;
	}
}

/**************************************
复位DS18B20,并检测设备是否存在
**************************************/
void DS18B20_Reset()
{
	CY = 1;
	while (CY)
	{
		DQ = 0;                     //送出低电平复位信号
		DelayX0us(48);              //延时至少480us
		DQ = 1;                     //释放数据线
		DelayX0us(6);               //等待60us
		CY = DQ;                    //检测存在脉冲
		DelayX0us(42);              //等待设备释放数据线
	}
}

/**************************************
从DS18B20读1字节数据
**************************************/
BYTE DS18B20_ReadByte()
{
	BYTE i;
	BYTE dat = 0;

	for (i=0; i<8; i++)             //8位计数器
	{
		dat >>= 1;
		DQ = 0;                     //开始时间片
		__nop__;                    //延时等待
		__nop__;
		DQ = 1;                     //准备接收
		__nop__;                    //接收延时
		__nop__;
		if (DQ) dat |= 0x80;        //读取数据
		DelayX0us(6);               //等待时间片结束
	}

	return dat;
}

/**************************************
向DS18B20写1字节数据
**************************************/
void DS18B20_WriteByte(BYTE dat)
{
	char i;

	for (i=0; i<8; i++)             //8位计数器
	{
		DQ = 0;                     //开始时间片
		__nop__;                    //延时等待
		__nop__;
		dat >>= 1;                  //送出数据
		DQ = CY;
		DelayX0us(6);               //等待时间片结束
		DQ = 1;                     //恢复数据线
	}
}


/*
 *
 */

int main()
{
	unsigned int read_temp;
	BYTE h;
	int  l;
	int count=0;

	init_uart();

	while(1) {
		if (flag) {
			flag = 0;
			read_temp = ReadTemp();
			
			l = (int)(TPL & 0x0f)*625;
			h = TPL/16 + TPH*16;
			printf("%d\t%d.%04d\n\r", count++,  h, l);
		}
	}
}		
