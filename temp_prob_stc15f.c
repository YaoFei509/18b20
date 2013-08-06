/*****************************************************
 *  温度测量
 *  
 *  STC 15F104E DS18B20@P3.7
 *  9600,n,8,1
 *
 *  Yao Fei  feiyao@me.com
 */

#include <stc12.h>

typedef unsigned char uchar;

// 仿真串口
#include "soft_uart.h"

// 温度传感器
#include "ds18b20_1t.h"

// 记录温度数据
uchar TPH, TPL;
uchar flag;   // 定时采样标志

// 小数部分
char const __code digis[16]= {0, 6, 13, 19, 
			      25, 31, 38, 44, 
			      50, 56, 63, 69,
			      75, 81, 88, 94};

// 定时器0，产生10ms中断，100次即为1s
#define FOSC  11059200
#define HZ    100
#define T0MS  (65536 - FOSC/12/HZ)

void init_timer()
{
	AUXR &= 0x7f;  // timer0 in 12T mode
	TMOD = 0;      // all timer in mode 0 
	TL0   = T0MS; 
	TH0   = T0MS >> 8; 
	TR0   = 1;
	ET0   = 1;
//	TF0   = 0;
	EA    = 1;
}

char  times = 0;
void timer0_handler() __interrupt 1  __using 2
{
	if (HZ == times++) {
		flag = 0x55;
		times = 0;
	}
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

/*
 * 主程序
 */

int main()
{
	char h, l;
	uchar hour=0, minu=0, sec=0; //时分秒

	init_timer();

	while(1) {
		if (flag) {
			P3_2 = !P3_2; // for test
			flag = 0;
			ReadTemp();

			h = (TPH<<4) + ((TPL>>4) & 0x0f);
			
			if (h<0) 
				l = digis[16 - (TPL&0xf)];
			else
				l = digis[TPL&0xf];
			
			print_num(hour);
			putchar(':');
			print_num(minu);
			putchar(':');
			print_num(sec);
			putchar('\t');
			
			if (h<0) {
				putchar('-');
				h = -h;
			}
			print_num(h);
			
			putchar('.');
			print_num(l);
			putchar('\n');
			putchar('\r');

			// update h:m:s
			if (60 == sec++) {
				sec = 0;
				if (60 == minu++) {
					minu = 0;
					if (24 == hour++) { 
						hour = 0;
					}
				}
			}
		}
	}
}

