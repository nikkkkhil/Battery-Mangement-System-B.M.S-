/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/

#if defined(__XC)
    #include <xc.h>         /* XC8 General Include File */
#elif defined(HI_TECH_C)
    #include <htc.h>        /* HiTech General Include File */
#endif

#include <stdint.h>        /* For uint8_t definition */
#include <stdbool.h>       /* For true/false definition */

#include "system.h"        /* System funct/params, like osc/peripheral config */
#include "user.h"          /* User funct/params, such as InitApp */
#include "main.h"
#include "UARTPIC12.h"          //RS-232 functions

/******************************************************************************/
/* User Global Variable Declaration                                           */
/******************************************************************************/


void Timer1Int(uint8_t *IntCount, bool *timeout)
//Called by the interrupt function.  That function clears the interrupt flag after
//calling this function.
//Sets timout to true if IntCount gets high enough.
{
    ++(*IntCount);
    if (*IntCount == (COMMS_TIMEOUT - 10))
    {   //In here if we are about to time out with nothing recieved.
        //Put out a quck data stream so that the next module's IntCount is reset.
        //But this module's IntCount is not reset, so it may eventually timeout.
        putch1(0x05);
        putch1(0x05);
        putch1(0xff);
    }

    if (*IntCount == COMMS_TIMEOUT)
    {
        *timeout = true;
        *IntCount = 0;
    }

}

uint8_t rx_stream(uint8_t *arryptr1, uint8_t *arryptr2, bool *timeout, uint8_t *int_cnt)
//Recieves a complete data stream into the two arrays.
//Returns 0 if it wasn't a valid stream, otherwise the array size (where "3" means 2 elements).
//A valid stream starts with a command (1 - 5), finishes with 0xff, and is
//CELLMAX size or smaller.
//Second last byte is CRC.  Function returns 0 if invalid CRC.
//Each array has a size of CELLMAX/2.
//The 0xff is not stored in the array.  Neither is the CRC.
{
    uint8_t charbuffer;  //Place to store a character temporarily
    uint8_t i;          //counter for arrays
    uint8_t CRC = 0;        //Place for CRC calculation

    *int_cnt = 0;   //Reset the comms watchdog value
    *timeout = false;

    do
    {
        charbuffer = getch();
    }                       //getch() returns 0 if it times out.
    while (!charbuffer && !*timeout);    //keep on looking until we receive something or timeout
    if (*timeout)
        return 0;   //end function if timed out.

    *int_cnt = 0;   //Reset the comms watchdog value

    for (i = 0; ((charbuffer != 0xff) && (i < CELLMAX)); i++)
    {   //keep on receiving until we get 0xff or fill our arrays.
        CRC = CRC + charbuffer; //Add each one to get the CRC value.

        if ((i == 0) && (charbuffer > 5))
        {           //This is the first char in the received string, it should
            return 0;  //be a command (1, 2, 3, 4 or 5). Abort if it is not.
        }
      //this next bit of code puts charbuffer into the array.  cmdarray1 1st,
      //then onto cmdarray2 once the first one is full.
        if (i < (CELLMAX/2))
            *(arryptr1+i) = charbuffer;
        else
            *(arryptr2+(i-(CELLMAX/2))) = charbuffer;

        do
        {
            charbuffer = getch();
        }
        while (!charbuffer && !*timeout);    //keep on looking until we receive something or timeout
        if (*timeout)
            return 0;   //end function if timed out.

        *int_cnt = 0;    //we are receiving, reset comms watchdog timer
    }
    if (charbuffer == 0xff)
    {//We have received a data stream. time to check the CRC:

        //We added the received CRC to our calculated one, so we need to subtract it again:
        CRC = CRC - ((i-1) < (CELLMAX/2) ? *(arryptr1+(i-1)) : *(arryptr2+((i-1)-(CELLMAX/2))));
        if ((CRC == 0xff) || (CRC == 0x00)) CRC = 0xfe; //0xff or 0x00 not allowed in data stream

        if (CRC == ((i-1) < (CELLMAX/2) ? *(arryptr1+(i-1)) : *(arryptr2+((i-1)-(CELLMAX/2)))))
        { //In here if the received and calculated CRCs are the same.
            return i-1;    //A complete, valid data stream, size = i-1 (i was stored, but was the CRC, which we don't keep.
        }
        else return 0; //CRC is different from the received CRC.
    }
    else return 0;   //Something went wrong. Data stream received but not valid.
    
}


void tx_stream(uint8_t *arryptr1, uint8_t *arryptr2, uint8_t *arraysize,
                    uint8_t *TVar, uint8_t *VVar)
//Several functions:
//1. Acts on a shunt command, turning the shunt on or off.
//2. Transmits the stream of characters in the two arrays to the next module.
//3. Inserts Voltage or temperature data into the stream if requested.

