/**************************************
;WWW.STCMCU.COM
;Mobile:13922805190
;0755-82948412
�¶ȴ�����DS18B20���Գ���
��оƬ  : STC12C5A60S2 (1T)
����Ƶ��: 12.000MHz
**************************************/

#ifndef __DS18B20_1T_H
#define __DS18B20_1T_H

#include <8052.h>
//#include "STC15fxxxx.h"

typedef unsigned char BYTE;

#define DQ P3_3                     /*DS18B20�����ݿ�λP3.3*/
extern BYTE TPH;                           //����¶�ֵ�ĸ��ֽ�
extern BYTE TPL;                           //����¶�ֵ�ĵ��ֽ�

// for Keil C compatible
#define __nop__    __asm  nop __endasm


void DelayXus(BYTE n);
void DS18B20_Reset();
void DS18B20_WriteByte(BYTE dat);
BYTE DS18B20_ReadByte();

int  ReadTemp();


#endif
