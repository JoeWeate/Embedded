

#include <msp430.h> 
#include "useful.h"
#include "CORDIC.h"

//----------------ROTARY ENCODER LAB -----------------------

//---------------DEFINE STATEMENTS--------------------------
#define Button BIT3 // define Button variable as BIT 3
#define Red BIT0 // define Red LED as BIT0
#define Green BIT6 // define Green LED as BIT6

#define SIN B8_7(P1OUT) // define SIN-chip enable
#define SCLK B8_5(P1OUT) // define SCLK pin
#define LATCH B8_0(P2OUT) // define LATCH pin
#define BLANK B8_4(P1OUT) // define BLANK pin
#define BUTTON_HIGH    ((P1IN & 1<<3) == 0)

//macros for all the directions and their LED equivalents
#define N1	0x80
#define N2	0xC1
#define NE1 0x40
#define NE2	0xE0
#define E1	0x20
#define E2	0x70
#define SE1	0x10
#define SE2	0x38
#define S1	0x08
#define S2	0x1C
#define SW1	0x04
#define SW2	0x0E
#define W1	0x02
#define W2	0x0B
#define NW1	0x01
#define NW2	0x83
int tY=1200;
int tX=600;

//structs from the CORDIC.h file
coordinates coord;
measurement meas;

//global variables we update
int a;
int b;
int c;

//----------TYPEDEF INSTRUCTIONS--------------------

//specifies the desired direction we want to light the LEDs in
typedef enum PossibleDirections {
North, NorthEast, East, SouthEast, South, SouthWest, West, NorthWest, All
}Direction;

//specifies positive or negative x, y, or z axis being recorded.
typedef enum CalibrationAxis {
	pxa, mxa, pya, mya, pza, mza
} CalibrateDir;

//structs to specify which axis we are measuring and which direction to light up
CalibrateDir myAxis;
Direction myDirection;


//-----------GLOBAL VARIABLES------------------------
//-------------FUNCTION PROTOTYPES-------------------
void lightN();
void lightNE();
void lightNW();
void lightS();
void lightSE();
void lightSW();
void lightW();
void lightE();
int timer;
int counter;
int g1mSTimeout;
void InitPorts(); // Initializes all Port related variables
void InitTimerSystem(); //This should set up a periodic interrupt at a 1 microSecond rate using SMCLK as the clock source.
void light(byte mybyte);
volatile unsigned int measurements[3];

//calibration values are stored here.
int xMax;
int xMin;
int xOff;
int yMax;
int yMin;
int yOff;
int zMax;
int zMin;
int zOff;

int pxaInd=1;
int mxaInd=1;
int pyaInd=1;
int myaInd=1;
int pzaInd=1;
int mzaInd=1;
void getReadings();


#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR (void)
{
  __bic_SR_register_on_exit(CPUOFF);        // Clear CPUOFF bit from 0(SR)
}

#pragma vector = PORT1_VECTOR               // Port 1 interrupt service routine ->Post Lab


__interrupt  void  Port1_routine (void) { 	// This method handles the button interrupt.

P1IFG &= ~BIT3;										//Clears the interrupt flag that is shared among all bits of P1.

	switch (myAxis) {								//switch case executes different code each iteration within the same interrupt. Used in calibration
			    case pxa:
			    	getReadings();					//when in positive x orientation, record xMin, zOffset, and yOffset
			    	xMin = measurements[1];
			    	zOff = measurements[0];
			    	yOff = measurements[2];
			    	pxaInd = 0;
			    	_delay_cycles(1000);

			    	break;
			    case mxa:							//When in negative x orientation, record xMax
			    	getReadings();
			    	xMax = measurements[1];
			    	mxaInd = 0;
			    	_delay_cycles(1000);
			    	break;
			    case pya:							//When in positive y orientation, record yMax and xOffset
			    	getReadings();
			    	yMax = measurements[2];
			    	xOff = measurements[1];
			    	pyaInd = 0;
			    	_delay_cycles(1000);
			    	break;
			    case mya:							//When in negative y orientation, record yMin
			    	getReadings();
			    	yMin = measurements[2];
			    	myaInd = 0;
			    	_delay_cycles(1000);
			    	break;
			    case pza:							//When in positive z orientation, record zMax
			    	getReadings();
			    	zMax = measurements[0];
			    	pzaInd = 0;
			    	_delay_cycles(1000);
			    	break;
			    case mza:							//When in negative z orientation, record zMin
			    	getReadings();
			    	zMin = measurements[0];
			    	mzaInd = 0;
			    	_delay_cycles(1000);
			    	break;


	}

	P1IFG &= ~BIT3;									//clear interrupt flag
}


