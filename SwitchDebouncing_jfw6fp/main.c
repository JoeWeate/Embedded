/*****************************************************************************
 Add suitable comments to describe this code
 This header should explain what the program does,
 and what resources are required of the cpu and I/O pins.
 *****************************************************************************/

//Standard MSP430 includes
#include <msp430.h>
//Include useful.h for bit operations
#include "useful.h"
//Each program should be configured with each of the following sections
// clearly delineated as shown below.

// ***************** Constant Definitions Section ***************************************
//All constants should be in upper-case letters only.
#define MAKEDEBOUNCETIME 10		//Definitions for debounce Times for the P1.3 button (ms)
#define RELEASEBOUCETIME 23
#define PIN 3					//macros for the pin and port of the 1st button
#define PORT 1

#define PIN2 3					//macros for the pin and port of the 2nd button
#define PORT2 1
// ***************** End of Constant Definitions ****************************************

//****************** Section for Compiler pre-processor defines/Substitutions ***********
//This is an example :
#define buttonBit 	B8_3(P1IN);			//active low
#define LED1_ON   	B8_0(P1OUT) = 1 		//macro to turn on the red LED
#define LED1_OFF 	B8_0(P1OUT) = 0			//macro to turn off the red LED

#define LEDG_ON   	B8_6(P1OUT) = 1 		//macro to turn on the green LED
#define LEDG_OFF 	B8_6(P1OUT) = 0			//macro to turn off the green LED


// You should include similar defines for the other LED and debug pin operations
//******************* End of pre-processor defines section *******************************

//*******************Type Definitions Section*********************************************
typedef enum {
	DbExpectHigh, DbValidateHigh, DbExpectLow, DbValidateLow
} DbState;
typedef enum {
	Off, On
} SwitchStatus;
typedef struct {
	DbState ControlState;
	SwitchStatus CurrentSwitchReading;
	SwitchStatus CurrentValidState;
	SwitchStatus PreviousValidState;
	int prev;
	int pin;			//multiple buttons
	int port;

	//declare what Pin button is going to
	// You will have to insert more definitions in this struct.
	// For example, you should include values for valid on and off times
} SwitchDefine;
//******************End of Type Definitions Section***************************************

//********************** Global Variables Declaration Section ****************************
//The names of all global variables should begin with a lower case g
SwitchDefine gPushButton;    //Structure to contain all info for the P1.3 button
unsigned int g1mSTimeout; //This variable is incremented by the interrupt handler and
// decremented by a software call in the main loop.
unsigned int gButtonPresses=0;  //Number of button presses from all switches
unsigned int counter=64000;	//global counter variable +1 every 1ms
unsigned int greenCount;
unsigned int tenCount;
//********************** End of Global Variables Declaration Section *********************

//********************** Function Prototypes *********************************************
//This function returns the instantaneous value of the selected switch
SwitchStatus GetSwitch(SwitchDefine *Switch);
//This function debounces a switch input
DbState Debouncer(SwitchDefine *Switch);
//Initialize all variables
void InitializeVariables(void);

// Initialize all hardware subsystems
void InitializeHardware(void);
//These are explicit called from InitializeHardware()
void InitTimerSystem(); //This should set up a periodic interrupt at a 1 mS rate using SMCLK as the clock source.
void InitPorts(); //This should also be called from ManageSoftwareTimers every 10 seconds
//This should deal with software initiated timers.
void ManageSoftwareTimers(void);

#pragma vector = TIMER0_A0_VECTOR           // Timer A interrupt service routine

__interrupt  void  TimerA0_routine (void) {

	g1mSTimeout=1;							//timer counter is set to 1 so the counter knows an interrupt occured.
	TA0CCTL1 &= ~CCIFG;						//clears the interrupt flag.


}
//********************** End of Function Prototypes Section ******************************
// ======== main ========
void main(void) {
	WDTCTL = WDTPW + WDTHOLD ;			//watchdog timer
//*************** Initialization Section ***************************
	InitializeVariables();
	InitPorts();
	InitTimerSystem();
// ********************* End of Initialization Section *********************
	//Enable Global Interrupts Here ONLY
	 _BIS_SR(GIE);   						//global interrupt enable after all set-up is complete
	while (1) {								//endless while loop

		ManageSoftwareTimers();
		GetSwitch(&gPushButton);
		Debouncer(&gPushButton);
		//Include code that calls Debouncer()
	}
}
//----------------------------------------------------------------------

void InitPorts(){
		P1REN |= BIT3;    						// Enable pull-up/pull-down resistor
	    P1OUT |= BIT3;   					// Declare resistor pull-up
	    P1DIR |= BIT0 + BIT6;				//declares the outputs of the LED's and button
	    P1DIR &= ~ BIT3;
	    LED1_OFF;						//start with the LED off



}
//This function returns the instantaneous value of the selected switch
SwitchStatus GetSwitch(SwitchDefine *Switch){
if((Switch->port==1)){							//determing the port of the given switch
	if(P1IN & (BIT0<<(Switch->pin))){			//BIT0<<(Switch->pin) returns the Bit at whatever port.pin the switch is at

		(Switch->CurrentSwitchReading) = Off;   //Current value of the switch is off

	}
	else{
		(Switch->CurrentSwitchReading) = On;  //current value of the switch is on

	}
}

else{										//port of the switch is 2

	if(P2IN & (BIT0<<(Switch->pin))){		//BIT0<<(Switch->pin) returns the Bit at whatever port.pin the switch is at

			(Switch->CurrentSwitchReading) = Off;

		}
		else{
			(Switch->CurrentSwitchReading) = On;
		}

}

	return (Switch->CurrentSwitchReading);
}

