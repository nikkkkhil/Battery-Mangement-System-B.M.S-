

#include <stdint.h>        /* For uint8_t definition */
#include <stdbool.h>       /* For true/false definition */

#include "system.h"
#include "user.h"         /* For global variables*/

#include "UARTPIC16.h"       /*INBIT, TIMEOUT, BAUD definitions*/
#include <conio.h>
#include <xc.h>

void	putch1(uint8_t SerBuf)
{
//Writes SerBuf to the UART to be transmitted

    TXREG = SerBuf;
    __delay_us(DELAY); //The modules need a delay here to allow processing time.
    return;
}

void	putch2(uint8_t SerBuf)
{
//Writes SerBuf to OUTBIT2 as an 8 bit RS-232 signal
//Data bits = 8
//Stop bits = 1
//Parity = none
//Uses BAUD to set the baud rate

    uint8_t BitCount = 7;       //8 bits in the output

    OUTBIT2 = 0;  //Start Bit - set in mirror byte
    OUTPORTS  //write it to the port

    __delay_us(BAUD);   //keep it there for the required time

    do
    {
        OUTBIT2 = SerBuf & 1; //Mask out all but bit 0,
        OUTPORTS  //write it to the port
        SerBuf = SerBuf >> 1;       //Shift right
        __delay_us(BAUD-LOOPTIME);  //Keep the bit there for the required time

    } while (BitCount--);
    //Now we need to transmit a stop bit (1)
    OUTBIT2 = 1;
    OUTPORTS
    __delay_us(BAUD);     //stop bit

    return;
}
