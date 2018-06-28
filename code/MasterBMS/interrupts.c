/******************************************************************************/
/*Files to Include                                                            */
/******************************************************************************/

#if defined(__XC)
    #include <xc.h>         /* XC8 General Include File */
#elif defined(HI_TECH_C)
    #include <htc.h>        /* HiTech General Include File */
#endif

#include <stdint.h>         /* For uint8_t definition */
#include <stdbool.h>        /* For true/false definition */
#include "main.h"
#include "user.h"

/******************************************************************************/
/* Interrupt Routines                                                         */
/******************************************************************************/

#ifndef _PIC12

void interrupt isr(void)
{
    /* This code stub shows general interrupt handling.  Note that these
    conditional statements are not handled within 3 seperate if blocks.
    Do not use a seperate if block for each interrupt flag to avoid run
    time errors. */


    /* Determine which flag generated the interrupt */
    if(TMR2IF)
    {//Timer 2 has expired:
        Timer2Int(&int_cnt, &timeout);
        TMR2IF=0; /* Clear Interrupt Flag 1 */
    }
    else if (RCIF)
    {//UART has received a character
        RXInt();  
    }

}


#endif


