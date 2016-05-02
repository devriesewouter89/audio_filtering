#include "main.h"
#include <time.h>

typedef struct {
 uint32_t offset;
 time_t data[500]; //8 bytes
} data_struct;

void USART_puts(volatile char *s);
void handle_USART_message(char *);
