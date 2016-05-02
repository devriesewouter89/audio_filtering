#include "main.h"
#include <string.h>
#include <time.h>

/*
struct data_struct{
 uint8_t offset;
 uint8_t data[500];
};
*/

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

extern RTC_TimeTypeDef time_struct;
extern RTC_DateTypeDef date_struct;
void handle_USART_message(char *USART1_gets){
	//DEBUG - Delete once done with debugging the time set
	char buffer[50];
	
	/*Application sends the current time*/
	if( strstr(USART1_gets, "INIT") != NULL ){
		time_t raw_time;	//We're using a time_t variable because they're only 8 bytes in size
		struct tm *new_time_struct;
		
		//DEBUG - What am I receiving
		USART_puts("INIT Message is:\n");
		USART_puts(USART1_gets);
		USART_puts("\n");

		//The last 8 bytes of the message are the timestamp.
		//memcpy(&raw_time, &USART1_gets[4], sizeof(time_t));
		
		new_time_struct = gmtime(&raw_time);
		
		
		//Extract day month year, hours, minutes and seconds from the resulting struct
		time_struct.RTC_H12     = RTC_H12_PM;
		time_struct.RTC_Hours   = new_time_struct->tm_hour;
		time_struct.RTC_Minutes = new_time_struct->tm_min;
		time_struct.RTC_Seconds = new_time_struct->tm_sec;
		
		date_struct.RTC_Date		= new_time_struct->tm_mday;
		date_struct.RTC_Month		= new_time_struct->tm_mon+1;
		date_struct.RTC_Year		= new_time_struct->tm_year;
		
		//Initialize RTC with new time
		if( RTC_SetTime(RTC_Format_BCD, &time_struct) == SUCCESS ){
			USART_puts("RTC_SetTime SUCCESS\n");
		}else{
			USART_puts("RTC_SetTime FAILURE\n");
		}
		
		if( RTC_SetDate(RTC_Format_BIN, &date_struct) == SUCCESS ){
			USART_puts("RTC_SetDate SUCCESS\n");
		}else{
			USART_puts("RTC_SetDate FAILURE\n");
		}
		
		free(new_time_struct);
		USART_puts("INITRSPNS");
	}else
	if( strcmp(USART1_gets, "GET") == 0 ){
		
	}else{
		USART_puts("Invalid request:\n");
		USART_puts(USART1_gets);
	}
}
