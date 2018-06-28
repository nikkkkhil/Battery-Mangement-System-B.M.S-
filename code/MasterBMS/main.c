/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/

#include <xc.h>         /* XC8 General Include File */

#include <stdint.h>        /* For uint8_t definition */
#include <stdbool.h>       /* For true/false definition */

#include "system.h"        /* System funct/params, like osc/peripheral config */
#include "user.h"          /* User funct/params, such as InitApp */
#include "main.h"
#include <stdio.h>
#include "UARTPIC16.h"      //RS-232 comms functions for the PIC16
#include "lcd_drv.h"        //Driving the LCD on the BMS Master board


void Timer2Int(uint8_t *IntCount, bool *timeout)
//Called by the interrupt function.
//Sets timout to true if IntCount gets high enough.  Used to check for comms timeout.
{
    if (++(*IntCount) == TIMEOUTVAL)
    {
        *timeout = true;
        *IntCount = 0;
    }

}


void tx_Volts()
//Transmits a Voltage request to the modules.
{
    putch1(0x01);   //Command 01 = Volts
    putch1(0x01);   //CRC value.
    putch1(0xff);
}

void tx_Temps()
//Transmits a Temperature request to the modules.
{
    putch1(0x03);   //Command 03 = Temps
    putch1(0x03);   //CRC value.
    putch1(0xff);
}

void tx_streamPC(uint8_t *arryptr)
//Transmits the data stream from the array pointed to by arrayptr.
//Finishes when it gets to 0xff.
//Transmits to the PC RS-232 port
//Converts each byte to ASCII (hex) and inserts a comma, eg 0x34 becomes "34,".
{
    uint8_t scratch=0;

    do
    {
        scratch = (*arryptr & 0xf0); //Mask out the bottom half
        scratch = scratch / 0x10;   //Divide it so that we get the left digit in the right side
        if (scratch <= 0x9) putch2('0' + scratch);  //Convert to ASCII digit and send
        if (scratch > 0x9) putch2('a' + (scratch-0xa));   //Convert to ASCII digit and send

        scratch = (*arryptr & 0x0f); //Mask out the top half
        if (scratch <= 0x9) putch2('0' + scratch);  //Convert to ASCII digit and send
        if (scratch > 0x9) putch2('a' + (scratch-0xa));   //Convert to ASCII digit and send

        putch2(',');

       // __delay_ms(10); //The modules need a delay here to allow processing time. (this is PC, so probably doesn't need this)
    } while (*(arryptr++) != 0xff);

    putch2(0xa);    //LF character
}




void RXInt()
//Recieves a data byte into one of the arrays.
//If the data strean was not valid, FaultCode is set to 1.
//A valid stream starts with a command (1-5), finishes with 0xff, second last is CRC, and is
//CELLMAX size or smaller.
//The 0xff is stored in the array but the CRC is not.
//timeout contains a bool which is set true by a timer interrupt. This func
//ends (returns 0) if timeout goes true.

//Called by the interrupt function when char received. That function clears
//the interrupt flag after calling this function.

