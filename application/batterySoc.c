#include "batterySoc.h"
#include "stm32l1xx_hal.h"
#include "cmsis_os.h"
#include "log_module.h"


#define BATTERY_DEBUG_ENABLE 0
#if BATTERY_DEBUG_ENABLE
#define BATTERY_LOG(...) DBG_LOG(__VA_ARGS__)
#else
#define BATTERY_LOG(...)
#endif


static  ADC_HandleTypeDef hadc;

/*
	通过ADC采样，反推VBAT的电压，VBAT----R(150K)----R(100K)---GND
                                                  |
												 ADC

    VBAT/(150K+100K) = VADC/(100K)==> VBAT = VADC *(150K +100K)/100K
*/


/*参考电压是3300mv*/

#define VREF          3280.0
/*量程是2的12次方*/
#define FULL_SCALE    (4096-1)

/*rank Ascending*/
static void BubbleSortAscending(uint32_t *arr, uint8_t size)  
{  
	uint32_t i, j, tmp;  
	for (i = 0; i < size - 1; i++) {  
	    for (j = 0; j < size - i - 1; j++) {  
	        if (arr[j] > arr[j+1]) {  
	            tmp = arr[j];  
	            arr[j] = arr[j+1];  
	            arr[j+1] = tmp;  
	        }  
	    }  
	}  
}

/* ADC init function */
static void MX_ADC_Init(ADC_HandleTypeDef *hadc)
{

  ADC_ChannelConfTypeDef sConfig;

    /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
    */
  hadc->Instance = ADC1;
  hadc->Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc->Init.Resolution = ADC_RESOLUTION_12B;
  hadc->Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc->Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc->Init.EOCSelection = ADC_EOC_SEQ_CONV;
  hadc->Init.LowPowerAutoWait = ADC_AUTOWAIT_DISABLE;
  hadc->Init.LowPowerAutoPowerOff = ADC_AUTOPOWEROFF_DISABLE;
  hadc->Init.ChannelsBank = ADC_CHANNELS_BANK_A;
  hadc->Init.ContinuousConvMode = DISABLE;
  hadc->Init.NbrOfConversion = 1;
  hadc->Init.DiscontinuousConvMode = DISABLE;
  hadc->Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc->Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc->Init.DMAContinuousRequests = DISABLE;
  if (HAL_ADC_Init(hadc) != HAL_OK)
  {
    DBG_LOG("HAL_ADC_Init fail\n");
  }

    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
    */
  sConfig.Channel = ADC_CHANNEL_7;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_4CYCLES;
  if (HAL_ADC_ConfigChannel(hadc, &sConfig) != HAL_OK)
  {
  	DBG_LOG("HAL_ADC_ConfigChannel conig fail\n");
   // _Error_Handler(__FILE__, __LINE__);
  }

}


/*ADC 采样10组数据，去掉最大值最小值，求平均*/
void get_battery_soc(uint8_t *soc,float* voltage)
{
	HAL_StatusTypeDef status;
	uint32_t value[10];
	uint8_t i = 0;
	MX_ADC_Init(&hadc);
	//wait 30ms
	//osDelay(30);
	
	for(i = 0; i < 10;i++){
		status = HAL_ADC_Start(&hadc);
		if(status != HAL_OK){
			DBG_LOG("HAL ADC start fail %d,i = %d\n",status,i);
		}
		status = HAL_ADC_PollForConversion(&hadc,0xff);
		if(status != HAL_OK){
			DBG_LOG("HAL_ADC_PollForConversion  fail %d\n",status);	
		}
		value[i] = HAL_ADC_GetValue(&hadc);
		osDelay(1);
	}
	BubbleSortAscending(value,10);
#if BATTERY_DEBUG_ENABLE
	for(i = 0;i < 10;i++){
		printf("value[%d] = %d\n",i,value[i]);
	}
#endif
	
	uint32_t value_add = 0;
	for(i = 1;i < 9;i++){
		value_add += value[i];
	}

	float value_ava = value_add/8.0;

	BATTERY_LOG("value ava %f\n",value_ava);

	*voltage = VREF/FULL_SCALE * value_ava;

	*soc = 10;
	
	BATTERY_LOG("voltage is %f\n",*voltage);
	
	status = HAL_ADC_Stop(&hadc);
	if(status != HAL_OK){
		DBG_LOG("HAL_ADC_Stop  fail %d\n",status);	
	}

	status = HAL_ADC_DeInit(&hadc);
	if(status != HAL_OK){
		DBG_LOG("HAL_ADC_DeInit  fail %d\n",status);	
	}

}


