
 
#ifndef __UART_H
#define __UART_H

#include <stm32f4xx.h>
#include <misc.h>			 // I recommend you have a look at these in the ST firmware folder
#include <stm32f4xx_usart.h> // under Libraries/STM32F4xx_StdPeriph_Driver/inc and src

#define MAX_STRLEN 12 // this is the maximum string length of our string in characters

void UART_Initialize(void);
void NVICInitialize(void);
void USART_puts(volatile char *s);
#endif
