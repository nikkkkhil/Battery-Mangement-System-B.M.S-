/******************************************************************************/
/* User Level #define Macros                                                  */
/******************************************************************************/


/*RS-232 port definitions */
#define INBIT   PORTAbits.RA5	//Port that this processor uses to input RS-232
#define OUTBIT1 PORTAMirror.RA4   //Port that this processor uses to output RS-232

#define COMMS_TIMEOUT 100   //Number of timer interrupts before we declare comms failure

/* Other definitions */
#define SHUNT_DRV PORTAMirror.RA2
#define OUTPORTS PORTAbits = PORTAMirror;

#define CELLMAX 52 //Maximum number of cells + 4. Must be even (is divided by 2)


#include <xc.h>

//GLOBAL VARIABLES
PORTAbits_t PORTAMirror;

/* PORTXMirror are required because although it is possible to set a single bit on a port,
 * it does not remember the previous state of the other bits on the port.  So setting
 * one bit can reset another bit.
 * So we set the bits in PORTXMirror, which should always hold what we want PORTX to output.
 */



/******************************************************************************/
/* User Function Prototypes                                                   */
/******************************************************************************/


void InitApp(void);         /* I/O and Peripheral Initialization */
