/*
 * 多个DS18B20并联搜索算法
 * 
 * http://blog.sina.com.cn/s/blog_57ad1bd20102uxxw.html
 */


#include "ds18b20_search.h"

// 记录所有DS18B20的ROM地址和个数，最多4个
BYTE DS18B20_ROM[MAXNUM][8];
BYTE DS18B20_Num = 0;


//全局搜索变量
char LastDiscrepancy;  //每轮搜索后指向最后一个走0的差异位
char LastFamilyDiscrepancy; //指向家族码（前8位）中最后一个走0的差异位
__bit LastDeviceFlag;   //搜到最后一个ROM后，程序通过判别将该变量置1，下轮搜索时即会结束退出
 

//--------------------------------------------------------------------------
//    在单总线上搜索第一个器件
// 返回TRUE: 找到, 存入ROM_NO缓冲；FALSE:无设备
// 先将初始化3个变量，然后调用OWSearch算法进行搜索
//--------------------------------------------------------------------------
__bit OWFirst()
{
	LastDiscrepancy = 0;
	LastDeviceFlag = FALSE;
	LastFamilyDiscrepancy = 0;
	
	return OWSearch();
}

//--------------------------------------------------------------------------
//    在单总线上搜索下一个器件
// 返回TRUE: 找到, 存入ROM_NO缓冲；FALSE:无设备，结束搜索
// 在前一轮搜索的基础上（3个变量均在前一轮搜索中有明确的值），再执行一轮搜索
//--------------------------------------------------------------------------

__bit OWNext()
{
	return OWSearch();
}
 
