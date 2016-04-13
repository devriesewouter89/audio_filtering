#include "stm32f4xx_usart.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"


void UART_Initialize(void)
{
 /* Enable peripheral clock for USART1 */
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
 
USART_InitTypeDef USART_InitStructure;

/* USART1 configured as follow:
 * BaudRate 9600 baud
 * Word Length 8 Bits
 * 1 Stop Bit
 * No parity
 * Hardware flow control disabled
 * Receive and transmit enabled
 */
 USART_InitStructure.USART_BaudRate = 9600;
 USART_InitStructure.USART_WordLength = USART_WordLength_8b;
 USART_InitStructure.USART_StopBits = USART_StopBits_1;
 USART_InitStructure.USART_Parity = USART_Parity_No;
 USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
 USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
 
USART_Init(USART1, &USART_InitStructure); // USART configuration
USART_Cmd(USART1, ENABLE); // Enable USART
}
 
void GPIOInitialize(void)
{
 RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); //Enable clock for GPIOB
 GPIO_InitTypeDef GPIO_InitStructure;
/* USART1 Tx on PB6 | Rx on PB7 */
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
 GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
 GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
 
GPIO_Init(GPIOB, &GPIO_InitStructure);
 GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1);//Connect PB6 to USART1_Tx
 GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1);//Connect PB7 to USART1_Rx
}
 
/*void NVICInitialize(void)
{
 NVIC_InitTypeDef NVIC_InitStructure;
 NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
 NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
 NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
 NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
 
NVIC_Init(&NVIC_InitStructure);
 USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
 
}
 */
void USART_puts(volatile char *s){
 
while(*s){
 // wait until data register is empty
 while( !(USART1->SR & 0x00000040) );
 USART_SendData(USART1, *s);
 *s++;
 }
}