void calibrate(){									//calibration method
	switch (myAxis) {
		    case pxa:								//when measuring the positive x, light up East and sense input. Then record it appropriately
		    	while(pxaInd&&(!(BUTTON_HIGH))){
		    		light(E1);
		        	getReadings();
		    	}

		    	xMin = measurements[1];
		    	zOff = measurements[0];
		    	yOff = measurements[2];
		    	pxaInd = 0;
		    	_delay_cycles(1000);

		    	_delay_cycles(100000);
		    	break;
		    case mxa:								//when measuring the negative x, light up west and sense input. Then record it appropriately
		    	while(mxaInd&&(!(BUTTON_HIGH))){
		    		light(W1);
		    		getReadings();
		    	}
		    	xMax = measurements[1];
		    	mxaInd = 0;
		    	_delay_cycles(100000);
		    	break;
		    case pya:								//when measuring the positive y, light up north and sense input. Then record it appropriately
		    	while(pyaInd&&(!(BUTTON_HIGH))){
		    		light(N1);
		    		getReadings();
		    	}
		    	yMax = measurements[2];
		    	xOff = measurements[1];
		    	pyaInd = 0;
		    	_delay_cycles(100000);
		    	break;
		    case mya:								//when measuring the negative y, light up south and sense input. Then record it appropriately
		    	while(myaInd&&(!(BUTTON_HIGH))){
		    		light(S1);
		    		getReadings();
		    	}
		    	yMin = measurements[2];
		    	myaInd = 0;
		    	_delay_cycles(100000);
		    	break;
		    case pza:								//when measuring the positive z, light up all LEDs and sense input. Then record it appropriately
		    	while(pzaInd&&(!(BUTTON_HIGH))){
		    		light(0xFF);
		    		getReadings();
		    	}
		    	zMax = measurements[0];
		    	pzaInd = 0;
		    	_delay_cycles(100000);
		    	break;
		    case mza:								//when measuring the negative z, light up none and sense input. Then record it appropriately
		    	while(mzaInd&&(!(BUTTON_HIGH))){
		    		light(0x00);
		    		getReadings();
		    	}
		    	zMin = measurements[0];
		    	mzaInd = 0;
		    	_delay_cycles(100000);
		    	break;
	}
}

void lightUp(){

counter++;
if(counter>=30){
counter=0;
}

if(counter>= 0 && counter <=15){			//primary direction lights up

	switch (myDirection) {
	    case North:
	    	light(N1);
	    	//_delay_cycles(10);
	    	break;

	    case NorthEast:
	    	light(NE1);
	    	break;

	    case East:
	    	light(E1);
	    	//_delay_cycles(10);
	    	break;
	    case SouthEast:
	    	light(SE1);
	    	break;
	    case South:
	    	light(S1);
	    	break;

	    case SouthWest:
	    	light(SW1);
	    	break;

	    case West:
	    	light(W1);
	    	break;

	    case NorthWest:
	    	light(NW1);
	    	break;

	    case All:
	    	light(0xFF);
	    	_delay_cycles(25000);
	    	break;

	}

}
else if (counter <=16){						//secondary directions light up, with primary. Creates PWM effect.

	switch (myDirection) {		//determines what LED's to light
	    case North:
	    	light(N2);
	    	break;

	    case NorthEast:
	    	light(NE2);
	    	break;

	    case East:
	    	light(E2);
	    	break;
	    case SouthEast:
	    	light(SE2);
	    	break;
	    case South:
	    	light(S2);
	    	break;

	    case SouthWest:
	    	light(SW2);
	    	break;

	    case West:
	    	light(W2);
	    	break;

	    case NorthWest:
	    	light(NW2);
	    	break;

	    case All:
	    	light(0x00);
	    	_delay_cycles(25000);
	    	break;

	}

}
else{
	light (0x00);
}
}
//-----------------------------------------

void getReadings(void){
	 ADC10CTL0 &= ~ENC;
	    while (ADC10CTL1 & BUSY);               // Wait if ADC10 core is active
	    ADC10SA = (int) &(measurements);        // Data buffer start
	    ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion ready
}

void InitPorts() {
    P1REN |= BIT3;                          // Enable pull-up/pull-down resistor
    P1OUT |= BIT3;                          // Declare resistor pull-up
    P1IE |= BIT3;
    P1DIR &= ~ BIT3;

    P1DIR |= BIT7;							//Bits for bit banging declared as output
    P1DIR |= BIT5;
    P2DIR |= BIT0;
    P1DIR |= BIT4;


	P1DIR &= ~BIT0;							//accelerometer bits declares as inputs
	P1DIR &= ~BIT1;
	P1DIR &= ~BIT2;

}
//-----------------------------------------

