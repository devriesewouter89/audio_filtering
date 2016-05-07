#include <string.h>
#include "datalogging.h"
#include "gui_interface_communications.h"

void save_record(record_keeper *r, RTC_TimeTypeDef t, RTC_DateTypeDef d){
	memcpy(&r->records[r->head].time_log_struct, &t, sizeof(RTC_TimeTypeDef));
	memcpy(&r->records[r->head].date_log_struct, &d, sizeof(RTC_DateTypeDef));
	
	r->head = (r->head + 1)%500;
	r->num_records = (r->num_records < 500) ? (r->num_records + 1) : 500;
}

void transmit_records(record_keeper *r){
	int x, start_index = 0;
	char buffer[100];
	
	//Calculating the the index we'd wanna start at
	start_index = r->head - r->num_records;
	start_index = (start_index < 0) ? (start_index + 500) : start_index;
	
	//We go for as many indexes as we have currently recordes. Maximum of 500
	for(x = 0; x < r->num_records; x++){
		RTC_TimeTypeDef t = r->records[(start_index + x)%500].time_log_struct;
		RTC_DateTypeDef d = r->records[(start_index + x)%500].date_log_struct;
		
		//Convert back to 24 Hour
		int hours = ( t.RTC_H12 == RTC_H12_AM) ? t.RTC_Hours : t.RTC_Hours + 12;
		sprintf(buffer, "%2d,%2d,%2d,%2d,%2d,%4d\n", hours, t.RTC_Minutes, t.RTC_Seconds, d.RTC_Date, d.RTC_Month, d.RTC_Year+1970);
		USART_puts(buffer);
	}
	USART_puts("ENDDATA\n");
}
