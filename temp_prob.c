/**
*　　　　　　　　┏┓　　　┏┓+ +
*　　　　　　　┏┛┻━━━┛┻┓ + +
*　　　　　　　┃　　　　　　　┃
*　　　　　　　┃　　　━　　　┃ ++ + + +
*　　　　　　　┃　████━████ ┃+
*　　　　　　　┃　　　　　　　┃ +
*　　　　　　　┃　　　┻　　　┃
*　　　　　　　┃　　　　　　　┃ + +
*　　　　　　　┗━┓　　　┏━┛
*　　　　　　　　　┃　　　┃
*　　　　　　　　　┃　　　┃ + + + +
*　　　　　　　　　┃　　　┃　　　　Code is far away from bug
*　　　　　　　　　┃　　　┃　　　　with the animal protecting
*　　　　　　　　　┃　　　┃ + 　　　　神兽保佑,代码无bug
*　　　　　　　　　┃　　　┃
*　　　　　　　　　┃　　　┃　　+
*　　　　　　　　　┃　 　　┗━━━┓ + +
*　　　　　　　　　┃ 　　　　　　　┣┓
*　　　　　　　　　┃ 　　　　　　　┏┛
*　　　　　　　　　┗┓┓┏━┳┓┏┛ + + + +
*　　　　　　　　　　┃┫┫　┃┫┫
*　　　　　　　　　　┗┻┛　┗┻┛+ + + +
*/

/*****************************************************
 *  温度测量
 *  
 *  STC89C52 DS18B20@P3.7
 *  STC11F04E DS18B20@P3.3 (Pin 7)
 *
 *  9600,n,8,1
 *
 *  Yao Fei  feiyao@me.com
 */
#include <stc12.h>

typedef unsigned char uchar;

// 11F04E 是1T单片机
#ifdef STC11F04E
#include "ds18b20_1t.h"
#else
#include "ds18b20.h"
#endif


uchar flag;   // 是否采样标志

// 小数部分
char const __code digis[16]= {0, 6, 13, 19, 
			      25, 31, 38, 44, 
			      50, 56, 63, 69,
			      75, 81, 88, 94};
// for Keil C compatible
#define __nop__    __asm  nop __endasm

#ifdef STC11F04E
// 11F04E主频只有5.66MHz
#define FOSC  5660000
#else
// 89RC52是11.0592MHz
#define FOSC  11059200
#endif

#define HZ    100
#define T0MS  (65536 - FOSC/12/HZ)

// 采集到的温度
uchar TPH, TPL;
uchar rom[4][8];  //Max 4 DS18B20

// 初始化串口和定时器
void init_uart()
{
	AUXR = 0;

	SCON  = 0x50;  // SCON mode 1, 8bit enable ucvr
	TMOD |= 0x21;

#ifdef STC11F04E       // 有独立波特率发生器BRT
	PCON |= 0x80;  // SMOD = 1
	AUXR  = 0x13;  // enable BRTR, ExtRAM, select BRTR

	BRT   = 0xFD;  // Baud Rate Timer 9600
#else
	PCON |= 0x00;  // SMOD = 0 

	TH1   = 0xFD;  // 9600
	TL1   = 0xFD;  // 9600 
	TR1   = 1;
#endif
	
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

char* const __code  hexchar="023456789ABCDEF";

// UART interrupt handler
void serial() __interrupt 4 __using 3
{
	uchar c;
	
	if (RI) {
		c = SBUF;
		RI = 0;

		if ( 't' == c ) 
			flag = 0x55;
		else
			flag = 0;
	} 
}

// Timer 0 handler
uchar t0count =0;
void timer0() __interrupt 1 __using 2
{
	TR0 = 0;
	TL0 = T0MS;
	TH0 = T0MS>>8;
	TR0 = 1;

	t0count++;
	if( HZ == t0count) {
		flag = 0x55;
		t0count = 0;
	}

	P1_7 = !P1_7; // for test
}


// 打印十进制数字
void print_num(unsigned char dat)
{
	char i;

	for (i=0;i<3;i++) {
		if (dat < 100) 
			break;
		dat -= 100;
	}

	if (i) 
		putchar('0'+i);

	for (i=0;i<10;i++) {
		if (dat < 10) 
			break;
		dat -= 10;
	}

	putchar('0'+i);
	putchar('0'+dat);
}

// 打印16进制数据
void print_hex(char data)
{
	putchar(hexchar[(data >> 4) & 0x0f]);
	putchar(hexchar[data & 0x0f]);
}


/*
 * 主程序
 */
int main()
{
	char h, l;
	uchar hour=0, minu=0, sec=0; //时分秒

	init_uart();

	if (0 == StartDS18B20()) {
		DS18B20_ReadRom(rom[0]);
	}
	
	while(1) {
		// 恶心的前后台，中断触发标志
		if (flag) {
			flag = 0;

			h = StartDS18B20();
			
			// 先打印时标
			print_num(hour);
			putchar(':');
			print_num(minu);
			putchar(':');
			print_num(sec);
			putchar('\t');

			if (h==0) {
				ReadTemp(rom[0]);
				
				h = (TPH<<4) + ((TPL>>4) & 0x0f);
				
				if (h<0) 
					l = digis[16 - (TPL&0xf)];
				else
					l = digis[TPL&0xf];
				
			
				if (h<0) {
					putchar('-');
					h = -h;
				}
				print_num(h);
				
				putchar('.');
				print_num(l);

				putchar('\t');
				
				// print ROM
				for (l=0; l<8; l++) {
					print_hex(rom[0][l]);
					if (7==l)
						putchar('\t');
					else
						putchar(':');
				}
			}
			
			putchar('\n');
			
			// update h:m:s
			if (59 == sec++) {
				sec = 0;
				if (59 == minu++) {
					minu = 0;
					if (23 == hour++) { 
						hour = 0;
					}
				}
			}
		}
	}
}