{
    uint8_t charbuffer; //temporary place to put an array element
    static uint8_t i;          //for indexing the array
    static bool DoVolts, DoTemp, Ignore;    //What do we do with this data?
    timeout = false;
    int_cnt = 0;       //set this to 0 to indicate that we are doing something.
                        //it is incremented by a timer (using interrupts), and
                        //timeout is set to true if it gets too big.
    if (FaultCode == 1) FaultCode = 0; //If there has been a comms fault then reset it now.
    if (RCSTAbits.FERR || RCSTAbits.OERR) //If there was a framing error on this char
    {//Framing error or overrun error:
        Ignore = true;  //Ignore the rest of the data
        FaultCode = 1;  //Flag a comms fault
        charbuffer = RCREG; //Get byte from the receive register
        RCSTAbits.CREN = 0;//Clear the overrun error
        RCSTAbits.CREN = 1; //Enable reception
    }
    
    charbuffer = RCREG; //Get byte from the receive register

    if(!DoVolts&&!DoTemp&&!Ignore)
    {//This is the first byte - should be a command (1-5)
        switch (charbuffer)
        {
            case 1: //Volts command
                DoVolts = true;
                if (FaultCode == 2) FaultCode = 0; //No comms fault now, reset FaultCode
                break;
            case 2: //Balance command, ignore this
                Ignore = true;
                break;
            case 3: //Temps
                DoTemp = true;
                break;
            case 4: //Module comms error. Need to store the next byte, will store it in the Temps array.
                DoTemp = true;
                FaultCode = 2;
                break;
            case 5: //Module comms ping, ignore this.
                Ignore = true;
                break;
           default: //First byte, but wasn't a command.  Flag a comms fault (1).
               Ignore=true; //Will ignore (not store) the rest of this data up to 0xff
               FaultCode = 1;
               break;
        }
    }

    if(DoVolts)
    {
        if ((i>CELLMAX) || (charbuffer==0xff))
        {//This is the last received char.  The previous one (i-1) was the CRC, which we overwrite (we don't use the CRC).
            Volts[i-1] = charbuffer;  //0xff as the terminator
            No_Of_Cells = i - 2;    //A complete, valid data stream.
        }   //i - 1 (for the command) - 1 (for the CRC) = No. of modules
        else 
        {
            Volts[i]=charbuffer; //Store the char in the Volts array
        }
    }

    if(DoTemp)
    {
        if ((i>CELLMAX) || (charbuffer==0xff))
        {//This is the last received char.  The previous one (i-1) was the CRC, which we overwrite (we don't use the CRC).
            Temps[i-1] = charbuffer;  //0xff as the terminator
        }
        else
        {
            Temps[i]=charbuffer;    //Store the char in the Temps array
        }
    }
   
    i++;
   
   if((i>(CELLMAX+1))||(charbuffer==0xFF))
   {//We have recieved the last char, reset ready for next time.
       i=0;
       DoVolts=false;
       DoTemp=false;
       Ignore=false;
    }
}



int24_t HiLow(uint8_t *Volts, uint8_t *Temps,
                uint8_t *V_Max, uint8_t *V_Min, uint8_t *HiVCell, uint8_t *LoVCell,
                uint8_t *T_Max, uint8_t *T_Min, uint8_t *HiTCell, uint8_t *LoTCell)
// find max and min values of volts and temp
// Returns total voltage.
// Max and min values are averaged over 4 readings.
//Note that V_Min, V_Max, HiCell and LoCell all refer to the array values
//not the actual cell voltages ie everything is upside down!
{
    int24_t BattVolts=0;
    uint8_t i = 1;  //counter for arrays, start at the first datum (skip the command)
//Temporay places to store current values before averaging:
    uint8_t NowV_Max=0, NowV_Min=0xFF;
    uint8_t NowT_Max=0, NowT_Min=0xFF;

    if (*Volts == 0x01)
    { //It is a valid array.
        while(Volts[i] != 0xFF)
        {   
            if (Volts[i] > NowV_Max)
            {
                NowV_Max = Volts[i];
                *HiVCell = i;
            }
            if (Volts[i] < NowV_Min)
            {
                NowV_Min = Volts[i];
                *LoVCell = i;
            }
                             // calculate total battery volts
            BattVolts = BattVolts + (1263405/(316+(int32_t)Volts[i]));
            i++;
        }
        //Now for the rolling average calculations:
        *V_Min = ( ((*V_Min*3) + (NowV_Min)) / 4);
        *V_Max = ( ((*V_Max*3) + (NowV_Max)) / 4);
    }
    if (*Temps == 0x03)
    {//It is a valid array.
        i = 1;

        while(Temps[i] != 0xFF)
        {
            if (Temps[i] > NowT_Max)
            {
                NowT_Max = Temps[i];
                *HiTCell = i;
            }
            if (Temps[i] < NowT_Min)
            {
                NowT_Min = Temps[i];
                *LoTCell = i;
            }
            i++;
        }
        //Now for the rolling average calculations:
        *T_Min = ( ((*T_Min*3) + (NowT_Min)) / 4);
        *T_Max = ( ((*T_Max*3) + (NowT_Max)) / 4);
    }

    return BattVolts;

}


