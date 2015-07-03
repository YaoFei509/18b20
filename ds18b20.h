#ifndef __DS18B20_H
#define __DS18B20_H

#include <stc12.h>

typedef unsigned char BYTE;

#define DQ P3_3                    /*DS18B20的数据口位P3.3*/
extern char TPH;                   //存放温度值的高字节
extern char TPL;                   //存放温度值的低字节

// for Keil C compatible
#define __nop__    __asm  nop __endasm

//------------------------------------------------------
BYTE StartDS18B20();
void DelayXus(BYTE n);
BYTE DS18B20_Reset();
void DS18B20_WriteByte(BYTE dat);
BYTE DS18B20_ReadByte();
BYTE DS18B20_ReadRom(BYTE *);
// return value in TPH TPL
// Yao Fei
void ReadTemp(BYTE *rom);
#endif
