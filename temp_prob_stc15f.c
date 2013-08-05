/*****************************************************
 *  温度测量
 *  
 *  STC 15F104E DS18B20@P3.7
 *  9600,n,8,1
 *
 *  Yao Fei  feiyao@me.com
 */
//#include <8052.h>
#include <stc12.h>
#include <stdio.h>

#include "soft_uart.h"
#include "ds18b20_1t.h"

BYTE TPH, TPL;

typedef unsigned char uchar;

BYTE  flag;   // 是否采样标志

char const hexs[] = "0123456789ABCDEF";

// 小数部分
char* const digis[16]= {"0", "06", "13", "19", 
			"25", "31", "38", "44", 
			"5",  "56", "63", "69",
			"75", "81", "88", "94"};


#define FOSC  11059200
#define HZ    100
#define T0MS  (65536 - FOSC/12/HZ)

//__sfr AUXR = 0x8e; // STC 

void init_timer()
{
	AUXR &= 0x7f;  // timer0 in 12T mode


	TMOD = 0;      // all timer in mode 0 

	TL0   = T0MS; 
	TH0   = T0MS >> 8; 

	TR0   = 1;
	ET0   = 1;
	EA    = 1;
}

// for SDCC, printf
void putchar(char c) 
{
	TxSend(c);
}

char  times = 0;

void timer0_handler() __interrupt __using 1
{
	times++;

	if (HZ == times) {
		flag = 0x55;
		times = 0;
	}

	P3_6 = !P3_6; // for test
}


/*
 *
 */

int main()
{
	unsigned int read_temp;
	char h, l;
	unsigned int count=0;

	init_timer();

	while(1) {
		if (flag) {
			flag = 0;
#if 1
			read_temp = ReadTemp();			
			h = (TPH<<4) + ((TPL>>4) & 0x0f);
			
			if (h<0) 
				l = 16 - (TPL&0xf);
			else
				l = TPL&0xf;
#else 
			h = 1; l=1;
#endif
			
			printf("%u\t%d.%s\n\r", count++,  h, digis[l]);
		}
	}
}		
