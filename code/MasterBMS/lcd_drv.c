
#include "lcd_drv.h"
#include <stdint.h>        /* For uint8_t definition */
#include <stdbool.h>       /* For true/false definition */
#include "system.h"
#include "user.h"         /* For global variables*/

#include <xc.h>




// Define the LCD type and details
#define lcd_type 2        // 0=5x7, 1=5x10, 2=2 lines
#define lcd_line_two 0x40 // LCD RAM address for the 2nd line
 
 

// Send a nibble to the display
void lcdSendNibble(int8_t nibble) {
    // Note:  !! converts an integer expression
    // to a boolean (1 or 0).
    LCD_DB4 = !!(nibble & 1);
    LCD_DB5 = !!(nibble & 2);
    LCD_DB6 = !!(nibble & 4);
    LCD_DB7 = !!(nibble & 8);
    
    OUTPORTS
   
    __delay_us(2);

    LCD_E = 1;
    OUTPORTS
    __delay_us(2);
    LCD_E = 0;
    OUTPORTS
}
 
 
// Send a byte to the display.
void lcdSendByte(int8_t address, int8_t n) {
    LCD_RS = 0;
    OUTPORTS
    __delay_us(60);
 
    if(address) {
        LCD_RS = 1;
    }  
    else {
        LCD_RS = 0;
    }   
    OUTPORTS

    __delay_us(2);
    LCD_E = 0;
    OUTPORTS
 
    lcdSendNibble(n >> 4);
    lcdSendNibble(n & 0xf);
}
 
 
// Initialise the display.  This is the outline of what is
// required:
// 1 : Wait 20ms for the display to stabilise after power-up
// 2 : Set RS and ENABLE low
// 3 : Wait at least 200us
// 4 : Send nibble cmd 0x03
// 5 : Wait at least 200us
// 6 : Send nibble cmd 0x03 again
// 7 : Wait at least 200us
// 8 : Send nibble cmd 0x03 a third time
// 9 : Wait at least 200us
// 10: Send nibble cmd 0x02 to enable 4-bit mode
// 11: Wait 5ms
// 12: Send byte cmd 0x28 (4-bit communications, 2 lines, 5x8)
// 13: Send byte cmd 0x0c (Turn the display on)
// 14: Send byte cmd 0x01 (Clear and home the display)
// 15: Send byte cmd 0x06 (Set left-to-right direction)
void initLCD (void) {
    int8_t i;
    LCD_RS = 0;
    LCD_E = 0;
    OUTPORTS
 
    __delay_ms(15);
 
    for(i = 0; i < 3; i++) {
        lcdSendNibble(0x03);
        __delay_ms(5);
    }
 
    lcdSendNibble(0x02); // Home the cursor
 
    lcdSendByte(0, 0x28); // 4-bit, 2 line, 5x8 mode
    __delay_ms(5);
    lcdSendByte(0, 0x0C); // Turn on display
    __delay_ms(5);
    lcdSendByte(0, 0x01); // Clear & home display
    __delay_ms(5);
    lcdSendByte(0, 0x06); // Left to Right
    __delay_ms(5);
}
 
 
// Position the cursor on the display.  (1, 1) is the top left
void lcdGoto(int8_t x, int8_t y) {
    int8_t address;
    address = (y != 1)? lcd_line_two : 0;
    address += x-1;
    lcdSendByte(0, 0x80 | address);
}
 
 
// Write characters to the display
void lcdPutC(char c) {
    switch(c) {
        case '\f':
            lcdSendByte(0,1);
            __delay_ms(2);
            break;
     
        case '\n':
            lcdGoto(1,2);
            break;
     
        case '\b':
            lcdSendByte(0,0x10);
            break;
     
        default:
            lcdSendByte(1,c);
            break;
    }
}

void lcdPrint(const char *lcdstring)
{//puts the string on the LCD. Use 0x00 as a string terminator
    int i = 0;
    while (*(lcdstring+i) != 0x00)
    {
        lcdPutC(*(lcdstring+i));
        i++;
    }
}

 