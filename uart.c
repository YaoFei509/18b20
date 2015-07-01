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

void init_uart()
{

#ifndef STC15F104 // soft uart	
	AUXR = 0;

	SCON  = 0x50;  // SCON mode 1, 8bit enable ucvr

#ifdef STC11F04E       // 有独立波特率发生器BRT
	PCON |= 0x80;  // SMOD = 1
	AUXR  = 0x13;  // enable BRTR, ExtRAM, select BRTR

	BRT   = 0xFD;  // Baud Rate Timer 9600
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
	IE   |= 0x90;  // enable serial interrupt 

#endif // STC15F104
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
#else   // SOFT UART

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

