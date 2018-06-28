/******************************************************************************/
/* System Level #define Macros                                                */
/******************************************************************************/


/* Microcontroller MIPs (FCY) */
#define SYS_FREQ        16000000L
#define FCY             SYS_FREQ/4
#define _XTAL_FREQ      16000000L     //This is for the __delay macros


/******************************************************************************/
/* System Function Prototypes                                                 */
/******************************************************************************/

/* Custom oscillator configuration funtions, reset source evaluation
functions, and other non-peripheral microcontroller initialization functions
go here. */

void ConfigureOscillator(void); /* Handles clock switching/osc initialization */
void set_timer1(uint16_t val); //sets the timer1 value

