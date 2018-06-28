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

/******************************************************************************/
/* User Functions                                                             */
/******************************************************************************/

/* <Initialize variables in user.h and insert code for user algorithms.> */

void InitApp(void)
{
    TRISAbits.TRISA0 = 0; //PORTA bit 0 output (Charger Current Limit)
    TRISAbits.TRISA1 = 0; //PORTA bit 1 output (LCD)
    TRISAbits.TRISA2 = 1; //PORTA bit 2 input  (Switch 1)
    TRISAbits.TRISA3 = 1; //PORTA bit 3 input  (Switch 2)
    TRISAbits.TRISA4 = 1; //PORTA bit 4 input  (Not Used)
    TRISAbits.TRISA5 = 1; //PORTA bit 5 input  (Not Used except in programming)
    TRISAbits.TRISA6 = 0; //PORTA bit 6 output (2nd port RS232 out)
    TRISAbits.TRISA7 = 0; //PORTA bit 7 output (LCD)

    TRISBbits.TRISB0 = 0; //PORTB bit 0 output (Charger Control) 
    TRISBbits.TRISB1 = 1; //PORTB bit 1 input  (Data in from optocoupler)
    TRISBbits.TRISB2 = 0; //PORTB bit 2 output (1st port RS232 out)
    TRISBbits.TRISB3 = 0; //PORTB bit 3 output (LED control)
    TRISBbits.TRISB4 = 0; //PORTB bit 4 output (LCD)
    TRISBbits.TRISB5 = 0; //PORTB bit 5 output (LCD)
    TRISBbits.TRISB6 = 0; //PORTB bit 2 output (LCD)
    TRISBbits.TRISB7 = 0; //PORTB bit 1 output (LCD)

/*Iniitalise the output ports*/
    OUTBIT2 = 1;        //RS-232 port
    OUTBIT1 = 1;        //RS-232 port
    CHARGER = 0;        //Charger Enabled
    OUTPORTS

    ANSELAbits.ANSA2 = 0;   //PORTA bit 2 ditital input
    ANSELAbits.ANSA3 = 0;   //PORTA bit 3 ditital input
    ANSELBbits.ANSB1 = 0; //PORTB bit 1 digital input

//Timer 1:
/*    T1CONbits.TMR1ON = 1;
    T1GCONbits.TMR1GE = 0; //These two bits initialise TIMER1 "always on"
    T1CONbits.TMR1CS1 = 0;
    T1CONbits.TMR1CS0 = 1;
    T1CONbits.T1OSCEN = 0; //These three bits set TIMER1 clock to internal
    T1CONbits.T1CKPS = 2; //Prescaler for TIMER1 3 = 8, 2 = 4
    TMR1H = 0;
    TMR1L = 0; //Initialise TIMER1 value */
//Timer 2:
    T2CONbits.TMR2ON = 1;
    T2CONbits.T2CKPS = 3;   //Prescaler for Timer 2, 0=1, 1=4, 2=16, 3=64
    T2CONbits.T2OUTPS = 3;  //Postscaler for Timer 2. 0=1, 1=2, 9=10 etc. up to 15=16

//    TMR1IF = 0; //Clear TIMER1 Interrupt
    TMR2IF = 0; //Clear TIMER2 Interrupt
//    PIE1bits.TMR1IE = 1; // 1 = Enable TIMER1 Interrupt
    PIE1bits.TMR2IE = 1; // 1 = Enable TIMER2 Interrupt
    INTCONbits.PEIE = 1;    //1 = Peripheral interrupt enable (0 disables all peripheral interrupts)
    INTCONbits.GIE = 1;     //1 = Global interrupt enable (0 disables all interrupts)
    
//    INTCONbits.IOCIE = 1;   //Enable Interrupt On Change interrupts (Port B)
//    IOCBP = 0;//No rising edge interrupts
//    IOCBN = 2;//Falling edge interrupt for Port B pin 1.

//PWM configuration for Port B pin 0 (charger control). Uses timer 4.
    TRISBbits.TRISB0 = 1; //Disable output while we set it up
    APFCON0bits.CCP1SEL = 1;    //Make RB0 CCP1 (would normally be on RB3)
    PR4 = 0xFF;   //This is the period of the waveform. Period = (PR4+1)*0.000000125*Prescalar. FF = 1.9kHz, decrease for faster
    CCP1CONbits.CCP1M = 0B1100; //PWM mode
    CCPR1L = 0xf0;  //Most significant 8 bits of the 10 bit duty cycle
    CCP1CONbits.DC1B = 0B11; //Least significant 2 bits of the 10 bit duty cycle
    CCPTMRSbits.C1TSEL = 1; //PWM using CCP1 is based on timer 4
    TMR4IF = 0;         //Clear the timer 4 interrupt flag.
    T4CONbits.T4CKPS = 2;   //Prescaler for Timer 4, 0=1, 1=4, 2=16, 3=64
    T4CONbits.TMR4ON = 1;   //Turn timer on
    TRISBbits.TRISB0 = 0; //Enable the output now it is set up

//UART Baud rate = 9600:
    SPBRGL = 24;//832;
    SPBRGH = 0;
    TXSTAbits.SYNC = 0;
    TXSTAbits.BRGH = 0;
    BAUDCONbits.BRG16 = 0;
//UART TX
    BAUDCONbits.SCKP = 0;   //1=Inverted logic
    RCSTAbits.SPEN = 1; //Serial Port Enable
    TXSTAbits.TXEN = 1; //TX Enable
//UART RX
    PIE1bits.RCIE = 1;  //Enable inerrupts for RX
    RCSTAbits.CREN = 1; //Enable reception


//Watchdog Timer
    WDTCONbits.WDTPS = 0B01100; //01100 = approx 4s watchdog delay time

}

