/* 
 * File:   main.h
 * Author: root
 *
 * Created on 3 August 2013, 3:15 PM
 */

#ifndef MAIN_H
#define	MAIN_H

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* MAIN_H */

/*GLOBAL VARIABLES*/

uint8_t int_cnt = 0;   //These are used for the comms watchdog.  IntCount counts up
bool timeout = false;       //via timer interrupt, and is reset to zero when comms is recieved.


/*Function Stubs*/

void Timer1Int(uint8_t *IntCount, bool *timeout);
uint8_t rx_stream(uint8_t *arryptr1, uint8_t *arryptr2, bool *timeout, uint8_t *int_cnt);
void tx_stream(uint8_t *arryptr1, uint8_t *arryptr2, uint8_t *arraysize,
                    uint8_t *TVar, uint8_t *VVar);
void Analogues(uint8_t *Volts, uint8_t *Temp);

