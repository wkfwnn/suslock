
#include "hexToStr.h"

void int2Char(int p_nNum, char *p_Ch)
{
	if(p_nNum <= 9)
	{
		*p_Ch = p_nNum + '0'; 
	}
	else 
	{
		/*0 - 9 是十个数*/
		//*p_Ch = (p_nNum -10) + 'A';
	}
}
 
 
/*传入16进制数据和长度*/
void str2BcdStr(char *p_Str, int p_nLen, char *p_StrBcd)
{
	int i = 0, j = 0;
	
	for(i = 0; i < p_nLen; ++i)
	{		
		int2Char((p_Str[i] >> 4)&0x0F, &p_StrBcd[j++]);	
		int2Char(p_Str[i] &0x0F, &p_StrBcd[j++]);
	}
	return ;
}