//Transmits the array (arraysize bytes of data), starting at arrayptr1+0, and continuing
//on into arrayptr2 if required.
//Calculates a CRC and transmits that after the array
//Puts 0xff at the end of the data stream (after the CRC).
{
    uint8_t charbuffer; //place to store chars from the arrays.
    uint8_t CRC = 0;    //Place to store the CRC error-checking value
    bool shuntflg = false;

    for (uint8_t cntr=0;cntr < *arraysize; cntr++)
    {
            //store the value from the array into charbuffer:
        charbuffer = (cntr < (CELLMAX/2) ? *(arryptr1+cntr) : *(arryptr2+(cntr-(CELLMAX/2))));

        if (cntr == 0)
        {       //The first byte is a command
            putch1(charbuffer); //First send the command on to the next module.
            CRC = CRC + charbuffer; //Add up the CRC for every transmitted byte.

            switch (charbuffer)
                {   //Then act on the command.
                case 01:    //Command to transmit voltage level
                    putch1(*VVar);      //pass our data on.
                    CRC = CRC + *VVar;
                    break;

                case 02:    //Command to turn shunt on or off (next byte is on (1) or off (2))
                    shuntflg = true;
                    break;

                case 03:    //Command to transmit temperature
                    putch1(*TVar);      //pass our data on.
                    CRC = CRC + *TVar;
                    break;
                case 04:    //Comms error. Next byte is how many modules ago.
                    charbuffer = *(arryptr1+1); //Find out which module had the error.
                    putch1(charbuffer+1);       //Increment the module number
                    CRC = CRC + (charbuffer + 1);
                    if ((CRC == 0xff) || (CRC == 0x00)) CRC = 0xfe;//0xff or 0x00 not allowed.
                    putch1(CRC);
                    putch1(0xff);
                    return; //Return; this stream is always 4 chars long.
                case 05:        //Potential comms error - pass it on.
                    putch1(0x05);
                    putch1(0xff);
                    return;
            }
        }
        else
        { //This is not the first character.
            if (shuntflg)
            {   //In here if this is the second byte of a shunt command.
                //In this case we drive the shunt, but don't pass on the command.
                shuntflg = false;
                if (charbuffer == 01)
                {           //01 is on, anything else is off.
                    SHUNT_DRV = true;
                }
                else
                {
                    SHUNT_DRV = false;
                }
            }

            else
            {   //this is not a command, just something to send on to the next module
                putch1(charbuffer);
                CRC = CRC + charbuffer;
            }
        }
    }
    if ((CRC == 0xff) || (CRC == 0x00)) CRC = 0xfe; //0xff or 0x00 not allowed in data stream
    putch1(CRC);    //CRC value for our data stream.
    putch1(0xff);   //Terminator for data stream once array has been sent.

}


void Analogues(uint8_t *Volts, uint8_t *Temp)
//Sets up the ADC, gets values for supply Voltage, board temperature.
//Each time it is called it gets a value for one, then the value for the other
//next time.
//If the converstion is not complete then the function exits without changing anything.
{
    uint16_t scratch;   //place to store part of a calculation
    static bool VoltsorTemp;    //1 for Temp, 0 for volts.

    if (!ADCON0bits.GO_nDONE)        //Check if ADC is complete
    {   //If the ADC is complete, were we looking at Volts or temp?
        if (VoltsorTemp)
        {   //Temperature
            *Temp = (uint8_t)((ADRES & 0x03ff)/4);    //convert ADC result (10 bits)
                                                        //into 8 bits.
            VoltsorTemp = 0;
            ADCON0bits.CHS = 1; //Select volts for the ADC next
            ADCON1bits.ADPREF1 = 0; //Voltage reference = VDD pin.
        }
        else
        {   //Volts
            scratch = (ADRES & 0x03ff); //Voltage result
            if (scratch > 316)
            {   //
                if ((scratch - 316) > 0xfe)
                {   //Not allowed to have 0xff, this is the data stream terminator value.
                    *Volts = 0xfe;
                    SHUNT_DRV = false;  //voltage is too low, don't shunt.
                    OUTPORTS
                }
                else 
                {
                    *Volts = (uint8_t) (scratch - 316);
                }
            }
            else
            {   //Not allowed to have zero values in our data stream (reads as comms fail)
                *Volts = 1; 
            }

            VoltsorTemp = 1;
            ADCON0bits.CHS = 0; //Select temperature for the ADC next.
            ADCON1bits.ADPREF1 = 1; //Voltage reference = VRef pin.
        }
        ADCON0bits.GO_nDONE = 1;    //restart ADC
    }
}



/******************************************************************************/
/* Main Program                                                               */
/******************************************************************************/

void main(void)
{
    uint8_t tempvar;
    uint8_t Voltvar; //places to store temperature and Voltage info

    uint8_t cmdarray1[CELLMAX/2]; //Enough room for one byte per cell. Two arrays
    uint8_t cmdarray2[CELLMAX/2]; //because the compiler won't do one large one
    uint8_t arraycntr = 0;

    /* Configure the oscillator for the device */
    ConfigureOscillator();

    /* Initialize I/O and Peripherals for application */
    InitApp();
    
    while(1)
    {
        CLRWDT();   //Clear watchdog timer

        //Get a stream of data, store it in cmdarray1 & 2:
        arraycntr = rx_stream(cmdarray1, cmdarray2, &timeout, &int_cnt);
        if (!arraycntr && timeout)
        {   //rx_stream returns zero on comms error. 
            //Set up a data stream to say we had a comms error.
            cmdarray1[0] = 04;  //4 = comms error.
            cmdarray1[1] = 00;  //this is the first module with the comms error.
            arraycntr = 3;
        }
           
        if (arraycntr)  
        {//Transmit the data on: (either received data or the comms error data)
            tx_stream(cmdarray1, cmdarray2, &arraycntr, &tempvar, &Voltvar);
            int_cnt = 0;    //resets the comms timer
        }
        if (cmdarray1[0] == 04)
        {//A module has timed out - do not shunt.
            SHUNT_DRV = false;
        }
        
        OUTPORTS //Write the state of the SHUNT_DRV to the port.
        Analogues(&Voltvar, &tempvar);  //Get the analogue values from inputs.
        
        
    }

}


