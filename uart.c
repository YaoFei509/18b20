#include <stc12.h>

typedef unsigned char uchar;

#include "uart.h"

// Timer2 in STC15W204S
#ifdef STC15W204S
__sfr __at(0xD6) T2H;
__sfr __at(0xD7) T2L;
__sfr __at(0xAF) IE2;
#endif

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

#endif // __SOFT_UART
	
	EA    = 1;
}


#ifndef __SOFT_UART
// for SDCC, printf
void putchar(char c) 
{
	SBUF = c;
	while(!TI) {
	};
	TI = 0;
}

#else   // SOFT UART

#define	P_TXD P3_1   /*定义模拟串口发送端,可以是任意IO*/

// for STC15F104E  use  *104
// for STC15F104W  use  *75
void	BitTime(void)
{
	unsigned int  i;
	i = ((FOSC/ 100) * 75) / 130000L - 1; //根据主时钟来计算位时间
	while(--i);
}

void putchar(char  dat)
{
	char	i;

	P_TXD = 0;    // start bit 
	BitTime();
	for(i=0; i<8; i++) {
		P_TXD = (dat & 1) ;
		dat >>= 1;
		BitTime();
	}
	P_TXD = 1;
	BitTime();  // stop 2 bit 
	BitTime();
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

