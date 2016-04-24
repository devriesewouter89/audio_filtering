#ifndef GUI_INTERFACE_COMMUNICATIONS
#define GUI_INTERFACE_COMMUNICATIONS

#include <main.h>
#include <string.h>

void USART_puts(volatile char *s);
void USART_put_data(uint8_t data);
void poll_USART(void);
void poll_Alarm(void);

#endif