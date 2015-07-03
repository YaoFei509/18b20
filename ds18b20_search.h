/*
 * 多个DS18B20并联搜索算法
 */

#ifndef __DS18B20_SEARCH_H
#define __DS18B20_SEARCH_H

#include <stc12.h>
#include "ds18b20.h"


#define TRUE 1
#define FALSE 0
#define MAXNUM 4


extern BYTE DS18B20_ROM[4][8];
extern BYTE DS18B20_Num;

BYTE DS18B20_Search();

__bit  OWSearch();  //算法核心函数，完成一次ROM搜索过程
__bit  OWFirst();   //调用OWSearch完成第一次搜索
__bit  OWNext();    //调用OWSearch完成下一次搜索

#endif
