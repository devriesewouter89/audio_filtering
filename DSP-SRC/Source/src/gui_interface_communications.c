#include "main.h"
#include "datalogging.h"
#include <string.h>
#include <time.h>

/*
struct data_struct{
 uint8_t offset;
 uint8_t data[500];
};
*/
void flush_buffer(char *USART_gets){
	int x;
	for(x = 0; x < 20; x++)
		USART_gets[x] = '\0';
}

//Senior Design Functions
void USART_puts(volatile char *s)
{

	while(*s){
		// wait until data register is empty
		while( !(USART2->SR & 0x00000040) );
		USART_SendData(USART2, *s++);
	}

}

extern RTC_TimeTypeDef time_struct;
extern RTC_DateTypeDef date_struct;
extern record_keeper records;
	
void handle_USART_message(char *USART_gets){
	char buffer[100];
	
	/*Application sends the current time*/
	if( strstr(USART_gets, "INIT") != NULL ){
		time_t raw_time = 0;	//We're using a time_t variable because they're only 4 bytes in size
		struct tm *new_time_struct;

		raw_time = atoi(&USART_gets[4]);		
		new_time_struct = localtime(&raw_time);
		
		//Extract day month year, hours, minutes and seconds from the resulting struct
		time_struct.RTC_H12     = (new_time_struct->tm_hour >= 12) ? RTC_H12_PM : RTC_H12_AM;
		time_struct.RTC_Hours   = (new_time_struct->tm_hour >= 13) ? new_time_struct->tm_hour - 12 : new_time_struct->tm_hour;
		time_struct.RTC_Minutes = new_time_struct->tm_min;
		time_struct.RTC_Seconds = new_time_struct->tm_sec;
		
		date_struct.RTC_Date		= new_time_struct->tm_mday;
		date_struct.RTC_Month		= new_time_struct->tm_mon+1;
		date_struct.RTC_Year		= new_time_struct->tm_year+1882;
		
		RTC_SetTime(RTC_Format_BCD, &time_struct);
		RTC_SetDate(RTC_Format_BIN, &date_struct);
		
		//Update the screen
		RTC_GetTime(RTC_Format_BIN, &time_struct);
		sprintf(buffer, "%02d:%02d", time_struct.RTC_Hours, new_time_struct->tm_min);
		TM_HD44780_Puts(0, 1, buffer);
		
		RTC_GetDate(RTC_Format_BIN, &date_struct);
		sprintf(buffer, "%d/%d/%4d", date_struct.RTC_Date, date_struct.RTC_Month, date_struct.RTC_Year+1970);
		TM_HD44780_Puts(0, 0, buffer);
		
		USART_puts("\n\0");
		USART_puts("INITRSPNS\n");
		free(new_time_struct);
	}else
	if( strstr(USART_gets, "GET") != NULL ){
		USART_puts("\n\0");
		
		transmit_records(&records);
	}else{
		sprintf(buffer, "Invalid request: %s\n", USART_gets);
		USART_puts(buffer);
	}
	
}
