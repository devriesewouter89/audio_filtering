#include "main.h"
//#include "gui_interface_communications.c"

RCC_ClocksTypeDef RCC_Clocks;
extern volatile uint8_t LED_Toggle;
volatile int user_mode;
void UART_Initialize(void);
void GPIOInitialize(void);
void NVICInitialize(void);


USART_InitTypeDef USART_InitStructure;
GPIO_InitTypeDef GPIO_InitStructure;
 
void UART_Initialize(void)
{
	 /* Enable peripheral clock for USART1 */
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	/* USART1 configured as follow:
	 * BaudRate 9600 baud
	 * Word Length 8 Bits
	 * 1 Stop Bit
	 * No parity
	 * Hardware flow control disabled
	 * Receive and transmit enabled
	 */
	 USART_InitStructure.USART_BaudRate = 9600;
	 USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	 USART_InitStructure.USART_StopBits = USART_StopBits_1;
	 USART_InitStructure.USART_Parity = USART_Parity_No;
	 USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	 USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	 
	 USART_Init(USART1, &USART_InitStructure); // USART configuration
	 USART_Cmd(USART1, ENABLE); // Enable USART
	 USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); //Set up read bit interrupt.
}
 
void GPIOInitialize(void)
{
 RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); //Enable clock for GPIOB

/* USART1 Tx on PB6 | Rx on PB7 */
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
 GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //http://www.amazon.com/STMICROELECTRONICS-STM32F4DISCOVERY-STM32-DISCOVERY-STM32F407/dp/B00HPLT1I4/ref=sr_1_1?ie=UTF8&qid=1454459969&sr=8-1&keywords=stm+32f4
 GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
 
 GPIO_Init(GPIOB, &GPIO_InitStructure);
 GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1);//Connect PB6 to USART1_Tx
 GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1);//Connect PB7 to USART1_Rx
}
 
void NVICInitialize(void)
{
 NVIC_InitTypeDef NVIC_InitStructure;
 NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
 NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
 NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
 NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
 
 NVIC_Init(&NVIC_InitStructure);
 USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
}

volatile char USART1_gets[5] = "NOT\n";
int USART1_valid_line = 0;
int USART1_read_index = 0;

int main(void)
{
	int x;
	data_struct data_points;
	char buffer[25];
	
	data_points.data[0]  = 0;
  data_points.data[1]  = 1;
  data_points.data[2]  = 1000;
  data_points.data[3]  = 10019990;
  data_points.data[4]  = 11200112;
  data_points.data[5]  = 12312300;
	data_points.data[6]  = 13371337;
	data_points.data[7]  = 13371370;
  data_points.data[8]  = 13389000;
  data_points.data[9]  = 14120123;
  data_points.data[10] = 14372690;
  data_points.data[11] = 31231231;
  data_points.data[12] = 42312310;
	data_points.data[13] = 43215123;
	data_points.data[499]= 46120312;
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
  //WavePlayBack(I2S_AudioFreq_48k); //Contains Main program loop
	
	while(1)
	{
	  if(USART1_valid_line){
			if( strstr((const char *)USART1_gets,"GET") ){
				for(x = 0; x < 500; x++){
					snprintf(buffer, 25, "%"PRIu64"0000000,\n", data_points.data[x]);
					USART_puts(buffer);
				}
				USART_puts("ENDDATA\n");
			} 
			else if( strstr((const char *)USART1_gets,"INIT") ){
				USART_puts("INITRSPNS\n");
			}
			else{
				USART_puts("Somenonesense\n");
			}
			USART1_valid_line = 0;
		}
	}
}

#ifdef USE_FULL_ASSERT
 
void assert_failed(uint8_t* file, uint32_t line)
{
 while (1)
 {}
}
#endif

