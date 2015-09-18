/*
 	Names:		Sam Abbate, Joey Weate, Tess Thoresen
	ID:			sja4cm, jfw6fp, tat3fq
	Lab #:		Lab12
	Title:		Motor Controller

	Description:
*/

#include <msp430.h> 
#include "useful.h"
//----------------ROTARY ENCODER LAB -----------------------

//---------------DEFINE STATEMENTS--------------------------
#define STBY BIT1
#define PWMA BIT2
#define AIN1 BIT3
#define AIN2 BIT4
#define LED BIT6
#define Num	10
#define red BIT0

int cwCount;
int ccwCount;

int cwDesired;
int ccwDesired;

#define SWITCHA_HIGH    ((P2IN & 1<<4) == 0) // Logic to to determine if Channel A is high
#define SWITCHB_HIGH    ((P2IN & 1<<3) == 0) // Logic to determine if Channel B is high

#define Red BIT0 // define Red LED as BIT0
#define Green BIT6 // define Green LED as BIT6

//----------TYPEDEF INSTRUCTIONS--------------------

typedef enum {
    AlowBlow, AhighBlow, AlowBhigh, AhighBhigh, Initialize		// Define the states the RotaryEncoder can have
} EncoderState;

typedef enum DbStatesPossible {
    DbExpectHigh, DbValidateHigh, DbExpectLow, DbValidateLow	// Define possible debouncer states
} DbState;
typedef enum SwitchesAvailable {								// Define the possible switches in the system, in this case there are 3 switches
    Switch1, Switch2
} Switches;
typedef enum SwitchValues {										// Define the Status of these switches-either on or off
    Off, On
} SwitchStatus;

typedef struct { 												// Define what each Switch has that makes it unique
    EncoderState CurrentEncoderState;
    Switches SwitchSelect;
    DbState ControlState;
    SwitchStatus CurrentSwitchReading;
    SwitchStatus CurrentValidState;
    SwitchStatus PreviousValidState;
    unsigned int prev;											// Variable used to measure changes in time between debouncing
    unsigned int MAKEDEBOUNCETIME;
    unsigned int RELEASEBOUNCETIME;
} SwitchDefine;

typedef struct { 												// Define the specific switches that the encoder uses
    SwitchDefine SwitchA;										// Dial rotation switch
    SwitchDefine SwitchB;										// Dial rotation switch
} EncoderDefinitions;

//-----------GLOBAL VARIABLES------------------------

int count = 0;
int tempCount = 0;
unsigned int j;

EncoderDefinitions rotaryEncoder;								//instances of the buttons and states below
EncoderState gEncoderState;

unsigned int g1mSTimeout; 										//This variable is incremented by the interrupt handler and decremented by a software call in the main loop.
long counter = 0;
unsigned int lsb = 0;
unsigned int msb = 0;
int cwRampUp=1;
int cwFull=0;
int cwRampDown=0;
int ccwRampUp=0;
int ccwFull=0;
int ccwRampDown=0;
int ourCount=0;
int ledCounter;

//-------------FUNCTION PROTOTYPES-------------------
void InitializeVariables(void);
void InitPorts(); 												// Initializes all Port related variables
void InitTimerSystem();
void led();
//This should set up a periodic interrupt at a 1 milliecond rate using SMCLK as the clock source.
EncoderState stateMachine(EncoderDefinitions *myRotorEncoder,
        EncoderState myEncoderState); 							// goes through the stateMachine of the rotary Encoder
DbState Debouncer(SwitchDefine *Switch); 						// Debounces the signals
SwitchStatus GetSwitch(Switches Select); 						// Sends in one of the three switches and returns if that signal of that switch is high or low
void ManageSoftwareTimers(void); 								// Manage the software timer interrupt system
void stop();
void CW();
void CCW();
void checkRotations();

