#include "main.h"

RCC_ClocksTypeDef RCC_Clocks;
extern volatile uint8_t LED_Toggle;
volatile int user_mode;
void UART_Initialize(void);
void GPIOInitialize(void);
void NVICInitialize(void);
void USART_puts( volatile char *s);

 
int main(void)
{
	GPIOInitialize();
	UART_Initialize();
	NVICInitialize();
	
  /* Initialize LEDs */
  STM_EVAL_LEDInit(LED3);
  STM_EVAL_LEDInit(LED4);
  STM_EVAL_LEDInit(LED5);
  STM_EVAL_LEDInit(LED6);
 
  /* Green Led On: start of application */
  STM_EVAL_LEDOn(LED4);
       
  /* SysTick end of count event each 1ms */
  RCC_GetClocksFreq(&RCC_Clocks);
  SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);

  /* Initialize User Button */
  STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_EXTI);

	USART_puts("Init complete! Hello World!     "); // just send a message to indicate that it works
  WavePlayBack(I2S_AudioFreq_48k); 
	while(1){
		
	}
}

#ifdef USE_FULL_ASSERT
 
void assert_failed(uint8_t* file, uint32_t line)
{
 while (1)
 {}
}
#endif