//Convert int16_t n to 5 char string in s[]
void MyItoA(int16_t n, uint8_t *s){
   uint8_t i;
   for(i = 0; i < 5; i++)
      s[i] = ' ';      //Fill s[] with spaces
   i = 0;
   do {       //generate digits in reverse order
        s[i++] = n % 10 + '0';
        n/=10;
    }
    while (n>0);
    s[5]= '\0';//add string terminator
   reverse(s);

}

bool SameString(uint8_t *s1, uint8_t *s2)
//Returns true if the first 4 characters in string s1 are the same as in s2
{
    uint8_t i=0;

    while ((s1[i] == s2[i]) && (++i < 4)); //loop until either the chars differ, or we get to 4th char

    if (i==4) return true; //All 4 chars were identical, return true.
    return false;   //Not all 4 were the same.
}


void CopyString(uint8_t *s1, uint8_t *FaultString)
//Pushes the first 4 characters in string s1 into the front of FaultString, with a space char.
//FaultString chars are pushed along 5 places, the last 5 are discarded.
{
    uint8_t i = 32;  //Array index - starts at end of FaultString (before string terminator at 33)
    uint8_t offset = 5; //How far back is the previous number from the current number
                        //(changes because of the carriage return etc. in the string)

    while (i > 11)
    {//Starting at the end of the second line, move all chars along.
     //stop when we get to 11 (the final digit of the first fault code).
        FaultString[i] = FaultString[i - offset];
        i--;

        switch (i){
            case 23 :
                i = 22; //There is a space at 23 which we don't want to touch.
                offset = 7;     //previous fault code is now 7 chars back.
            break;

            case 18:
                i = 15;
                offset = 5;
            break;
        }
    }
    i--;
    do
    {//Copy the 4 chars from s1 into FaultString.
        FaultString[i] = s1[i-7]; //The first digit of the first fault code in FaultString is at 7.
    }while (i-- -7); //finish once the zeroth char has been copied over from s1.

}

//Reverses 5 char string in place
void reverse(uint8_t *s){
    uint8_t c, i, j;

    for (i=0,j=4;i<j; i++, j--)
   {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }

}


void Fmt1Point(uint8_t *s){
	uint8_t i;
		for(i=0;i<3;i++)
			s[i]=s[i+1];
		s[3]='.';
}

void Fmt2Point(uint8_t *s){

		s[4]=s[3];
		s[3]=s[2];
		s[2]='.';

}
void DisplayIt(uint8_t *s){
    uint8_t i;
    for(i=0;i<5;i++){
        if(s[i]!= ' ') //don't display leading blanks
            lcdPutC(s[i]);
    }
}
int16_t Convert_Volts(uint8_t Array_Val){
	int16_t Volt_Val;
	Volt_Val=1263405/(316+Array_Val);
	return(Volt_Val);
}

uint8_t Convert_Temp(uint8_t Array_Val){
	int16_t Temp_Val, K;
	if(Array_Val>103) K=26;	//these values for AVX thermistor
	else if(Array_Val>92) K=25;
	else if(Array_Val>81) K=24;
	else K=22;
	Temp_Val=(((100*(128-(signed int16_t)Array_Val))/K)+250)/10;
	if (Temp_Val<0) Temp_Val = 0;
	return((uint8_t)Temp_Val);
}


