/* 
 * File:   UARTPIC16.h
 * Author: Christopher Walkden
 *
 * Created on 11 August 2013, 3:50 PM
 *
 * File to define UART parameters for the PIC16 in the BMS Master circuit
 */
/*
 Note that two UARTs are used.  The on-chip (automatic) UART is used to talk to the BMS
 Modules.  Another UART is created in software using PORTA bit 6, this one is used to
 transmit data to the PC (no receive).*/



#ifndef UART_H
#define	UART_H

#define BAUD    104    //Delay in uS to set BAUD rate for RS-232.
                        //9600 baud, BAUD = 104, LOOPTIME = 10
                        
#define LOOPTIME 10 //The time in uS the PIC takes to exectute instructions in the loop.
                    //This value is a fudge factor, determined by experiment.
#define TIMEOUT 65535U//The number of loops to execute before getch() times out.
                          //65535 is approximately 225ms for 16MHz oscillator.
#define DELAY 1500       //uS delay for modules after sending char. Make this a bit longer than the time
                        //to transmit an entire char


#endif	/* UART_H */

#include <xc.h>

void putch1(uint8_t SerBuf);
void putch2(uint8_t SerBuf);