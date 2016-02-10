#include "main.h"

typedef struct {
 uint8_t offset;
 uint8_t data[500];
} data_struct;

void USART_puts(volatile char *s);
void USART_put_data(uint8_t data);