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
 *  串口参数: 9600,n,8,1
 *  指令控制版。从串口接受一个字符命令，输出不同的数据
 *  N 输出传感器个数
 *  T 输出一次“ROM地址 测温值”
 *  R 复位传感器
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
// 采集到的温度
char TPH, TPL;

void Disp_Temp(int num)
{
	char h, l, i;
	
	if (0 == StartDS18B20()) { //启动所有DS18B20测温
		for (i=0;i<num;i++) {
			// print ROM
			for (l=0; l<8; l++) {
				print_hex(DS18B20_ROM[i][l]);
				if (7==l)
					putchar('\t');
				else
					putchar(':');
			}
						
			//读温度
			ReadTemp(DS18B20_ROM[i]);
						
			//组合高低位
			h = (TPH<<4) + ((TPL>>4) & 0x0f);
						
			// 用查表算法，省略*0.0625运算
			if (h > 127) 
				l = digis[16 - (TPL&0xf)];
			else
				l = digis[TPL&0xf];
						
			// 正负号
			if (h > 127 ) {
				putchar('-');
				h = -h;
			}
			print_num(h);						
			putchar('.');
			print_num(l);
						
			putchar('\r');
			putchar('\n');
						
		} // for i<num
	}
}


/*
 * 主程序
 */
int main()
{
	char num;
	char cmd;    // 串口字符

	// 先初始化串口
	init_uart();

	// 找不到DS18B20则一直显示S
	while ((num = DS18B20_Search()) == 0) {
		putchar('S');
	}

	// 忽略掉上电时第一个假的测温值85度
	StartDS18B20();

	Disp_Temp(num);
	
	while(1) {
		puts("Press R/N/T:");
		if (cmd = getchar()) {
			switch (cmd) {
			case 'R':
			case 'r': // reset
				StartDS18B20();
				puts("\r\n");
				break;

			case 'n':
			case 'N': // get number of 18B20
				print_num(num);
				puts("\r\n");
				break;
				
			case 'T':
			case 't':
				Disp_Temp(num);
				break;

			default:
				puts("Unknown.\r\n");
				break;
			}
		} //if (cmd
	} //while
} //main
