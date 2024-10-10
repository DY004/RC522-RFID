#include "PWR_ON_OFF.h"
#include "gpio.h"
#include "key.h"
#include "delay.h"

void PWR_ON_OFF(void)
{
	if(KEY0 == GPIO_PIN_RESET)//检测关机按键的按下
	{
		HAL_Delay(50);
		while(!KEY0);//等待按键松开
		HAL_GPIO_WritePin(GPIOB,POWER_CTR_Pin, GPIO_PIN_RESET);//把开机的IO
	
	}


}


