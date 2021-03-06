/***************************************************
 * DS18B20 
 * 12T 
 ***************************************************/
#include "ds18b20.h"

// 检测并启动DS18B20转换
// 返回0: 存在并已经启动
//     1: 不存在DS18B20
__bit StartDS18B20()
{
	if (DS18B20_Reset() == 0) {
		DS18B20_WriteByte(0xCC);        //跳过ROM命令
		DS18B20_WriteByte(0x44);        //开始转换命令
	} else {
		return 1;
	}

	return 0;
}

void ReadTemp(BYTE *rom)
{
	int i;
	
	while (!DQ);                    //等待转换完成
	DS18B20_Reset();                //设备复位

	if (rom == 0) {
		DS18B20_WriteByte(0xCC);        //跳过ROM命令
	} else {
		DS18B20_WriteByte(0x55);    // match rom
		for (i=0;i<8;i++) {
			DS18B20_WriteByte(rom[i]);
		}
	}
	DS18B20_WriteByte(0xBE);        //读暂存存储器命令
	TPL = DS18B20_ReadByte();       //读温度低字节
	TPH = DS18B20_ReadByte();       //读温度高字节
}

//----------------------  工具函数 -------------------------
/**************************************
延时X*10微秒(STC90C52RC@12M)
不同的工作环境,需要调整此函数
当改用1T的MCU时,请调整此延时函数
**************************************/
void DelayX0us(BYTE n)
{
	while (n--) {
		__nop__;
		__nop__;
	}
}

/**************************************
复位DS18B20,并检测设备是否存在
**************************************/
__bit DS18B20_Reset()
{
	__bit CY;
	
	DQ = 1;
	DelayX0us(2);
	DQ = 0;                     //送出低电平复位信号
	DelayX0us(48);              //延时至少480us
	DQ = 1;                     //释放数据线
	DelayX0us(6);               //等待60us
	CY = DQ;                    //检测存在脉冲
	DelayX0us(42);              //等待设备释放数据线

	return CY; 	
}

/**************************************
从DS18B20读1字节数据
**************************************/
BYTE DS18B20_ReadByte()
{
	BYTE i;
	BYTE dat = 0;

	for (i=0; i<8; i++) {              //8位计数器
		dat >>= 1;
		DQ = 0;                     //开始时间片
		__nop__;                    //延时等待
		__nop__;
		DQ = 1;                     //准备接收
		__nop__;                    //接收延时
		__nop__;
		if (DQ) dat |= 0x80;        //读取数据
		DelayX0us(6);               //等待时间片结束
	}

	return dat;
}

/**************************************
向DS18B20写1字节数据
**************************************/
void DS18B20_WriteByte(BYTE dat)
{
	char i;

	for (i=0; i<8; i++) {             //8位计数器
		DQ = 0;                     //开始时间片
		__nop__;                    //延时等待
		__nop__;
		dat >>= 1;                  //送出数据
		DQ = CY;
		DelayX0us(6);               //等待时间片结束
		DQ = 1;                     //恢复数据线
	}
}

/*********************************************************
 *功能:读序列
 **参数：无返回
 *********************************************************/
BYTE DS18B20_ReadRom(BYTE *rom)
{
	BYTE i;
	DS18B20_Reset(); 
	DS18B20_WriteByte(0x33);
	for (i=0; i<8; i++) {
		rom[i] = DS18B20_ReadByte();
	}
	return 0;
}

// 读一位
__bit DS18B20_ReadBit()
{
	__bit result;

	DQ = 0;            //开始时间片

	__nop__;           //延时等待
	__nop__;
	__nop__;
	__nop__;
	
        DQ = 1;                     //准备接收

	__nop__;           //延时等待
	__nop__;
	__nop__;
	__nop__;

	result = DQ;

	DelayX0us(6);               //等待时间片结束

        return result;
}

void DS18B20_WriteBit(__bit dat)
{
	DQ = 0;                     //开始时间片
	__nop__;                    //延时等待
	__nop__;
	dat >>= 1;                  //送出数据
	DQ = dat;
	DelayX0us(6);               //等待时间片结束
	DQ = 1;                     //恢复数据线
}
