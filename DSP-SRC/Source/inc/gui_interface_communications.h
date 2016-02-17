#include "main.h"

typedef struct {
 uint32_t offset;
 uint64_t data[500]; //8 bytes
} data_struct;

void USART_puts(volatile char *s);
void USART_put_data(uint8_t data);