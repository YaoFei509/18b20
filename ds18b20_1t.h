/**************************************
;WWW.STCMCU.COM
;Mobile:13922805190
;0755-82948412
温度传感器DS18B20测试程序
主芯片  : STC12C5A60S2 (1T)
工作频率: 12.000MHz
**************************************/

#ifndef __DS18B20_1T_H
#define __DS18B20_1T_H

#include <stc12.h>

typedef unsigned char BYTE;

#define DQ P3_3                    /*DS18B20的数据口位P3.3*/
extern BYTE TPH;                   //存放温度值的高字节
extern BYTE TPL;                   //存放温度值的低字节

// for Keil C compatible
#define __nop__    __asm  nop __endasm

//------------------------------------------------------
BYTE StartDS18B20();
void DelayXus(BYTE n);
BYTE DS18B20_Reset();
void DS18B20_WriteByte(BYTE dat);
BYTE DS18B20_ReadByte();
BYTE DS18B20_ReadRom(BYTE *rom);
// return value in TPH TPL
// Yao Fei
void ReadTemp();
#endif
