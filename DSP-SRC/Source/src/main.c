#define STM32F40_41xxx

#include "main.h"
#include "gui_interface_communications.h"

/*Private Variables*/
extern volatile uint8_t LED_Toggle;
volatile int user_mode;

RTC_TimeTypeDef time_struct;
RTC_DateTypeDef date_struct;
RTC_InitTypeDef RTC_init_struct;
RTC_AlarmTypeDef alarm_struct;

RCC_ClocksTypeDef RCC_Clocks;

USART_InitTypeDef USART_InitStructure;
GPIO_InitTypeDef GPIO_InitStructure;

/*Private Functions*/
void UART_Initialize(void);
void GPIOInitialize(void);
void EXTIInitialize(void);
void NVICInitialize(void);
void TIMER_init(void);

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
 GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1);
 GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1);
}

void EXTIInitialize(void){
	EXTI_InitTypeDef  EXTI_InitStructure;
	
	/* Internally RTC Alarm A and EXT17 are connected */
	EXTI_ClearITPendingBit(EXTI_Line17);
  EXTI_InitStructure.EXTI_Line = EXTI_Line17;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);	

}

void NVICInitialize(void)
{
 NVIC_InitTypeDef NVIC_InitStructure;
 NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

 /* Enable the UART Incoming transmission interrupt */
 NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
 NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
 NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
 NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
 NVIC_Init(&NVIC_InitStructure);

 /* Enable the RTC Alarm Interrupt */
 NVIC_InitStructure.NVIC_IRQChannel = RTC_Alarm_IRQn;
 NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
 NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
 NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
 NVIC_Init(&NVIC_InitStructure);
}

void TIMER_init(void){
	//Everything in here is taken from this manual and stackoverflow:
	//http://www2.st.com/content/ccc/resource/technical/document/application_note/7a/9c/de/da/84/e7/47/8a/DM00025071.pdf/files/DM00025071.pdf/jcr:content/translations/en.DM00025071.pdf
	//http://stackoverflow.com/questions/18565410/clock-configuration-of-rtc-in-stm32l-in-lsi-lse-hse-only
	//http://stackoverflow.com/questions/21834403/how-to-display-time-on-stm32-discovery
	
	//Enable PWR clock
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
	
	//Unlock RTC registers
	PWR_BackupAccessCmd(ENABLE);
	RTC_WriteProtectionCmd(DISABLE);
	
	//Reset RTC Domain
	RCC_BackupResetCmd(ENABLE);
	RCC_BackupResetCmd(DISABLE);

	RCC_LSICmd(ENABLE);
	
	while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);
	
  //Tie RTC clock to one of three possible clock sources (Manual page 8)
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
	//Gotta turn things on too
	RCC_RTCCLKCmd(ENABLE);
	
	//Wait for RTC APB registers synchronisation
  RTC_WaitForSynchro();
		
	//Program the prescalers register if needed
	//Values taken from manual page 9.
	RTC_init_struct.RTC_AsynchPrediv = 127;
	RTC_init_struct.RTC_SynchPrediv = 249;
	//Decide on the Hour format (12 vs 24 hours)
	RTC_init_struct.RTC_HourFormat = RTC_HourFormat_24;
	
	//Will disable write protection, enter init mode, write all relevant registers and return failure or success
	if( RTC_Init(&RTC_init_struct) == SUCCESS ){
		USART_puts("RTC_Init SUCCESS\n");
	}else{
		USART_puts("RTC_Init FAILURE\n");
	}
	
  //Set time to 1/1/xx00 00:00:00	
  time_struct.RTC_H12     = RTC_H12_AM;
  time_struct.RTC_Hours   = 0x00;
  time_struct.RTC_Minutes = 0x00;
  time_struct.RTC_Seconds = 0x00;
	//RTC_DateStructInit(&date_struct);
	if( RTC_SetTime(RTC_Format_BCD, &time_struct) == SUCCESS ){
		USART_puts("RTC_SetTime SUCCESS\n");
	}else{
		USART_puts("RTC_SetTime FAILURE\n");
	}
	
	RTC_AlarmStructInit(&alarm_struct);
	alarm_struct.RTC_AlarmTime.RTC_H12     = RTC_H12_AM;
  alarm_struct.RTC_AlarmTime.RTC_Hours   = 0x00;
  alarm_struct.RTC_AlarmTime.RTC_Minutes = 0x00;
  alarm_struct.RTC_AlarmTime.RTC_Seconds = 0x00;
  alarm_struct.RTC_AlarmDateWeekDay 		 = 0x00;
  alarm_struct.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_Date;
	//Enabling all alarm masks will trigger an interrupt every second (Cortex-M4 manual Page 12)
	alarm_struct.RTC_AlarmMask = RTC_AlarmMask_All;
	
	RTC_SetAlarm(RTC_Format_BCD, RTC_Alarm_A, &alarm_struct);
	//Enabling the Alarm Interrupt
	RTC_ITConfig(RTC_IT_ALRA, ENABLE);
	
	if( RTC_AlarmCmd(RTC_Alarm_A, ENABLE) == SUCCESS){
		USART_puts("Alarm_A_enable SUCCESS\n");
	}else{
		USART_puts("Alarm_A_enable FAILURE\n");
	}
	
	//Clear possible bit on Alarm A interrupt
	RTC_ClearFlag(RTC_FLAG_ALRAF);
}

volatile char USART1_gets[5] = "NOT\n";
int USART1_valid_line = 0;
int USART1_read_index = 0;

int main(void)
{
	char time_buffer[50];
	
	GPIOInitialize();
	UART_Initialize();
	USART_puts("GPIO and UART initialized\n");
	EXTIInitialize();
	NVICInitialize();
	USART_puts("EXTI and NVIC initialized\n");
	TIMER_init();
	USART_puts("Timer initialized\n");
	
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
	
  WavePlayBack(I2S_AudioFreq_48k); //Contains Main program loop
	while(1)
	{ 
		/*if(RTC_GetITStatus(RTC_IT_ALRA) == SET){
			RTC_GetTime(RTC_Format_BIN, &time_struct); 
			sprintf(time_buffer, "%02d:%02d:%02d\n", time_struct.RTC_Hours, time_struct.RTC_Minutes, time_struct.RTC_Seconds);
			USART_puts(time_buffer);
			RTC_ClearITPendingBit(RTC_IT_ALRA);
		}*/
	}
}

#ifdef USE_FULL_ASSERT
 
void assert_failed(uint8_t* file, uint32_t line)
{
 while (1)
 {}
}
#endif

