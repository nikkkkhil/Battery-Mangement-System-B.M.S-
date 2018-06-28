/*
 * File:   main.h
 * Author: root
 *
 * Created on 8 August 2013, 6:38 PM
 */

#ifndef MAIN_H
#define	MAIN_H

//Global variables and constants:

#define VERSION1 "PIC16F1827\n" //ID and
#define VERSION2 "Version 1.31" //version number of code to display on LCD

uint8_t  int_cnt=0;  //Counts the timer interrupts. Reset this as a watchdog on comms.
bool timeout = false;   //Set to true when InterruptCounter is not reset.
#define TIMEOUTVAL 225   //Number of interrupts before we timeout from comms

//PWM control variables & constants:
#define ChargerLimit 26  //Value to limit charging current. 24 = approx 1A on TC Charger
#define ChargerOff 0x00     //Value to turn charging off
#define ChargerOn 0xff      //Value to turn charging on full

#define CELLMAX 50 //Maximum number of cells + 2 (needs to be an even number)

#define MaxCellVolts 4250    //Max cell Voltage (milliVolts).  Alarm and disable charger if one exceeds.3800
#define MinCellVolts 2750    //Min cell Voltage in milliVolts.  Alarm if one is lower.2650
const int8_t BalVal = 35;   //30=3.65, 35=3.60, 45=3.50 V. Cell balancing value. Raw value, so higher number is lower voltage
#define SlowCharger   3600   //Reduce charger current if one cell goes higher than this (mV)
#define SlowBand 150  //Go to normal charger current if cell Voltage down to SlowCharger - SlowBand
#define MaxCellTemp 60      //Alarm if a cell gets hotter than this
#define MaxPackVolts 600000   //Turn off charger if the pack reaches this voltage (charger must be faulty)

static uint8_t No_Of_Cells=(CELLMAX-2);  //The number of cells (modules). Is set when we receive some voltage data.

uint8_t Volts [CELLMAX];  //Arrays for cell data
uint8_t Temps [CELLMAX];
static uint8_t FaultCode;  //0 = no fault, 1,2 = comms, 3 = cell under Volts, 4 = pack over Volts,
                           //5 = cell over Volts, 6 = cell over temp (4 or greater turn off charger)


//Function prototypes:
void Timer2Int(uint8_t *IntCount, bool *timeout);
void RXInt();

void tx_Volts();
void tx_Temps();
void tx_streamPC(uint8_t *arryptr);
int24_t HiLow(uint8_t *Volts, uint8_t *Temps,
                    uint8_t *V_Max, uint8_t *V_Min, uint8_t *HiVCell, uint8_t *LoVCell,
                    uint8_t *T_Max, uint8_t *T_Min, uint8_t *HiTCell, uint8_t *LoTCell);
void MyItoA(int16_t n, uint8_t *s);
bool SameString(uint8_t *s1, uint8_t *s2);
void CopyString(uint8_t *s1, uint8_t *s2);
void reverse(uint8_t *s);
void Fmt1Point(uint8_t *s);
void Fmt2Point(uint8_t *s);
void DisplayIt(uint8_t *s);
int16_t Convert_Volts(uint8_t Array_Val);
uint8_t Convert_Temp(uint8_t Array_Val);
uint8_t DoBalance(bool Balance);
void init(void);
//int16_t TotalVolts(uint8_t* Volts);
void Charger(uint24_t PackVolts, uint8_t *FaultCode, uint8_t MaxCellVoltage, uint8_t NoOfShunts, uint8_t *ChargerState);
void FaultCheck (uint8_t *V_Max, uint8_t *V_Min, uint8_t *T_Min, uint8_t *FaultCode);


#endif	/* MAIN_H */