#pragma vector = TIMER0_A0_VECTOR 								// Associate timer interrupt routine with a specific vector
__interrupt void TimerA0_routine(void) 							// Timer interrupt routine that increases global timer counter
{
	g1mSTimeout = 1;


if (cwRampUp){
	CW();
	//P1OUT |= red;
	TACCR1 = TACCR1 + 1;  //changes the current value at CCR1 by either +1 based on the value of inc (described above).

	if(TACCR1>TACCR0){
		cwRampUp=0;
		cwFull=1;
	}
}

if (cwRampDown){
	CW();
	//P1OUT |= red;
	TACCR1 = TACCR1 - 1;  //changes the current value at CCR1 by either +1 based on the value of inc (described above).

	if(TACCR1<1){
		cwRampDown=0;
		stop();
		//delay();
		ccwRampUp=1;
		_delay_cycles(16000000);
	}
}

if (ccwRampUp){
	CCW();
	//P1OUT |= red;
	TACCR1 = TACCR1 + 1;  //changes the current value at CCR1 by either +1 based on the value of inc (described above).

	if(TACCR1>TACCR0){
		ccwRampUp=0;
		ccwFull=1;
	}
}

if (ccwRampDown){
	CCW();
	//P1OUT |= red;
	TACCR1 = TACCR1 - 1;  //changes the current value at CCR1 by either +1 based on the value of inc (described above).

	if(TACCR1<1){
		ccwRampDown=0;
		checkRotations();
		stop();
		//delay();
		_delay_cycles(16000000);
		cwRampUp=1;
	}
}



	TA0CCTL0 &= ~CCIFG;			//clears the routines flags
    											// Increase global counter for managesoftwaretimers() check
}

#pragma vector = TIMER0_A1_VECTOR 								// Associate timer interrupt routine with a specific vector
__interrupt void TimerA1_routine(void) 							//Interrupt routine for button
{
    TACCTL1 &= ~(CCIFG);// reset flags
   // P1OUT &= ~red;
	if(cwRampUp){
		stop();
	}
	if(cwRampDown){
		stop();
	}
	if(ccwRampUp){
		stop();
	}
	if(ccwRampDown){
		stop();
	}


}

void ManageSoftwareTimers(void) {								// This method checks the value of the global 1ms counter
    if (g1mSTimeout == 1) {										// If the interrupt has been executed
        g1mSTimeout = 0;										// Reset the interrupt counter variable
        counter++;												// And increase the global counter variable used for debouncing.

    }

}



void checkRotations(){
ourCount=Num*48;

	if(cwCount>ourCount){

		cwDesired-= ((cwCount-ourCount));
	}
	else if(cwCount<ourCount){

		cwDesired += ((ourCount-cwCount));
	}
	if(ccwCount>ourCount){

		ccwDesired-= ((ccwCount-ourCount));
	}
	else if(ccwCount<ourCount){

		ccwDesired+=((ourCount-ccwCount));
	}


	ccwCount=0;
	cwCount=0;

}
//---initializing buttons and rotary
void InitializeVariables(void) {

    rotaryEncoder.SwitchA.ControlState = DbExpectHigh;			   		//Set up SwitchA(clockwise turns) defaults. Start at DbExpectHigh
    rotaryEncoder.SwitchA.CurrentSwitchReading = GetSwitch(Switch1);	//Set current switch reading
    rotaryEncoder.SwitchA.CurrentValidState = Off;						//Set current valid state to off
    rotaryEncoder.SwitchA.MAKEDEBOUNCETIME = 5;							//Debounce time set to 5ms both ways (rise and fall).
    rotaryEncoder.SwitchA.RELEASEBOUNCETIME = 5;
    rotaryEncoder.SwitchA.SwitchSelect = Switch1;						//Set switch 1 as available.


    rotaryEncoder.SwitchB.ControlState = DbExpectHigh;   				//Set up SwitchB (counter-clockswise turns) defaults. Start at DbExpectHigh
    rotaryEncoder.SwitchB.CurrentSwitchReading = GetSwitch(Switch2);	//Set current switch reading
    rotaryEncoder.SwitchB.CurrentValidState = Off;						//Set current valid state off
    rotaryEncoder.SwitchB.MAKEDEBOUNCETIME = 5;							//Debounce time set to 5ms both ways.
    rotaryEncoder.SwitchB.RELEASEBOUNCETIME = 5;
    rotaryEncoder.SwitchB.SwitchSelect = Switch2;						//Set switch 2 as available.


    cwDesired=Num*48;
    ccwDesired=Num*48;
}

//-----------------------------------------

void InitPorts() {

	P1DIR |= STBY;
	P1DIR |= PWMA;
	P1DIR |= AIN1;
	P1DIR |= AIN2;
	P1DIR |= LED;
	//P1DIR |= red;


}
//-----------------------------------------

