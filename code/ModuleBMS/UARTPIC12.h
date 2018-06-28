/* 
 * File:   UARTPIC12.h
 * Author: Christopher Walkden
 *
 * Created on 11 August 2013, 3:50 PM
 *
 * File to define UART parameters for the PIC12 in the BMS Module circuit
 */

#ifndef UART_H
#define	UART_H

#define TIMEOUT 65535U //The number of loops to execute before getch() times out.
                          //65535 is approximately 225ms for 16MHz oscillator.
#define DELAY 1500       //uS delay for modules after sending char. Make this a bit longer than the time
                        //to transmit an entire char

#endif	/* UART_H */

#include <xc.h>


//Functions
uint8_t	getch(void);
void putch1(uint8_t SerBuf);
