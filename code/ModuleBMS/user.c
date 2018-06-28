/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/

#if defined(__XC)
#include <xc.h>         /* XC8 General Include File */
#elif defined(HI_TECH_C)
#include <htc.h>        /* HiTech General Include File */
#endif

#include <stdint.h>         /* For uint8_t definition */
#include <stdbool.h>        /* For true/false definition */

#include "user.h"
#include "UARTPIC12.h"

/******************************************************************************/
/* User Functions                                                             */
/******************************************************************************/

/* <Initialize variables in user.h and insert code for user algorithms.> */

void InitApp(void) {
    /* Initialize User Ports/Peripherals/Project here */
    TRISAbits.TRISA5 = 1; //PORTA bit 5 input (optocoupler rs-232)
    TRISAbits.TRISA2 = 0; //PORTA bit 2 output (balancing resistors)
    TRISAbits.TRISA0 = 1; //PORTA bit 0 input (thermistor)
    TRISAbits.TRISA1 = 1; //PORTA bit 1 input (voltage reference)
    TRISAbits.TRISA4 = 0; //PORTA bit 4 output (RS-232 Dout)
    ANSELAbits.ANSA0 = 1; //PORTA bit 0 analog input
    ANSELAbits.ANSA1 = 1; //PORTA bit 1 analog input
    PORTAbits.RA2 = 0; //PORTA bit 2 (balancing resistors) initialised low
    PORTAbits.RA4 = 1;  //Initialised at RS-232 logic 0 = high

    PORTAMirror.RA2 = 0;  //Echoed in the PORTA mirror
    PORTAMirror.RA4 = 1;  //

    ADCON1bits.ADCS = 7; //ADC clock setup = FRC
    ADCON1bits.ADFM = 1; //ADC high/low byte selection (1 = right justified)
    ADCON1bits.ADPREF0 = 0; //Voltage reference = VRef pin.
    ADCON1bits.ADPREF1 = 1; //Voltage reference = VRef pin.
    ADCON0bits.CHS = 0; //Select analogue channel 0
    ADCON0bits.ADON = 1; //Enable ADC
    ADCON0bits.GO_nDONE = 1; //Start ADC

    T1CONbits.TMR1ON = 1;
    T1GCONbits.TMR1GE = 0; //These two bits initialise TIMER1 "always on"
    T1CONbits.TMR1CS1 = 0;
    T1CONbits.TMR1CS0 = 1;
    T1CONbits.T1OSCEN = 0; //These three bits set TIMER1 clock to internal
    T1CONbits.T1CKPS = 3; //Prescaler for TIMER1 = 8
    TMR1H = 0;
    TMR1L = 0; //Initialise TIMER1 value

    TMR1IF = 0; //Clear TIMER1 Interrupt
    PIE1bits.TMR1IE = 1; // 1 = Enable TIMER1 Interrupt
    INTCONbits.PEIE = 1;
    INTCONbits.GIE = 1;

    APFCONbits.RXDTSEL = 1; //Move UART Rx to RA5 pin
    APFCONbits.TXCKSEL = 1; //Move UART Tx to RA4 pin

//UART Baud rate = 9600:
    SPBRGL = 24;
    SPBRGH = 0;
    TXSTAbits.SYNC = 0;
    TXSTAbits.BRGH = 0;
    BAUDCONbits.BRG16 = 0;
//UART TX
    BAUDCONbits.SCKP = 0;   //1=Inverted logic
    RCSTAbits.SPEN = 1; //Serial Port Enable
    TXSTAbits.TXEN = 1; //TX Enable
//UART RX
//    PIE1bits.RCIE = 1;  //Enable inerrupts for RX
    RCSTAbits.CREN = 1; //Enable reception




    //Watchdog Timer:
    WDTCONbits.WDTPS = 0b01100; //Watchdog delay time 01100 = approx 4s.


}