//--------------------------------------------------------------------------
//     单总线搜索算法，利用了一些状态变量，这是算法的核心程序，代码也较长
//     返回TRUE: 找到, 存入ROM_NO缓冲；FALSE:无设备，结束搜索
//--------------------------------------------------------------------------
__bit OWSearch()
{
	char id_bit_number;    //指示当前搜索ROM位（取值范围为1-64）

	//下面三个状态变量含义：
	//last_zero：  指针，记录一次搜索（ROM1-64位）最后一位往0走的混码点编号
	//search_direction：搜索某一位时选择的搜索方向（0或1），也是“一写”的bit位值
	//rom_byte_number： ROM字节序号，作为ROM_no[]数组的下标，取值为1—8
	char last_zero, rom_byte_number, search_result;
	__bit id_bit, cmp_id_bit,  search_direction; //二读（正码、反码）、及一写（决定二叉搜索方向）

	unsigned char rom_byte_mask ; //ROM字节掩码，
	
	// 初始化本次搜索变量
	id_bit_number = 1;
	last_zero = 0;
	rom_byte_number = 0;
	rom_byte_mask = 1;
	search_result = 0;

// ------------------------------------------------------------------
//1。是否搜索完成（已到最后一个设备）？
//-------------------------------------------------------------------
	if (!LastDeviceFlag)  // LastDeviceFlag由上轮搜索确定是否为最后器件，当然首次进入前必须置False
	{
		if (DS18B20_Reset())    //复位总线
		{
			LastDiscrepancy = 0;  //复位几个搜索变量
			LastDeviceFlag = FALSE;
			LastFamilyDiscrepancy = 0;
			return FALSE;    //如果无应答，返回F，退出本轮搜索程序
		}
		DS18B20_WriteByte(0xF0);   //发送ROM搜索命令F0H
		//DelayXus(US60);
		
//=====================================================================
// 开始循环处理1-64位ROM，每位必须进行“二读”后进行判断，确定搜索路径
// 然后按选定的路径进行“一写”，直至完成全部位的搜索，这样一次循环
// 可以完成一轮搜索，找到其中一个ROM码。
//=====================================================================

		do {        //逐位读写搜索，1-64位循环
			id_bit = DS18B20_ReadBit();   //二读：先读正码、再读反码
			cmp_id_bit = DS18B20_ReadBit();
			
			if (id_bit  && cmp_id_bit)  { //二读11，则无器件退出程序
				break;
			}
			else {      //二读不为11，则需分二种情况
				//*********************************************
				// 第一种情况：01或10，直接可明确搜索方向
				if (id_bit != cmp_id_bit) {
					search_direction = id_bit;  // 记下搜索方向search_direction的值待“一写”
					//*********************************************
				} else {	     
					// 否则就是第二种情况：遇到了混码点，需分三种可能分析：
					// 1。当前位未到达上轮搜索的“最末走0混码点”（由LastDiscrepancy存储）
					//    说明当前经历的是一个老的混码点，判别特征为当前位在（小于）LastDiscrepancy前
					//    不管上次走的是0还是1,只需按上次走的路即可，该值需从ROM_NO中的当前位获取
					if (id_bit_number < LastDiscrepancy) {
						search_direction = ((DS18B20_ROM[MAXNUM-1][rom_byte_number] & rom_byte_mask) > 0);
					} else {
						// 2。当前位正好为上轮标记的最末的混码点，这个混码点也就是上次走0的点
						//    那么这次就需要走1
						// 3。除去上二种可能，那就是第3种可能： 这是一个新的混码点，
						//    id_bit_number>LastDiscrepancy
						//。。然而下一条语句巧妙地将上二种可能合在一起处理，看不懂我也没办法了
						search_direction = (id_bit_number == LastDiscrepancy);
					}
					//************************************************
					// 确定了混码点的路径方向还没完事，还需要更新一个指针：last_zero
					// 这个指针每搜索完一位后（注意是一bit不是一轮）总是指向新的混码点
					// 凡遇到新的混码点，我们按算法都是先走0，所以凡遇走0的混码点必须更新此指针
					if (search_direction == 0) {
						last_zero = id_bit_number;
						// 下面二条是程序的高级功能了：64位ROM中的前8位是器件的家族代码，
						// 用LastFamilyDiscrepancy这个指针来记录前8位ROM中的最末一个混码点
						// 可用于在多类型器件的单线网络中对家族分组进行操作
						if (last_zero < 9) {
							LastFamilyDiscrepancy = last_zero;
						}
					}
				}

                                // 确定了要搜索的方向search_direction，该值即ROM中当前位的值，需要写入ROM
				// 然而64位ROM需分8个字节存入ROM_NO[]，程序使用了一个掩码字节rom_byte_mask
				// 以最低位为例：该字节值为00000001，如记录1则二字节或，写0则与反掩码
				if (search_direction == 1) {
					DS18B20_ROM[MAXNUM-1][rom_byte_number] |= rom_byte_mask;
				} else {
 					DS18B20_ROM[MAXNUM-1][rom_byte_number] &= ~rom_byte_mask;
				}
				
				// 关键的一步操作终于到来了：一写
				DS18B20_WriteBit(search_direction);
				
				// 一个位的操作终于完成，但还需做些工作，以准备下一位的操作：
				// 包括：位变量id_bit_number指向下一位；字节掩码左移一位
				id_bit_number++;
				rom_byte_mask <<= 1;
				
                                // 如果够8位一字节了，则对该字节计算CRC处理、更新字节号变量、重设掩码
				if (rom_byte_mask == 0)
				{
					//docrc8(DS18B20_ROM[MAXNUM-1][rom_byte_number]);  // CRC计算原理参考其他文章
					rom_byte_number++;
					rom_byte_mask = 1;
				}
			}
		} while(rom_byte_number < 8);  // ROM bytes编号为 0-7

		//流程图中描述从1到64的位循环，本代码中是利用rom_byte_number<8来判断的
		//至此，终于完成8个字节共64位的循环处理
		//=================================================================================
 
		// 一轮搜索成功，找到的一个ROM码也校验OK，则还要处理二个变量
		if (!((id_bit_number < 65))) {
			// 一轮搜索结束后，变量last_zero指向了本轮中最后一个走0的混码位
			// 然后再把此变量保存在LastDiscrepancy中，用于下一轮的判断
			// 当然，last_zero在下轮初始为0，搜索是该变量是不断变动的
			LastDiscrepancy = last_zero;
			// 如果这个指针为0，说明全部搜索结束，再也没有新ROM号器件了
			if (LastDiscrepancy == 0)
				LastDeviceFlag = TRUE;  //设置结束标志
        
			search_result = TRUE;  //返回搜索成功
		}
	}
// ------------------------------------------------------------------
//搜索完成，如果搜索不成功包括搜索到了但CRC错误，复位状态变量到首次搜索的状态。
//-------------------------------------------------------------------
	if (!search_result || !DS18B20_ROM[MAXNUM-1][0]) {
		LastDiscrepancy = 0;
		LastDeviceFlag = FALSE;
		LastFamilyDiscrepancy = 0;
		search_result = FALSE;
	}
	return search_result;
}
//=====================================================================
//   至此，OWSearch函数结束。函数实现的是一轮搜索，如成功，可得到一个ROM码
//=======================================================================

BYTE DS18B20_Search()
{
	__bit rst;
	char i;
	char num = 0;

	rst = OWFirst();

	while ((rst) && (num < MAXNUM))  {
		for (i=0;i<8;i++) {
			DS18B20_ROM[num][i] = DS18B20_ROM[MAXNUM-1][i];
		}
		
		num++;
		
		rst = OWNext(); 
		
	}
	
	return num;
}

