
#include "intToString.h"
#include "math.h"

void intostring(uint8_t *originData,uint8_t originDataLen,uint8_t *outputData,uint8_t radix)
{
	int i = 0;
	uint8_t hundred;
	uint8_t ten;
	uint8_t one;
    if(radix == 16){
    	goto hextoa;
    }
    if(radix == 10){
    	goto itoa;
    }
	return;

hextoa:
	for(i = 0;i < originDataLen;i++){
		ten = originData[i]/16;
		one = originData[i]%16;
		if(ten > 9){
			outputData[i*2] = ten + 0x37;
		}else{
			outputData[i*2] = ten + 0x30;
		}
		if(one > 9){
			outputData[i*2 + 1] = one + 0x37;
		}else{
			outputData[i*2 + 1] = one + 0x30;
		}
	}
	goto exit;
itoa:
	for(i = 0;i < originDataLen;i++){
		hundred = originData[i]/100;
		ten = originData[i]%100/10;
		one = originData[i]%100%10;
		outputData[i*3] = hundred + 0x30;
		outputData[i*3 + 1] = ten + 0x30;
		outputData[i*3 + 2] = one + 0x30;
	}
	goto exit;
exit:
	return;
}

int myatoi(uint8_t *data,int radix)
{
	uint8_t power = strlen(data);
	uint32_t value = 0;
	uint32_t ret = 0;
	if(radix == 10){
		return atoi(data);
	}
	if(radix == 16){
		while(*data != 0x00){
			if(*data >= '0' && *data <= '9'){
				value = *data - '0';
			}
			if(*data >= 'A' && *data <= 'F'){
				value = *data - 'A' + 0x0A;
			}
			if(*data >= 'a' && *data <= 'f'){
				value = *data - 'a' + 0x0A;
			}
            

			ret += value * pow(16,(power - 1));
			printf("value = %d,power =%d,ret = %x\n",value,power,ret);
			power -= 1;
			data++;
		}
		return ret;
	}

}


