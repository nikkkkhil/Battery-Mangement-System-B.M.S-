/******************************************************************************/
/* User Level #define Macros                                                  */
/******************************************************************************/


/*RS-232 port definitions */
#define INBIT   PORTBbits.RB1	//Port that this processor uses to input RS-232
#define OUTBIT1 PORTBMirror.RB2   //Port that this processor uses to output RS-232
#define OUTBIT2 PORTAMirror.RA6	//2nd RS-232 port (to a computer)
//Note that in Neville's version, this bit is also used to reduce charger
//current to 1/2 amp if any modules are shunting.  Could implement this if our
//charger could do this - i.e. can be used for comms or charger control...

/* LCD interface definitions */
#define LCD_DB4 PORTBMirror.RB7
#define LCD_DB5 PORTBMirror.RB6
#define LCD_DB6 PORTBMirror.RB5
#define LCD_DB7 PORTBMirror.RB4

#define LCD_E   PORTAMirror.RA7
#define LCD_RS  PORTAMirror.RA1

/* Other definitions */
#define LED     PORTBMirror.RB3     //Bit to drive the on board LED
#define CHARGER PORTBMirror.RB0     //0 = charger enable, 1 = charger disable
//#define LIMIT   PORTAMirror.RA0     //0 = charger current limited, 1 = unlimited
#define SW_1    PORTAbits.RA2       //Input Switch 1
#define SW_2    PORTAbits.RA3       //Input Switch 2

#define OUTPORTS PORTAbits = PORTAMirror; PORTBbits = PORTBMirror;

#include <xc.h>

//GLOBAL VARIABLES
PORTAbits_t PORTAMirror;
PORTBbits_t PORTBMirror;

/* PORTXMirror are required because although it is possible to set a single bit on a port,
 * it does not remember the previous state of the other bits on the port.  So setting
 * one bit can reset another bit.
 * So we set the bits in PORTXMirror, which should always hold what we want PORTX to output.
 */


/******************************************************************************/
/* User Function Prototypes                                                   */
/******************************************************************************/

void InitApp(void);         /* I/O and Peripheral Initialization */
