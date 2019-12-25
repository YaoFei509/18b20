#include <stc12.h>

typedef unsigned char uchar;

#include "uart.h"

// Timer2 in STC15W204S
#ifdef STC15W204S
__sfr __at(0xD6) T2H;
__sfr __at(0xD7) T2L;
__sfr __at(0xAF) IE2;
#endif

#ifndef __SOFT_UART
// for SDCC, printf
void putchar(char c) 
{
	SBUF = c;
	while(!TI) {
	};
	TI = 0;
}

char getchar()
{
	while(!RI) {};
	RI=0;
	return SBUF;
}

#else // SOFT UART

//定义串口 TX/RX 引脚
#define RXD P3_0 
#define TXD P3_1 

char TBUF,RBUF;
char TDAT,RDAT;
char TCNT,RCNT;
char TBIT,RBIT;
__bit TING,RING;
__bit TEND,REND;

//-----------------------------------------
//Timer interrupt routine for UART

void timer0_handler() __interrupt 1 __using 2
{
	if (RING) {
		if (--RCNT == 0) {
			RCNT = 3;                      //重置发送波特率计数器
			if (--RBIT == 0) {
				RBUF = RDAT;            //保存数据到RBUF
				RING = 0;               //停止接收
				REND = 1;               //设置接收完成标志
			} else {
				RDAT >>= 1;
				if (RXD) {
					RDAT |= 0x80; //移位RX数据到RX缓冲
				}
			}
		}
	} else if (!RXD) {
		RING=1;                       //设置开始接收标志
		RCNT=4;                       //初始化接收波特率计计数器
		RBIT=9;                       //初始化接收位数(8数据位+1停止位)
	}
	
	if (--TCNT == 0) {
		TCNT=3;                       //重置发送波特率计数器
		if (TING) {                   //判断是否发送
			if (TBIT == 0)	{
				TXD  = 0;               //发送开始位
				TDAT = TBUF;            //从TBUF加载数据到TDAT
				TBIT = 9;               //初始化发送位数(8数据位+1停止位)
			} else {
				TDAT >>= 1;           //移位数据到CY
				if (--TBIT == 0)
				{
					TXD  = 1;
					TING = 0;           //停止发送
					TEND = 1;           //设置发送完成标志
				} else {
					TXD = CY;           //写CY到TX脚
				}
			}
		}
	}

}

//-----------------------------------------
//发送串口数据
void putchar(char dat)
{
	while (!TEND) {};
	TBUF=dat;
	TEND=0;
	TING=1;
}

//-----------------------------------------
//接受串口数据
char getchar()
{
	while (!REND) {};
	REND = 0;

	return RBUF;
}

#endif

// 打印十进制数字
void print_num(unsigned char dat)
{
	char i;

	if (dat > 100) {
		i = dat / 100;
		dat %=100;
		putchar('0'+i);
	}

	i = dat/10;
	dat %= 10;

	putchar('0'+i);
	putchar('0'+dat);
}

// 打印16进制数据
char* const __code  hexchar="0123456789ABCDEF";
void print_hex(char data)
{
	putchar(hexchar[(data >> 4) & 0x0f]);
	putchar(hexchar[data & 0x0f]);
}


/* 9600bps @ 11.0592MHz */
#define BAUDS 0xFE80

void init_uart()
{
#ifndef __SOFT_UART
	AUXR = 0;
	SCON  = 0x50;  // SCON mode 1, 8bit enable ucvr

#ifdef STC11F04E       // 有独立波特率发生器BRT

	PCON |= 0x80;  // SMOD = 1
	AUXR  = 0x03;  // enable BRTR, ExtRAM, select BRTR

	BRT   = 0xFD;  // Baud Rate Timer 9600

	AUXR |= 0x10;

#elif defined STC15W204S

	// 15W204S没有Timer1, 用Timer2做波特率发生器
	T2L   = (65536 - (FOSC/4/BAUD));
	T2H   = (65536 - (FOSC/4/BAUD)) >> 8 ;
	AUXR  |= 0x14;  // T2 in 1T
	AUXR  |= 0x01;  // select T2 as baud

#else

	// Normal 8031
	PCON |= 0x00;  // SMOD = 0 
	TMOD |= 0x20;  // Timer1 as baud

	TH1   = 0xFD;  // 9600
	TL1   = 0xFD;  // 9600 
	TR1   = 1;
#endif

#else   // __SOFT_UART
	
	TMOD = 0x00;                        //定时器0在16位自动重装模式
	TL0  = (char)BAUDS;
	TH0  = BAUDS>>8;                    //初始化定时器0并设定重装值
	AUXR |= 0x80;                       //定时器0在1T模式
	TR0  = 1;                           //定时器0开始运行
	ET0  = 1;                           //使能定时器0中断
	PT0  = 1;                           //提高定时器0的优先级

	TING = 0;
	RING = 0;
	TEND = 1;
	REND = 0;
	TCNT = 0;
	RCNT = 0;
	
#endif // __SOFT_UART
	
	EA    = 1;
}