void InitTimerSystem() {
    TACCR0 = 3500; 							//set frequency, so that an interrupt occurs about every 1ms.
    TACCR1 = 0;
    TACCTL0 = CCIE | CM_1; 					//capture/compare interrupt enable; CM1 means capture. This enables the timer.
    TACCTL1 = CCIE | CM_1; 					//capture/compare interrupt enable; CM1 means capture. This enables the timer.
    TACTL = TASSEL_2 | ID_0 | TACLR | MC_1; //TASSEL_2 sets timer SMCLK source select to ACLK/1, specified as "/1" by ID_0. MC_1 runs it in count up
                                            //mode, and TACLR clears the timer to start.
    DCOCTL = CALDCO_16MHZ;                   //set the clock frequency to 1MHz
    BCSCTL1 = CALBC1_16MHZ;
}
//-----------------------------------------

SwitchStatus GetSwitch(Switches Select) {
    SwitchStatus ReturnValue = Off;
    if (Select == Switch1) { 				//If the Switch is Switch1, then determine if it is being pressed
        if (SWITCHA_HIGH)
            ReturnValue = On;
    }

    else if (Select == Switch2) { 			//If the Switch is Switch2, then determine if it is being pressed
        if (SWITCHB_HIGH)
            ReturnValue = On;
    }

    return (ReturnValue); 					//Returns if the respective switch is being pressed
}

DbState Debouncer(SwitchDefine *Switch) {	//Debouncer method moves states according to the state machine provided in the notes.

    DbState MyState = Switch->ControlState;							//Save current control state
    unsigned int time = Switch->prev;								//Save last recorded time to calculate delay
    Switch->CurrentSwitchReading = GetSwitch(Switch->SwitchSelect);	//Set current switch reading to the switch select

    switch (MyState) {												//Initialize switch cases
    case DbExpectHigh:												//If MyState is DbExpectHigh:
        if (!(Switch->CurrentSwitchReading == On)) {				//And if the current reading is not "on"..
            Switch->ControlState = DbValidateHigh;					//Set the co00ntrol state to DbValidateHigh
        } else {
            time = counter;											//else update with new time
        }
        break;
    case DbValidateHigh:											//If MyState is DbValidateHigh
        if (Switch->CurrentSwitchReading == On) {					//And if the current switch reading is "on"...
            time = counter;											//Update recorded time
            Switch->ControlState = DbExpectHigh;					//Move to next state
        } else if ((counter - time) >= Switch->RELEASEBOUNCETIME) {	//Else if the different in times is greate than the release time
            Switch->PreviousValidState = On;						//Previous valud state toggled on
            Switch->CurrentValidState = Off;						//Current valid state toggled on
            Switch->ControlState = DbExpectLow;						//Move control state to next state of machine.
            time = counter;											//And update time
        }
        break;
    case DbExpectLow:												//If MyState is DbExpectLow
        if (Switch->CurrentSwitchReading == On) {					//Set current switch reading on
            Switch->ControlState = DbValidateLow;					//Move along state machine
        } else {
            time = counter;											//If not, update time
        }

        break;
    case DbValidateLow:												//If MyState is DbValidateLow
        if (!(Switch->CurrentSwitchReading == On)) {				//And if current switch reading is not "on"
            time = counter;											//Update time
            Switch->ControlState = DbExpectLow;						//Move along state machine
        } else if ((counter - time) >= Switch->MAKEDEBOUNCETIME) {	//If not, check if time difference is greater than makedebounce time. If it is:
            Switch->PreviousValidState = Off;						//Previous valid state off
            Switch->CurrentValidState = On;							//Current valid state on
            Switch->ControlState = DbExpectHigh;					//Move along state machine
            time = counter;											//Update time
        }

        break;
    default:
        MyState = DbExpectHigh;
    }
    MyState = Switch->ControlState;									//Update MyState with the current control state and return it for debugging
    return MyState;
}

