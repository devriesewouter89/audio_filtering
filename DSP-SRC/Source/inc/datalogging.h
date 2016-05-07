#ifndef DATALOGGING_H
#define DATALOGGING_H
#include <time.h>
#include "stm32f4xx_rtc.h"
extern RTC_TimeTypeDef time_struct;
extern RTC_DateTypeDef date_struct;

struct time_data{
	RTC_TimeTypeDef time_log_struct;
	RTC_DateTypeDef date_log_struct;
};
typedef struct time_data time_data;

struct record_keeper{
	time_data records[500];
	int head;
	int num_records;
};
typedef struct record_keeper record_keeper;

void save_record(record_keeper*, RTC_TimeTypeDef, RTC_DateTypeDef);
void transmit_records(record_keeper*);

#endif
