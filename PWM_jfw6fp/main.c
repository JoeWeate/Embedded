#include <msp430.h> 

//Joey Weate
//Lab 6: PWM with extra credit
//Collabs: Sam Abbate
//TA AID: in-lab -> helped with debugging strategy (Christina)
//				 -> helped with hinting about clearing flags(Jon)


#define Button BIT3							//create macro for the button
#define Red BIT0							//create macro for the red led
#define Green BIT6							//create macro for the green led
int i=0;  //integer for the for loop in the main method.
int inc = 1;
int on = 1;
//interrupt for CCR0
#pragma vector = TIMER0_A0_VECTOR           // Timer A interrupt service routine

__interrupt  void  TimerA0_routine (void) { // Handle the interrupt. This is the interrupt that is executed when
											//TimerA reaches its max value, since it is configured below in "count-up" mode.

	P1OUT |= Red;
if (on){					//if ON the red LED will change.  IF ON==0, then the LED will be paused.
	if (TACCR1 > TACCR0){	// when the value of CCR1 is incremented past CCR0 ( when 100% duty cycle is reached)
		inc = -1;				// turn the incrementation to decrease the value of CCR1(dim the light)
				P1OUT|=	Green;						//flash Green LED -turn on
				P1OUT &= ~ Green;  					//turns off the Green LED -> post lab.  Want to flash Green LED at change of direction
	}

	if (TACCR1 < 1 ){		//when the value of the CCR1 is back at 0% duty cycle (very dim)
		inc = 1;			//change the incrementation to positive, to brighten the LED

			P1OUT|=	Green;						//flash Green LED -turn on
			P1OUT &= ~ Green; 					 //turns off the Green LED -> post lab.  Want to flash Green LED at change of direction
	}

	TACCR1 = TACCR1 + inc;		//changes the current value at CCR1 by either +/- 1 based on the value of inc (described above).
}
	TA0CCTL0 &= ~CCIFG;			//clears the routines flags
}
//interrupt for CCR1
#pragma vector = TIMER0_A1_VECTOR           // Timer A interrupt service routine

__interrupt  void  TimerA1_routine (void) {

	P1OUT &= ~Red;				//turns off the red LED
	TA0CCTL1 &= ~CCIFG;			//clears the routines flags
}
//interrupt for the button
#pragma vector = PORT1_VECTOR               // Port 1 interrupt service routine ->Post Lab

__interrupt  void  Port1_routine (void) { 	// This method handles the button interrupt.

	P1IFG &= ~BIT3;							//Clears the interrupt flag that is shared among all bits of P1.

	if (on) on= 0;							//toggle whether or not CCR1 can change.
											//On=0 means CCR1 will NOT change. 1 means it WILL change.
	else on =1;
	//TACCTL0 ^= CCIE;						//toggles the capture/command interrupt between enabled and disabled.
											//When the LED is flickering and the button is pressed, this intterupt
											// disables the timer, which consequently stops the flickering of the LED.
											//When pressed again, the timer is enabled again and thus the LED begins flickering again.
}


//the main method
int main(void) {
    WDTCTL = WDTPW | WDTHOLD;				//stop the watchdog timer so the program won't be interrupted if executed for long.
	
    TACCR0 = 500;							//set frequency, so that an interrupt occurs about every 500us.
    TACCR1 = 0;

    TACCTL0 = CCIE | CM_0; 					//capture/compare interrupt enable; CM0 means no capture. This enables the timer.
    TACCTL1 = CCIE | CM_0; 					//capture/compare interrupt enable; CM0 means no capture. This enables the timer.



    TACTL = TASSEL_2 | ID_1 | TACLR | MC_1; //TASSEL_2 sets timer SMCLK source select to ACLK/1, specified as "/1" by ID_0. MC_1 runs it in "count up"
    										//mode, and TACLR clears the timer to start.

    DCOCTL = CALDCO_1MHZ; 					//set the clock frequency to 1MHz
    BCSCTL1 = CALBC1_1MHZ;

    P1DIR |= Green + Red;					//set the green and red LED direction to output without changing the other values in P1DIR.
    P1OUT &= ~Red;							//start with green and red LEDs off.
    P1OUT &= ~Green;
    P1DIR |= BIT4;

      P1REN |= BIT3;    						// Enable pull-up/pull-down resistor
      P1OUT |= BIT3;   						// Declare resistor pull-up
      P1IE |= BIT3;        					// Enable interrupts

    P1SEL |= BIT4;

    _BIS_SR(GIE);   						//global interrupt enable after all set-up is complete


    while(1){								//endless while loop
    }

	return 0;								//arbitrary return for the main method
}