EncoderState stateMachine(EncoderDefinitions *myRotorEncoder,			// after debounce
        EncoderState myEncoderState) {

    // The following switch statement describes the state machine for the rotary encoder as shown in the accompanying report
    switch (myEncoderState) {
    case Initialize:
        lsb = 0;
        msb = 0;
        count = 0; //count goes back to 0

        		//The following lines get the current SwitchA and SwitchB values, and sets myEncoderState accordingly
        if (myRotorEncoder->SwitchA.CurrentValidState == Off\
                && myRotorEncoder->SwitchB.CurrentValidState == Off) {
            myEncoderState = AlowBlow;
        }
        if (myRotorEncoder->SwitchA.CurrentValidState == On
                && myRotorEncoder->SwitchB.CurrentValidState == Off) {
            myEncoderState = AhighBlow;
        }
        if (myRotorEncoder->SwitchA.CurrentValidState == Off
                && myRotorEncoder->SwitchB.CurrentValidState == On) {
            myEncoderState = AlowBhigh;
        }
        if (myRotorEncoder->SwitchA.CurrentValidState == On
                && myRotorEncoder->SwitchB.CurrentValidState == On) {
            myEncoderState = AhighBhigh;
        }
        break;

        	//If A and B were low, update values with what they are now and increase count. If they havent changed, reinitialize.
    case AlowBlow:
        if (myRotorEncoder->SwitchA.CurrentValidState == On) {
            myEncoderState = AhighBlow;
            cwCount++;
            led();
        }
        if (myRotorEncoder->SwitchB.CurrentValidState == On) {
            myEncoderState = AlowBhigh;
            ccwCount++;
            led();
        }

        break;
        		//If A low and B high, update values with what they are now and increase count. If they havent changed, reinitialize.
    case AlowBhigh:
        if (myRotorEncoder->SwitchA.CurrentValidState == On) {
            myEncoderState = AhighBhigh;
            ccwCount++;
            led();
        }
        if (myRotorEncoder->SwitchB.CurrentValidState == Off) {
            myEncoderState = AlowBlow;
            cwCount++;
            led();
        }


        break;
        		//If A high and B low, update values with what they are now and increase count. If they havent changed, reinitialize.
    case AhighBlow:
        if (myRotorEncoder->SwitchA.CurrentValidState == Off) {
            myEncoderState = AlowBlow;
            ccwCount++;
            led();
        }
        if (myRotorEncoder->SwitchB.CurrentValidState == On) {
            myEncoderState = AhighBhigh;
            cwCount++;
            led();
        }

        break;
        		//If A and B were high update values with what they are now and increase count. If they havent changed, reinitialize.
    case AhighBhigh:
        if (myRotorEncoder->SwitchA.CurrentValidState == Off) {
            myEncoderState = AlowBhigh;
            cwCount++;
            led();
        }
        if (myRotorEncoder->SwitchB.CurrentValidState == Off) {
            myEncoderState = AhighBlow;
            ccwCount++;
            led();
        }

        break;

    default:							//Do this if none of the cases activate.
        myEncoderState = Initialize;	//Set state to initialize
        cwCount=0;					//Reset count
        ccwCount=0;


    }
    gEncoderState = myEncoderState;

    return myEncoderState;
}


void CCW(){
	P1OUT |= STBY;
	P1OUT |= PWMA;
	P1OUT &= ~AIN1;
	P1OUT |= AIN2;
}

void CW(){
	P1OUT |= STBY;
	P1OUT |= PWMA;
	P1OUT |= AIN1;
	P1OUT &= ~AIN2;
}
void led(){
	ledCounter++;
    if((ledCounter>=48)){
 	   P1OUT |= LED;
 	   _delay_cycles(25);
 	   P1OUT &= ~LED;
 	   ledCounter=0;
    }
}
void stop(){

	P1OUT |= STBY;
	P1OUT |= PWMA;
	P1OUT &= ~AIN1;
	P1OUT &= ~AIN2;

}
int main(void) {
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
    //*************** Initialization Section ***************************
    InitializeVariables();
    InitPorts();
    InitTimerSystem();

    //********************* End of Initialization Section *********************


    _BIS_SR(GIE);  //Enable Global Interrupts Here ONLY

//      static const unsigned char myArray[] ={ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, A, B, LC, D, E, F};

    while (1) {

    	if(cwFull){
    	   CW();
    	   if(cwCount >= cwDesired){
    		   cwFull=0;
    		   cwRampDown=1;
    	   }
       }

       if(ccwFull){
   		  CCW();
    	   if(ccwCount >= ccwDesired){
    		   ccwFull=0;
    		   ccwRampDown=1;
    	   }
       }


    	ManageSoftwareTimers();
        Debouncer(&(rotaryEncoder.SwitchA)); // debounce SwitchA
        Debouncer(&(rotaryEncoder.SwitchB)); // debounce SwitchB
        stateMachine(&rotaryEncoder, gEncoderState); // go through the state machine and change the counter variable respectively


    }
    return 0;
}