uint8_t DoBalance(bool Balance){
//If Balance =true, turn  on those shunts with voltages
//above the balance value ie Volts[i]<BalVal
//If Balance=false then turn off all shunts and return 0.
//Otherwise returns the number of cells shunting.

    uint8_t CRC=2;  //This is the checksum. Always starts out as 2 for the 1st command.
    uint8_t i=1;
    uint8_t CellsCounter = 0;
    while(Volts[i]!=0xFF)i++;//find end of Volts[],FF is terminator
                             //NB addressing is reversed when master sending
    putch1(02);	//02 is bal cmd, send it out to the modules

    if (Balance && (Volts[0]==01) && (i<CELLMAX))
    //In here if 1. Balance is true and 2. a couple of simple checks on the Volts array pass
    {//Send out commands to each of the modules telling them either to shunt (1) or not (2)
        while(i>1){
            if(Volts[i-1]>BalVal)	//i-1 is datum. Raw value, so higher number = less volts
            {
                putch1(02); //turn shunt off
                CRC = CRC + 2;
            }
            else
            {
                putch1(1); //turn shunt on
                CellsCounter++; //count how many are on
                CRC = CRC + 1;
            }
            i--;
        }
    }
    else
    {//We don't want any of the modules to shunt, so send 2 to all of them.
        CellsCounter = No_Of_Cells; //No_Of_Cells is 48 for 48 cells
        do
        {
            putch1(02);
            CRC = CRC + 2;
            CellsCounter--;
        } while(CellsCounter>0);
    }
    if ((CRC == 0xff) || (CRC == 0x00)) CRC = 0xfe; //0xff or 0x00 not allowed in data stream
    putch1(CRC);
    putch1(0xFF);//add terminator
    return(CellsCounter);
}

void Charger(uint24_t PackVolts, uint8_t *FaultCode, uint8_t MaxCellVoltage, uint8_t NoOfShunts, uint8_t *ChargerState)
//Puts the charger into one of three states: Enabled, Disabled or current Limited.
//Checks Switch 2. If pressed in for more that 10 calls of this function, charger is disabled
//for a short time.  LED is ON if charger is disabled - this overrides the LED value from 
//the FaultCheck function, which is called before this one.  LED is not changed 
//unless charger is disabled.
//The charger current is limited once the MaxCellVoltage exceeds SlowCharger
//Once all modules are balancing, the charger is turned off.  It stays off until MaxCellVoltage drops
//to SlowCharger - SlowBand.
//ChargerState is on of:
//0 = OK to charge
//1 = Charger limited
//2 = Charger disabled
{
    static uint8_t OverRide;    //Used to manually override the charger (turns off for a little while)

    if ((*ChargerState == 0) && (Convert_Volts(MaxCellVoltage) > SlowCharger))
    { //Voltage nearing turn-off value, limit the charger current
        *ChargerState = 1;
    }

    if (*ChargerState != 0)
    {//If the charger is current limited or off(could have happened this time or previously)
        if ((Convert_Volts(MaxCellVoltage) < (SlowCharger - SlowBand)))
        {//Voltage has dropped down, so re-enable full charger current 
            *ChargerState = 0;
        }
    }

    if (PackVolts/10 > MaxPackVolts) *FaultCode = 4;  //Pack Voltage too high - fault condition
    if (*FaultCode > 3) 
    {//Any fault code 4 or greater requires turning off the charger.
        *ChargerState = 2;
    }

    if (OverRide)
    {//The code here either turns the charger off or leaves it unchanged.
        if (OverRide < 10)
        {//The switch was pressed in last time - is it still?
            if (!SW_2) OverRide++; //If button is pressed now, increment (charger is still on)
            else OverRide = 0;      //Button has to be held in for 10 iterations, or we revert.
        }
        else
        {//We are in our countdown. Turn the charger off and LED on until OverRide goes past 255 to 0.
            OverRide++;//Increments once every time this function is called until it flips over past 255 to 0, gives a delay.
            *ChargerState = 2;  //Turn charger off
            LED = 1; //LED on
            OUTPORTS    //Write the LED value to the output port
        }
    }
    else if (!SW_2) OverRide++; //Switch is on, but wasn't last time.

    if (NoOfShunts >= No_Of_Cells)
    {//All modules are shunting, turn off the charger
        *ChargerState = 2; 
    }

    switch (*ChargerState)
    {//Write the PWM value to the register that controls the PWM output
        case 0:
            CCPR1L = ChargerOn;
        break;
        case 1:
            CCPR1L = ChargerLimit;
        break;
        case 2:
            CCPR1L = ChargerOff;
        break;
        default:
            CCPR1L = ChargerOn;
    }
}

