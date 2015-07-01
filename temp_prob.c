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
 *  STC15W204S DS18B20@P3.3
 *  STC15F104(E/W) DS18B20@P3.3 
 *
 *  9600,n,8,1
 *
 *  Yao Fei  feiyao@me.com
 */
#include <stc12.h>

#include "uart.h"

typedef unsigned char uchar;

// 11F04E 是1T单片机
#if (defined STC11F04E) || (defined STC15W204S) || (defined STC15F104 )
#include "ds18b20_1t.h"
#else
#include "ds18b20.h"
#endif


// for Keil C compatible
#define __nop__    __asm  nop __endasm


#define HZ    100
#define T0MS  (65536 - FOSC/12/HZ)

// 小数部分
char const __code digis[16]= {0, 6, 13, 19, 
			      25, 31, 38, 44, 
			      50, 56, 63, 69,
			      75, 81, 88, 94};

uchar flag;   // 是否采样标志
// 采集到的温度
uchar TPH, TPL;
uchar rom[4][8];  //Max 4 DS18B20

// 初始化串口和定时器
void init_timer0()
{
	TMOD |= 0x01;  // Timer0 for 100Hz
	TL0 = T0MS & 255;
	TH0 = T0MS>>8;
	TR0   = 1;
	ET0   = 1;
}

#ifndef STC15F104
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
#endif

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

#ifdef STC15F104
	P3_2 = !P3_2;
#else
	P1_1 = !P1_1; // for 100Hz test
#endif
}


/*
 * 主程序
 */
int main()
{
	char h, l;
	uchar hour=0, minu=0, sec=0; //时分秒

	init_uart();
	init_timer0();

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

