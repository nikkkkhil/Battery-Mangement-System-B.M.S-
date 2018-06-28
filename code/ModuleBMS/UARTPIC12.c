

#include <stdint.h>        /* For uint8_t definition */
#include <stdbool.h>       /* For true/false definition */

#include "system.h"

#include "UARTPIC12.h"      /*INBIT, TIMEOUT, BAUD definitions*/
#include <conio.h>
#include <xc.h>
#include "user.h"

uint8_t	getch(void)
//Reads an 8 bit RS-232 signal from the port defined in INBIT
//Returns the 8 bit value
//0 is returned if no start bit is received within TIMEOUT loop iterations
//Uses BAUD to set the baud rate.

{
    uint16_t TimeOut = TIMEOUT;//Time out counter

    for(;TimeOut;TimeOut--)                    //loop until break or timeout
    {
        if (PIR1bits.RCIF) break;
    }

    if (RCSTAbits.FERR || RCSTAbits.OERR)
    {//Framing error or overrun error, clear error and return 0.
        int8_t dummychar; //Need to read the char to clear the error, so this is our dummy
        dummychar = RCREG; //Get byte from the receive register
        RCSTAbits.CREN = 0;//Clear the overrun error
        RCSTAbits.CREN = 1; //Enable reception
        return 0;   //error
    }
    if (TimeOut == 0)
    {
        return 0;   //Return 0 if we timed out before getting a character
    }
    
    return RCREG; //This is the character
}


void	putch1(uint8_t SerBuf)
{
//Writes SerBuf to the UART to be transmitted

    TXREG = SerBuf;
    __delay_us(DELAY); //The modules need a delay here to allow processing time.
    return;
}
