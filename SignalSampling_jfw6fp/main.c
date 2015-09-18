//
//Name:		Joey Weate (jfw6fp)
//Collabs: 	Sam Abbate (sja4cm)
//Lab4: Signals and Sampling

//	Description: Samples analog input, recording discrete values for analysis.
	//			It makes use of the ADC10 converter of the MSP430 to accomplish this. By changing the
	//			"numberOfSamples" macro and "sampleExponent," macro users can alter the sampling rate.
	//			The sampling takes place in an interrupt timer, currently at 2ms



#include <msp430.h> 
#define Button BIT3							//creates a macro for the button
#define Green BIT6							//creates a macro for the green LED
#define Red BIT0							//creates a macro for the red LED
#define FGEN BIT7;							//creates a macro for the input function generator

#define numberOfSamples 8					//this macro is used to set the sampling rate of the converter. It must be a power of two.
											//It is created as a macro, rather than a global variable, because the global variable samples[]
											//is dependant on it.

#define sampleExponent 3;					//this macro must be the exponent of 2 that matches numberOfSamples. For example,
											//2^sampleExponent=numberOfSamples. We need this macro to find the average. It is used to divide
											//the sum of results without using the division operator.

int samples[numberOfSamples];				//array that holds instantaneous, discrete values of the signal. Implemented as a circular buffer.
int datapoint=0;							//global used to temporarily store single signal values.
int average = 0;							//global value used to display the calculated average of the values in a samples[] array.
int count=0;								//counter used to implement "moving window" index for the samples[] buffer.
											//data point, so that a single array has unique values at each index.
int sum=0;									//stores the sum of all values, so average can be determined.


void filter(){								//in-lab filter method:  determines the average of the values within the sample array (samples[])

	int j = 0;								//Find the average:
	for (j=0; j < numberOfSamples; j++){	//for loop that sums up all the samples in order to find the average
		average = average + samples[j];
	}

	average = average / numberOfSamples;	//The running sum is divided by the number of samples to find the Average.

}


void configureFunctionSensor(void){			//initial set up method for the ADC10 converter
											//in-lab: works for the internal temperature sensor
											//post-lab: outfitted to configure for analog input

	int i;
	ADC10AE0 |= FGEN;						//ADC10 analog is enabled for BIT7 using the macro defined as FGEN (function generator)
	ADC10CTL1 = INCH_7 + ADC10DIV_3;       	//Specifies ADC10 input channel 7 to the MUX, corresponding to BIT7 where the input is coming from.
											//ADC10 clock divider 3 is also set, meaning we divide by 4 as well.

	ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON;	//SREF_1 ->sets the reference voltage; ADC10SHT_3 -> sets the sampling and hold time to 64 clock cycles;
														// REFON -> enables the reference generator; ADC10ON ->turns on ADC and enables the interrupts;

	 __delay_cycles(10);                   	// Delays the cycles to allow the ADC Ref to settle (without the program will not
	 	 	 	 	 	 	 	 	 	 	 //	run properlly for the ADC will not be set up when called on)

	 ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion begin
	__bis_SR_register(GIE);        			// LPM0 with interrupts enabled
	__delay_cycles(10);                   	// allows for the ADC Ref to settle again

	datapoint = ADC10MEM;					//sets the current datapoint to the value currently in memory (ADC10MEM)

	for (i=0; i < numberOfSamples; i++){	//for loop that fills ALL indexes in the samples[] array with the value of the datapoint. All values in the array are now the same.
		samples[i] = datapoint;				//Only used in the initial configuration and is set up outside the interrupt.

	}

}

void filter2(void){							//post-lab filter: Uses a bit shifter to determine the average value.
											//Advantages: Easy to implement circular array/buffer;  Does not use the division operator of loops, saving time;
											//Samples the signal, then fills the samples[] with distinct value at each position.

	P1OUT |= Green;							//Green LED is turned on at the beginning of the method.
											//This LED will be turned off at the end in order to see how long it took the method to run.

	count++;								//increments the index

	sum -= samples[count-1]; 				//subtracts the oldest value from the circular array before it is removed- samples[].
	samples[count-1] = ADC10MEM; 			//the new value overwrites the oldest value.
	sum += samples[count-1]; 				//the new value is added to the overall sum of the array values

	if (count == numberOfSamples){ 			//makes sure that the count never goes beyond the scope of the array.  Once it hits the last spot,
		count = 0;							// the count begins back at the beginning of the array --> creates the circular array
	}

	average = sum; 							//average becomes the value that is the sum
	average >>= sampleExponent; 			//shifts the average (sum) by the sampleExponent macro corresponding to the size of the array (numberOfSamples)
											//A shift of 3 is equivalent to dividing by 8, however time is saved (2^3=8).

	P1OUT &= ~Green;						//Turns the green LED off when the method is over
}


#pragma vector = TIMER0_A0_VECTOR           // Timer A interrupt service routine

__interrupt  void  TimerA0_routine (void) { // Method that handles the interrupt. This is the interrupt that is executed when
											//TimerA reaches its max value, since it is configured in the "count-up" mode.

	P1OUT |= Red;							//Turns on the Red LED so we know when the interrupt method is running.
	filter2();								//call the "moving window" function that samples the signal

	ADC10CTL0 |= ENC + ADC10SC;             //The sampling and conversion start.
											//ENC-> enables conversion while ADC10SC starts conversion.
											//--filter2() is called before enabling the conversion due to the ADC10 taking time to convert the
											//  analog input and put it into the ADC10MEM (memory). This was avoided in the in-lab with delay_cycles()...
											//----delay_cycles are not reliable and waste time, so enabling conversion after the first filter2() call
											//    will give ADC10 adequate time to convert and place to ADC10MEM. This allows it to be ready for the next filter2() call


	P1OUT &= ~Red;							//turns off the Red LED to show the end of the intterupt method.


}


/*
 * main.c
 */

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;				//stops the watchdog timer (program won't be interrupted if executed for a long period)

    TACCR0 = 0x260;							//sets the frequency to 460Hz, so that an interrupt occurs about every 2ms.

    TACCTL0 = CCIE | CM_0; 					//CCIE-> Capture/Compare Interrupt Enable; CM0-> no capture; Enables the timer.

    TACTL = TASSEL_2 | ID_3 | TACLR | MC_1; //TASSEL_2-> sets timer S source select to ACLK/8, specified as "/8" by ID_3;  MC_1 -> runs it in "count up"
    										//mode; TACLR -> clears the timer to start.

    DCOCTL = CALDCO_1MHZ; 					//set the clock frequency to 1MHz
    BCSCTL1 = CALBC1_1MHZ;

    P1DIR |= Green + Red;					//sets the green LED direction to output without changing the other values in P1DIR.
    P1OUT &= ~Red;							//Red LED is off to start
    P1OUT &= ~Green;						//Green LED is off to start

    _BIS_SR(GIE);   						//Global Interrupt Enable (GIE) after the set-up is complete


	P1DIR &= ~FGEN;							//sets the direction of PIN1.7 to be an input

	configureFunctionSensor();				//handles the initial configuration for ADC. (post-lab speed up)

    while(1){								//Endless while loop that allows the program to constantly run.
    }

	return 0;								//arbitrary return for the main
}
