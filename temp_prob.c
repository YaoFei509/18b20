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
 *  自动温度采集探头
 *  
 *  STC89C52       DS18B20@P3.7
 *  STC11F04E      DS18B20@P3.3 (Pin 7)
 *  STC15W204S     DS18B20@P3.3
 *  STC15F104(E/W) DS18B20@P3.3 
 *
 *  支持最多并联四个DS18B20
 *
 *  9600,n,8,1 
 *  每秒输出一次时标、测温值、ROM地址、时间(x10ms)
 *
 *  Yao Fei  feiyao@me.com
 */

#include <stc12.h>

#include "uart.h"
#include "ds18b20.h"
#include "ds18b20_search.h"

typedef unsigned char uchar;

// for Keil C compatible
#define __nop__    __asm  nop __endasm

// 小数部分  1/16 = 0.0625 
char const __code digis[16]= {0, 6, 13, 19, 
			      25, 31, 38, 44, 
			      50, 56, 63, 69,
			      75, 81, 88, 94};

char flag;   // 是否采样标志
// 采集到的温度
char TPH, TPL;

// ---------------  定时器 -------------------------
// 初始化定时器
#define HZ    100
#define T0MS  (65536 - FOSC/12/HZ)

void init_timer0()
{
#ifdef STC11F04E
	TMOD = 1;  // 标准8051 模式1,16比特
#else
	TMOD = 0;  // Timer0 for 100Hz
		   // Mode 0, 16bit auto load
#endif
	TL0 = T0MS & 255;
	TH0 = T0MS>>8;
	TR0   = 1;
	ET0   = 1;
}

// Timer 0 handler
uchar times = 0;
void timer0() __interrupt 1 __using 2
{
#ifdef STC11F04E  // 标准8051不能自动装载
	TL0 = T0MS & 255;
	TH0 = T0MS>>8;
#endif
	times++;
	if (HZ == times) {
		flag = 0x55;
		times = 0;
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
uchar hour=0, minu=0, sec=0; //时分秒

int main()
{
	char h, l;
	char num, i;

	// 先初始化串口
	init_uart();

	// 找不到DS18B20则一直显示S
	while ((num = DS18B20_Search()) == 0) {
		putchar('S');
	}

	// 忽略掉上电时第一个假的测温值85度
	StartDS18B20();

	// 初始化定时器
	init_timer0();
	
	while(1) {
		// 恶心的前后台，中断触发标志
		if (flag) {
			flag = 0;
			if (0 == StartDS18B20()) { //启动所有DS18B20测温
				for (i=0;i<num;i++) {
                                        // 先打印时标
					print_num(hour);
					putchar(':');
					print_num(minu);
					putchar(':');
					print_num(sec);
					putchar('\t');

					//读温度
					ReadTemp(DS18B20_ROM[i]);

					//组合高低位
					h = (TPH<<4) + ((TPL>>4) & 0x0f);
					
					// 用查表算法，省略*0.0625运算
					if (h<0) 
						l = digis[16 - (TPL&0xf)];
					else
						l = digis[TPL&0xf];
					
					// 正负号
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
						print_hex(DS18B20_ROM[i][l]);
						if (7==l)
							putchar('\t');
						else
							putchar(':');
					}

					// 打印此时的定时器时间，估算运行时间
					print_num(times);
					
					putchar('\n');
					putchar('\r');
					
				} // for i<num
			}
			
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