void FaultCheck (uint8_t *V_Max, uint8_t *V_Min, uint8_t *T_Min, uint8_t *FaultCode)
//Checks for several faults, sets FaultCode and turns on LED if required.
//Note that V_Max is the lowest value in the array, V_Min the highest.
{
    static uint8_t LastFault;

    LED = 0;                      //LED disabled by default

    if (!SW_2) {*FaultCode = 0;}  //Reset fault if switch SW_2 is pushed

    if (timeout)
    { //Comms error - we haven't received something for a while
        *FaultCode = 1;
    }

    if ((*FaultCode != 1) && (*FaultCode != 2))
    { //If there is no Comms fault (1 or 2), then test for other faults:
        if (Convert_Volts(*V_Max) < MinCellVolts)
        {//A cell voltage is too low
            *FaultCode = 3;
        }
        if (Convert_Volts(*V_Min) > MaxCellVolts)
        {//A cell voltage is too high
            *FaultCode = 5;
        }
        if (Convert_Temp(*T_Min) > MaxCellTemp)
        {//A cell temperature is too high
            *FaultCode = 6;
        }
    }
    if (*FaultCode > 0) {LED = 1;}  //Turn LED on for fault codes 1 and above.
    OUTPORTS
}

/******************************************************************************/
/* Main Program                                                               */
/******************************************************************************/