//----------------------------------------------------------------------
//Function that debounces
DbState Debouncer(SwitchDefine *Switch) {
	int time;
//Code must be added to access the internal variable through the SwitchDefine pointer
	switch (Switch->ControlState) {
		case DbExpectHigh:						//state of expecting a high
			if((Switch->CurrentSwitchReading)==On){
				Switch->prev= counter;			//stores the value for when the signal was first recieved.  if occurs long enough-> valid.
				(Switch->ControlState)= DbValidateHigh;
			}

		break ;
		case DbValidateHigh:					//state to validate a high
			if((Switch->CurrentSwitchReading)==On){		//must still be tring to get into the on position
				time =counter;					//temp store for the counter (value to compare to when the signal was first recieved)
					if((time-(Switch->prev))>= MAKEDEBOUNCETIME){		//if the difference frmo when the signal was first recieved til now is > 10
						P1OUT^= BIT0;			//toggle red LED
						gButtonPresses++;		//itterate the number of button presses
						(Switch->CurrentValidState)==On;		//successfully got into On state
						(Switch->ControlState)= DbExpectLow;	//state is now expecting a low
					}
			}
			else if((time-(Switch->prev) ) < 0 ) {		//controls roll over in the unsigned int of the counter
				int temp= 65535-(Switch->prev);			//gets the difference from the previous time til when the roll over began
				if((temp+time)>=MAKEDEBOUNCETIME){		//compares the signal time (rolled over + starting over) to 10
					P1OUT^= BIT0;						//toggles the red LED
					gButtonPresses++;					//itterate the button pressed counter
					(Switch->CurrentValidState)==On;	//the state is in the ON
					(Switch->ControlState)= DbExpectLow;//now expecting a low state next

				}
			}
			else{
				(Switch->ControlState)= DbExpectHigh;	//did not successfully get a signal (false high)
			}
		break ;
		case DbExpectLow:								//expecting a low signal, currently in high state
			if((Switch->CurrentSwitchReading)==Off){	//button is currrently in the off state
						Switch->prev= counter;			//store the value of time at the moment the button was pressed
						(Switch->ControlState)= DbValidateLow;		//validate the time to see if true low
						}


		break ;
		case DbValidateLow:				//testing for a true low

			if((Switch->CurrentSwitchReading)==Off){		//the button is still in the low state
					time =counter;							//temp store for current time to compare to previous time
								if((time-(Switch->prev))>= RELEASEBOUCETIME){		//if the time the signal has been low is grater than 23ms
									(Switch->CurrentValidState)==Off;				//a valid off state is confirmed
									(Switch->ControlState)= DbExpectHigh;			//an expected high signal is next
								}
						}
						else if((time-(Switch->prev) )<0) {		//controls roll over
							int temp= 65535-(Switch->prev);		//gets the difference from the previous time til when the roll over began
							if((temp+time)>RELEASEBOUCETIME){	//compares the signal time (rolled over + starting over) to 23
								(Switch->CurrentValidState)==Off;	//a valid off state is confirmed
								(Switch->ControlState)= DbExpectHigh;	//an expected high signal is next

							}
						}
						else{
							(Switch->ControlState)= DbExpectLow;	//did not successfull get a signal (false Low)
						}
		break ;
		default: (Switch->ControlState) = DbExpectHigh ;
	}
//The internal state should be updated here. It should also be returned as a debugging aid....
	return (Switch->ControlState) ;
}
//-------------------------------------------
void InitTimerSystem(){    //This should set up a periodic interrupt at a 1 mS rate using SMCLK as the clock source.

	 TACCR0 = 1000;							//set frequency, so that an interrupt occurs about every 1ms.
	 TACCR1 = 0;

	 TACCTL0 = CCIE | CM_1; 					//capture/compare interrupt enable; CM1 means capture. This enables the timer.
	 TACCTL1 = CCIE | CM_1; 					//capture/compare interrupt enable; CM1 means capture. This enables the timer.



	 TACTL = TASSEL_2 | ID_0 | TACLR | MC_1; 	//TASSEL_2 sets timer SMCLK source select to ACLK/1, specified as "/1" by ID_0. MC_1 runs it in count up
	    										//mode, and TACLR clears the timer to start.


    DCOCTL = CALDCO_1MHZ; 						//set the clock frequency to 1MHz
    BCSCTL1 = CALBC1_1MHZ;



}

//-------------------------------------------
void InitializeVariables(void) {
	gButtonPresses = 0;
	gPushButton.ControlState = DbExpectHigh;
	gPushButton.prev=0;
	gPushButton.pin=PIN;
	gPushButton.port=PORT;

}
//--------------
void ManageSoftwareTimers(){

if(g1mSTimeout==1){		//when an interrupt occurs, the value of g1mS is set to 1.
	g1mSTimeout=0;		//the g1mS is set back to 0
	counter++;			//the overall counter is increased by 1
	greenCount++;		//how many interrupts occurs to
	tenCount++;
}
if(greenCount==500){		//500ms occured, so toggle the green LED
	greenCount=0;			//start counting over again til the next half second
	P1OUT^= BIT6;			//toggle the green LED;
}
if(tenCount==10000){		//10 seconds has occured, so initialize the ports again.
	tenCount=0;				//resrtart the count for the next ten seconds
	InitPorts();			//re-initialize the ports
}
}
