#include "gui_interface_communications.h"

//Senior Design Functions
void USART_puts(volatile char *s)
{
	__disable_irq();
	
	while(*s){
		// wait until data register is empty
		while( !(USART1->SR & 0x00000040) );
		USART_SendData(USART1, *s++);
	}
	
	__enable_irq();
}

void USART_put_data(uint8_t data)
{
		
  // wait until data register is empty
  while( !(USART1->SR & 0x00000040) );
  USART_SendData(USART1, data);
	
	
}

//Interrupt USART Read
//USART Variables
extern volatile char USART1_gets[5];
extern int USART1_read_index;
extern int USART1_valid_line;

void poll_USART(void){
	
	int x = 0;
	char buffer[25];
	
	if(USART1_valid_line){
			if( strstr((const char *)USART1_gets,"GET") ){
				//for(x = 0; x < 500; x++){
					//snprintf(buffer, 25, "%"PRIu64"0000000,\n", data_points.data[x]);
				//	USART_puts(buffer);
				//}
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

extern volatile RTC_TimeTypeDef time_struct;
void poll_Alarm(void){
	if(RTC_GetITStatus(RTC_IT_ALRA) != RESET)
  {
		char time_buffer[50];
		RTC_GetTime(RTC_Format_BIN, &time_struct); 
		sprintf(time_buffer, "%02d:%02d:%02d\n", time_struct.RTC_Hours, time_struct.RTC_Minutes, time_struct.RTC_Seconds);
		USART_puts(time_buffer);
		
    STM_EVAL_LEDToggle(LED3);
    RTC_ClearITPendingBit(RTC_IT_ALRA);
    EXTI_ClearITPendingBit(EXTI_Line17);
  } 
}