void light(byte myByte) {    //bit bangs the proper LED lighting

    SCLK = LOW;
    _delay_cycles(1);
    LATCH = LOW;
    _delay_cycles(1);
    int i;

    for (i = 0; i < 8; i++) {
        if (myByte & 0x80) {
            SIN = HIGH;
        } else {
            SIN = LOW;
        }
        myByte <<= 1;

        _delay_cycles(1);

        SCLK = HIGH;
        _delay_cycles(1);
        SCLK = LOW;
        _delay_cycles(1);

    }
    LATCH = LOW;        //latch is low to force a rising;
    _delay_cycles(1);
    LATCH = HIGH;
}


void StartUp(){					//initialization sequence starts on boot.

    light(0x01);				//flash nw, w, s, se, e, ne, n and delay the cycles in between to create a slow sliding circle of lit LEDs
    _delay_cycles(300000);
    light(0x02);//w
    _delay_cycles(300000);
    light(0x04);//sw
    _delay_cycles(300000);
    light(0x08);//s
    _delay_cycles(300000);
    light(0x10);//se
    _delay_cycles(300000);
    light(0x20);//e
    _delay_cycles(300000);
    light(0x40);//ne
    _delay_cycles(300000);
    light(0x80);//n
    _delay_cycles(300000);

}

void getDirection(){							//this method uses the calculated angle and tilt to determine what direction the LEDs should show.

	if((meas.tilt<1700)&&(meas.tilt>1200)){
		myDirection = All;
	}

	else if(meas.angle < 5000){
		myDirection = SouthWest;
	}
	else if((meas.angle < 13000)&&(meas.angle > 5000) ){
		myDirection = South;
	}
	else if((meas.angle < 18000)&&(meas.angle > 13000) ){
			myDirection = SouthEast;
		}
	else if((meas.angle < 20500)&&(meas.angle>18000) ){
			myDirection = East;
		}
	else if((meas.angle < 24000)&&(meas.angle>20500) ){
			myDirection = NorthEast;
		}
	else if((meas.angle < 29000)&&(meas.angle>24000) ){
			myDirection = North;
		}
	else if((meas.angle < 33500)&&(meas.angle>29000) ){
			myDirection = NorthWest;
		}
	else if((meas.angle < 36000)&&(meas.angle>33500) ){
			myDirection = West;
		}
}

int main(void)
{

    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
    //*************** Initialization Section ***************************
    InitPorts();

    //ADC initialization
    ADC10CTL1 = INCH_2 + CONSEQ_3;            // A2/A1/A0, repeat multi channel
    ADC10CTL0 = ADC10SHT_2 + MSC + ADC10ON + ADC10IE + REFON;
    ADC10CTL0 &= ~REF2_5V;
    ADC10AE0 = (BIT0 + BIT1 + BIT2);          // P1.0,1, 2 Analog enable
    ADC10DTC1 = 0x03;                         // number of conversions

    StartUp();					//start up sequence
    calibrate();				//calibrate pxa first, then delay to give user time and to let the button settle.
    _delay_cycles(100000);
    myAxis = mxa;				//then calibrate the rest of the axises in the same way.
    _delay_cycles(100000);
   calibrate();
   _delay_cycles(100000);
    myAxis = pya;
    _delay_cycles(100000);
    calibrate();
    _delay_cycles(100000);
    myAxis = mya;
    _delay_cycles(100000);
    calibrate();
    _delay_cycles(100000);
    myAxis = pza;
    _delay_cycles(100000);
    calibrate();
    _delay_cycles(100000);
    myAxis = mza;
    _delay_cycles(100000);
    calibrate();
    _delay_cycles(100000);


    //********************* End of Initialization Section *********************
    BLANK = LOW; // Drive Blank low

    _BIS_SR(GIE);  //Enable Global Interrupts Here ONLY

	P1DIR &= ~BIT0;
	P1DIR &= ~BIT1;
	P1DIR &= ~BIT2;

  while(1)
  {

	 getReadings();								//get the readings
	 a = (measurements[1] - ((xMax+xMin)>>1));	//use calibrated info to normalize x,y,z inputs
	 b = (measurements[2] - ((yMax+yMin)>>1));
	 c = (measurements[0] - ((zMax+zMin)>>1));


	 coord.x = a;								//add normalized data to the coord struct
	 coord.y = b;
	 coord.z = c;


	 meas = getDisplaySetting(&coord);			//run the CORDIC function on this data to determine the angle and tilt of the device.

	 getDirection();							//according to this angle and tilt, set the LED direction
	 lightUp();									//lastly, light up the LED accordingly.



  }
}

