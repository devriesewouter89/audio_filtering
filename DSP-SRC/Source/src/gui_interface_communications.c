#include "main.h"
/*
struct data_struct{
 uint8_t offset;
 uint8_t data[500];
};
*/

//Senior Design Functions
void USART_puts(volatile char *s)
{
	while(*s){
		// wait until data register is empty
		while( !(USART1->SR & 0x00000040) );
		USART_SendData(USART1, *s++);
	}
}

void USART_put_data(uint8_t data)
{
  // wait until data register is empty
  while( !(USART1->SR & 0x00000040) );
  USART_SendData(USART1, data);
}

//stm32f4xx_it.c
			//if( strcmp((const char *)USART1_gets,"GET") ){
			//		USART_puts("EXAMPLEDATA1,EXAMPLEDATA2,EXAMPLEDATA3,EXAMPLEDATA4\0");
			//		strcpy((char *)USART1_gets, "NOT\0");
			//} else {
			//		USART_puts("This shouldn't print often.\n");
			//}
//Interrupt test. Will try to build a string by itself.

/*
void FLASH_write(int x){
	uint8_t flash_status = FLASH_COMPLETE;
	
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR |FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR |FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
	//Header puts Sector_15 as the 8 bytes between 0x0098 and 0x00A0. That seems unreasonably low? programering was talking about there being 11 sectors about 128k each?
	flash_status = FLASH_EraseSector(FLASH_Sector_15, VoltageRange_3);
	
	if (flash_status != FLASH_COMPLETE) {
		USART_puts("Flash Erase failed. Aborting write.\n");
		FLASH_Lock();
		return;
  }
  
	Here is where all the links below diverge. Actually writing to flash memory. There are some considerations about aligning(?) the data. I think I might be able to do
	for(int64_t address = start_address; address < end_address; address += 8) //Q: Do we need to jump by however many bytes we want to write into memory? I'd think so.
		*(address) = uint64_t_data_array[i++];
	
	and then to read:
	for(int64_t address = start_address; address < end_address; addredd += 8)
	 uint_64_t_data_array[i++] = *(address);
	
	uint8_t* address = &flash_data[0];
	http://www.eevblog.com/forum/microcontrollers/stm32f4-saving-data-to-internal-flash-(at-runtime)/ //Short forum discussion where people conveniently leave out everything that matters
	http://www.programering.com/a/MjMxQTMwATg.html //Jesus christ, guy can neither spell nor comment his goddamn arcane gibberish
	http://micromouseusa.com/?p=1556 Most useful start. Good code examples
	
	Justin suggest maybe using a card reader and external flash memory
	Get USART interaction between program and Microcontroller working
	
}
*/