void main(void)
{

    static uint8_t Startup = 50;   //This is decremented to zero. So if it is above zero, we are starting up.
    static uint8_t LCDClearCounter;//When this gets to 0 we clear the LCD
    static uint8_t CellsOn = 0;  //The number of cells currently balancing (shunt on).
    static uint8_t V_Max;
    static uint8_t  V_Min, T_Min, T_Max, HiVCell, LoVCell, HiTCell, LoTCell; //Cell volts, temps
    static uint8_t ChargeState=0;  //0=not charging, 1= charger limited, 2= charger disabled
    static int16_t v;
    static uint8_t LCDBuf[6];  //String for the LCD to display
    static uint8_t FaultString[34] = "Faults          \n                \0"; //String for the LCD to display. Ends with 0x00.
    static uint8_t DisplayMode = 1;  //0 = temps, 1 = Volts, 2 = balancing, 3 = faults, 4 = version, 9 = comms error
    static int24_t NowBattVolts, AvBattVolts;  //Voltage (in mV) of entire battery pack, instant and rolling average
    static uint8_t TXVal = 0;  //What do we request next? 0-9 = Volts, 10 = Temps

    /* Configure the oscillator for the device */
    ConfigureOscillator();

    /* Initialize I/O and Peripherals for application */
    InitApp();
    initLCD();
    lcdPutC ('\f'); //'\f' clears the screen and homes the cursor
    
    /* Initialise system variables*/
    FaultCode = 0;
    Volts[1]=0xFF;
    Temps[1]=0xFF;

  
    while(1)
    {
        CLRWDT();   //Clear watchdog timer.

        if (Startup)
        {   //Reset all faults when we first start up. Once Startup = 0, it stays 0.
            FaultCode = 0;
            Startup--;
        }

        if (!SW_1)
        {   //Switch 1 rotates through display modes.
            DisplayMode++;
            if (DisplayMode >= 5) DisplayMode = 0;
            initLCD(); //Initialise the LCD display when the switch is pressed (allows manual resetting of LCD).
        }

       TXVal++; //TXVal cycles from 1 to 9. 1-5 = volts, 6 = temps, 7 = balance, 8-9 = nothing (to provide a delay)
       switch (TXVal)
        {
            case 6:
                tx_Temps(); //Request Temperature data from the modules
                break;
            case 7: //If we need to balance, send that command to the modules
                CellsOn = DoBalance((FaultCode == 0) || (FaultCode == 4) || (FaultCode == 5));
                                    //If FaultCode == something else then don't balance.
                break;
           case 8:  //Transmit to PC. Normally Volts, but Temps if button is being pressed 
               if (!SW_2) tx_streamPC(Temps); else tx_streamPC(Volts);
               break;
           case 9:  
                TXVal = 0; //Gets incremented to 1 before the next switch
                break;
           default:
                tx_Volts(); //Request Voltage data from the modules
                break;
        }
        __delay_ms(300);//Delay to allow propogation time between modules

        //Find the min, max values from the arrays.
        NowBattVolts = HiLow(Volts, Temps, &V_Max, &V_Min, &HiVCell, &LoVCell, &T_Max, &T_Min, &HiTCell, &LoTCell);
        AvBattVolts = ( ((AvBattVolts*3) + NowBattVolts) / 4); //rolling average calculation

        if (!LCDClearCounter++) initLCD();//Initialises the LCD.  We need to do this occasionally because
          //the LCD periodically gets confused and displays garbage.  The counter reduces the frequency of the init.

        lcdPrint("\f");//Clear display ready for writing.

        if ((DisplayMode == 1) || (DisplayMode == 2))
        {//Display the first line, which is common to display modes 1 & 2)
            v = Convert_Volts(V_Min);   //Convert the raw value into a Voltage
            MyItoA(v,LCDBuf);   //Convert the Voltage to a string in LCDBuf
            Fmt2Point(LCDBuf);  //format it with 2 decimal places-round down
            lcdPrint("h ");     //"h" is for high voltage
            DisplayIt(LCDBuf);  //Display the Voltage string LCDBuf

            v = Convert_Volts(V_Max);
            MyItoA(v,LCDBuf);
            Fmt2Point(LCDBuf);
            lcdPrint(" l ");
            DisplayIt(LCDBuf);
            if (FaultCode)
            {   //Display "FX" if there is a fault
                lcdPrint(" F");
                MyItoA(FaultCode,LCDBuf);
                DisplayIt(LCDBuf);
            }
            lcdPrint("\n");
        }
        if ((FaultCode == 2) && !(FaultString[7] == '2') ) //A new fault 2 changes mode. Old ones don't force the issue
            //A module had an input comms error, but could still send
            DisplayMode = 9;    //This mode displays which module had the comms error

        // <editor-fold defaultstate="collapsed" desc="Writes to the display depending on DisplayMode">
        switch (DisplayMode) {
            case 0: //Temp Display Mode
                lcdPrint("Temp hi ");
                v = Convert_Temp(T_Min); //T_Min holds the min value, the max temp.
                MyItoA(v, LCDBuf);
                DisplayIt(LCDBuf);
                lcdPrint(" lo ");
                v = Convert_Temp(T_Max);
                MyItoA(v, LCDBuf);
                DisplayIt(LCDBuf);
                lcdPrint("\n");
                lcdPrint("Cell hi ");
                MyItoA(LoTCell, LCDBuf);
                DisplayIt(LCDBuf);
                lcdPrint(" lo ");
                MyItoA(HiTCell, LCDBuf);
                DisplayIt(LCDBuf);

                break;
            case 1: //Volts Display Mode
                MyItoA((AvBattVolts / 100), LCDBuf);
                Fmt1Point(LCDBuf); //format it with 1 decimal place-round down
                lcdPrint("Vt ");
                DisplayIt(LCDBuf);
                MyItoA(LoVCell, LCDBuf);
                lcdPrint(" h");
                DisplayIt(LCDBuf);
                MyItoA(HiVCell, LCDBuf);
                lcdPrint(" l");
                DisplayIt(LCDBuf);
                break;
            case 2: //Balancing Display Mode
                lcdPrint("Bal: ");
                MyItoA(CellsOn, LCDBuf); //Numbers of cells currently balancing
                DisplayIt(LCDBuf);
                lcdPrint(" Chgr: ");
                MyItoA(ChargeState, LCDBuf); //Numbers of cells currently balancing
                DisplayIt(LCDBuf);
                break;
            case 3: //Faults Display Mode
                lcdPrint(FaultString);
                break;
            case 4: //Version Display Mode
                lcdPrint(VERSION1);
                lcdPrint(VERSION2);
                break;
            case 9: //Module Comms Error Mode
                lcdPrint("Comms Error   ");
                if (FaultCode) { //Display "FX" if there is a fault
                    lcdPrint("F");
                    MyItoA(FaultCode, LCDBuf);
                    DisplayIt(LCDBuf);
                } else DisplayMode = 3; //If we were in this mode, but now the fault is cleared, change mode.

                lcdPrint("\n");
                lcdPrint("Module: ");
                MyItoA(Temps[1], LCDBuf); //This is the module with the comms error
                DisplayIt(LCDBuf);
                break;
                //Removed to make code fit in micro.           default:
                //               lcdPrint("\n");
                //               lcdPrint("Unhandled Error");
                //           break;
        }// </editor-fold>

         
        FaultCheck (&V_Max, &V_Min, &T_Min, &FaultCode); //Sets fault codes and lights LED on fault
        Charger(AvBattVolts, &FaultCode, V_Min, CellsOn, &ChargeState); //Enable or disable charger

 // <editor-fold defaultstate="collapsed" desc="Makes up a short fault string for the current fault (if any)">
        //The next piece of the code makes up a short fault string for the current fault (if any)
        //Puts the string into LCDBuf
        uint8_t Details = 0; //A 2 digit code containing details about the current fault
        //For faults: 0,1,4 = 00; 2 = cell with comms error; 3 = low voltage cell No.
        //5 = hi voltage cell number, 6 = hi temp cell number

        LCDBuf[0] = '0' + FaultCode; //Puts the first char in LCDBuf = single digit fault code
        LCDBuf[1] = '_'; //Second char is always '_'
        LCDBuf[4] = '\0'; //Null terminator because fault string always 4 chars long
        switch (FaultCode) {

            case 2:
                Details = Temps[1]; //Temps[1] contains the module with the comms error
                break;
            case 3: //Cell under voltage
                if (Convert_Volts(Volts[HiVCell]) < MinCellVolts) //Double check to see that this cell is under voltage
                {
                    Details = HiVCell; //HiVCell is the cell with the lowest voltage
                }
                break;
            case 5:
                if (Convert_Volts(Volts[LoVCell]) > MaxCellVolts)//Double check to see that this cell is over voltage
                {
                    Details = LoVCell; //LoVCell is the cell with the highest voltage
                }
                break;
            case 6:
                if (Convert_Temp(Temps[LoTCell]) > MaxCellTemp) {
                    Details = LoTCell; //LoTCell is the cell with the highest temp
                }
                break;
        }
        //So far we have LCDBuf = "X_  /0". We need to fill in the Details:
        LCDBuf[2] = '0' + (Details / 10); //Upper digit of the 2 digit Details
        LCDBuf[3] = '0' + (Details % 10); //Lower digit

        // </editor-fold>

        if (!SameString(LCDBuf, (FaultString + 7))) //Most recent fault stored at FaultString+7
        {//If this fault string is different than the most recent fault stored in FaultString:
            CopyString(LCDBuf, FaultString); //Push the current fault string into FaultString.
        }

    }
 

}


