/**************************************
 * 
 * 温度传感器DS18B20测试程序
 * 主芯片  : STC11F04E(1T)
 * 工作频率: 5.66MHz
 *
 **************************************/
#include "ds18b20_1t.h"

//---------------------------------------------------------------
// 延时时间
#ifdef STC11F04E
// 22 = 64us 24 = 70us
#define US60 24
#else
#define US60 60
#endif

#define US240 (4*US60)
#define US180 (3*US60)
//---------------------------------------------------------------

// 检测并启动DS18B20转换
// 返回0: 存在并已经启动
//     1: 不存在DS18B20
BYTE StartDS18B20()
{
	if (DS18B20_Reset() == 0) {
		DS18B20_WriteByte(0xCC);        //跳过ROM命令
		DS18B20_WriteByte(0x44);        //开始转换命令
	} else {
		return 1;
	}

	return 0;
}

// 读取测温值
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
延时X微秒(STC12C5A60S2@12M)
不同的工作环境,需要调整此函数
此延时函数是使用1T的指令周期进行计算,与传统的12T的MCU不同
**************************************/
void DelayXus(BYTE n)
{
    while (n--) {
        __nop__;
        __nop__;
    }
}


/**************************************
复位DS18B20,并检测设备是否存在
**************************************/
BYTE DS18B20_Reset()
{
	BYTE i=0;
	BYTE CY = 1;

	while (CY && (i<100)) {
		DQ = 0;                     //送出低电平复位信号
		DelayXus(US240);            //延时至少480us
		DelayXus(US240);
		DQ = 1;                     //释放数据线
		DelayXus(US60);               //等待60us
		CY = DQ;                    //检测存在脉冲
		DelayXus(US240);              //等待设备释放数据线
		DelayXus(US180);
		i++;
	}

	if (100 == i )
		return 1;
	else
		return 0;  //found
}

/**************************************
从DS18B20读1字节数据
**************************************/
BYTE DS18B20_ReadByte()
{
    BYTE i;
    BYTE dat = 0;

    for (i=0; i<8; i++) {           //8位计数器
        dat >>= 1;
        DQ = 0;                     //开始时间片
        DelayXus(1);                //延时等待
        DQ = 1;                     //准备接收
        DelayXus(1);                //接收延时
        if (DQ) {
		dat |= 0x80;        //读取数据
	}
        DelayXus(US60);               //等待时间片结束
    }

    return dat;
}

/**************************************
向DS18B20写1字节数据
**************************************/
void DS18B20_WriteByte(BYTE dat)
{
    char i;

    for (i=0; i<8; i++) {           //8位计数器
        DQ = 0;                     //开始时间片
        DelayXus(1);                //延时等待
        dat >>= 1;                  //送出数据
        DQ = CY;
        DelayXus(US60);               //等待时间片结束
        DQ = 1;                     //恢复数据线
        DelayXus(1);                //恢复延时
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
	DS18B20_WriteByte(0x33); //Read ROM
	for (i=0; i<8; i++) {
		rom[i] = DS18B20_ReadByte();
	}
	return 0;
}

//
/***********************自动搜索ROM*****************************/
BYTE DS18B20_Read_2bit(void)//读二位 子程序
{
	BYTE dat = 0;
	
	DQ = 0;
	__nop__;
	__nop__;
	DQ = 1;
	__nop__;
	__nop__;
	
	if (DQ) 
		dat = 2;
	
	DelayXus(4);
	
	DQ = 0;
	__nop__;
	__nop__;
	DQ = 1;
	__nop__;
	__nop__;
	
	if (DQ) 
		dat += 1;

 	return dat;
}


BYTE search_rom(BYTE *ss)//搜索ROM
{ 
	unsigned char k=0,l=0,chongtuwei=0,m=0,n=0;
	unsigned char s=0;
	unsigned char zhan[MAXNUM];

	do {
		DS18B20_Reset();
		DS18B20_WriteByte(0xf0); //搜索ROM命令	

		for(m=0;m<8;m++)
		{
			for(n=0;n<8;n++)
			{
				k = DS18B20_Read_2bit();//读两位数据

				s>>=1;

				switch (k) {
					case 0x01 : //01读到的数据为0 写0 此位为0的器件响应
							DS18B20_WriteBit(0);
	    					ss[(m*8+n)]=0;
							break;
					case	2: //读到的数据为1 写1 此位为1的器件响应
							s=s|0x80;
							DS18B20_WriteBit(1);
							ss[(m*8+n)]=1;
							break;

					case    0: //读到的数据为0  有冲突位 判断冲突位 
								//如果冲突位大于栈顶写0 小于栈顶写以前数据 等于栈顶写1
							chongtuwei=m*8+n+1;					
							if(chongtuwei>zhan[l]) {						
								DS18B20_WriteBit (0);
								ss[(m*8+n)]=0;												
								zhan[++l]=chongtuwei;						
							} else if(chongtuwei<zhan[l]) {
								s=s|((ss[(m*8+n)]&0x01)<<7);
								DS18B20_WriteBit(ss[(m*8+n)]);
							} else if(chongtuwei==zhan[l]) {
								s=s|0x80;
								DS18B20_WriteBit(1);
								ss[(m*8+n)]=1;
								l=l-1;
							}
							break;
					default:
					goto loop;
				}
			}
			ID[num][m]=s;		
		}
		num++;
	}
	while(zhan[l]!=0&&(num<MAXNUM));		
loop:
	search_cartoon();
	display_total();

}

