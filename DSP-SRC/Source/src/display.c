
#include "display.h"
#include "stm32f4xx.h"

void display_init(void) {
	
    //Initialize system
    //SystemInit();
    
    //Initialize LCD 20 cols x 4 rows
    TM_HD44780_Init(16, 2);
    
    //Put string to LCD
    TM_HD44780_Puts(0, 0, "Fuck Don1");
 
    //Wait a little
    //Delayms(3000);
     
}