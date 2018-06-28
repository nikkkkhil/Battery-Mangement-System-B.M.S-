/* 
 * File:   lcd_drv.h
 * Author: root
 *
 * Created on September 9, 2013, 10:55 AM
 */

#ifndef LCD_DRV_H
#define	LCD_DRV_H

#include <xc.h>         /* XC8 General Include File */
#include <stdint.h>        /* For uint8_t definition */
#include <stdbool.h>       /* For true/false definition */
#include <stdio.h>



void lcdPutC(char c);
void lcdGoto(int8_t x, int8_t y);
void initLCD (void);
void lcdSendByte(int8_t address, int8_t n);
void lcdSendNibble(int8_t nibble);
void lcdPrint(const char *lcdstring);


#endif	/* LCD_DRV_H */

