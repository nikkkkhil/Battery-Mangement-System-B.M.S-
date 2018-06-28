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

/******************************************************************************/
/* Interrupt Routines                                                         */
/******************************************************************************/

/* Baseline devices don't have interrupts. Unfortunately the baseline detection 
 * macro is named _PIC12 */

#ifndef _PIC12

void interrupt isr(void)
{
    /* This code stub shows general interrupt handling.  Note that these
    conditional statements are not handled within 3 seperate if blocks.
    Do not use a seperate if block for each interrupt flag to avoid run
    time errors. */

    
    /* Determine which flag generated the interrupt */
    if(TMR1IF)
    {
        TMR1IF=0; /* Clear Interrupt Flag 1 */
        Timer1Int(&int_cnt, &timeout);
    }
/*    else if (<Interrupt Flag 2>)
    {
        <Interrupt Flag 2=0>; 
    }  */
    else
    {
        /* Unhandled interrupts */
    }

}
#endif


