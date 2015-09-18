
// 	Name:	Joey Weate (jfw6fp)
//	Partner: Sam Abbate (sja4cm)
//	Lab #:	Lab7 SPI and Bit Banging



#include <msp430.h>

//Pin1 macros
#define MISO BIT1	//master input, servant output
#define MOSI BIT2	//master output, servant input
#define SCLK BIT4	//serial Clock (output from master)
#define size 100
//Pin2 macros
#define WP BIT1		//Write Protocol: enable and disable BPL bit in SR
#define HOLD BIT3	//temporarily stop serial communication without reseting the device
#define SS BIT0	//same as the CE (Slave Select)

typedef unsigned char byte;
volatile byte byte2;
void byteProgram(char d1);
void aai();
byte readSR();
void writeSR(char );
void writeRandom();
void waitWhileBusy();
void chipErase();
byte receive ();
void  check();
void send (mybyte);
byte receivedWhat[100];	//array of what was received
byte sentWhat[100];		//array of what was sent
byte add1=0x00;
byte add2=0x00;
byte add3=0x00;


byte seed=128;  //starting point- seed

//creates a new random int based on the previous seed
byte newRandom(unsigned int seeds){//creates a new random number based on the previous number

	byte a= (((seeds&BIT5)>>5)^((seeds&BIT3)>>3)^((seeds&BIT1)>>1)^(seeds&BIT0));//new  bit7 by xoring bits 5, 3,1, and 0.
														//>> are used to shift bits so that they are all aligned; necessary to XOR them.
			a=a<<7;  //puts value of a into bit7
			seeds=((seeds>>1)|a);// shifts every bit in the seed by 1 then puts the value of a into the MSB.
			return seeds;//returns the new random number



}
void check(){		//compares what was written with peusdo-numbers
int i;
int tempseed=1;		//same starting seed as what was written
for(i; i<size; i++){

	tempseed= newRandom(tempseed);	//same next seed algo
	if(receivedWhat[i]==tempseed){	//if what was read is the same as what was meant to be written
		P1OUT |= BIT6;		//turn the green LED on.
	}
		else{
	P1OUT |= BIT0;		//turn the red LED on.
}
}

}


//enables writing to memory
void writeEnable() {  //aka WREN
	P2OUT |= SS;	//Forces a falling edge on the chip enable
	P2OUT &= ~SS;
	send(0x06);		//no data is returned, so the value is discarded.
	P2OUT |= SS;	//drive the SS high so that WREN instruction can be executed
	}

void writeEnableSR(){

	P2OUT |= SS;	//Forces a falling edge on the chip enable
		P2OUT &= ~SS;
		send(0x50);		//no data is returned, so the value is discarded.
		P2OUT |= SS;	//drive the SS high so that WREN instruction can be executed
}
//resets the Write-enable latch and AAI bit to 0;
void writeDisable(){	//aka WRDI
	P2OUT |= SS;	//Forces a falling edge on the chip enable
	P2OUT &= ~SS;
	send(0x04);		//no data is returned, so the value is discarded.
	P2OUT |= SS;	//drive the SS high so that WREN instruction can be executed

}

//write to the status register
void writeSR(char write){
	P2OUT |= SS;
	P2OUT &= ~SS;
	send(0x01);
	send(write);
	P2OUT |= SS;

}

void waitWhileBusy(){

	byte mybyte=readSR();
	int busy= (BIT0 & mybyte);
	while(busy==1){
		mybyte=readSR();
		busy=(BIT0 & mybyte);

	}

}
void chipErase(){	//erases the chip, putting active highs in all spots->0xFF
	P2OUT |= SS;	//Forces a falling edge on the chip enable
	P2OUT &= ~SS;
	send(0x60); //opcode
	P2OUT |= SS;
	waitWhileBusy();
}
void writeRandom(){
int x=0;

while(x<100){
	byte myByte= newRandom(seed);
	byteProgram(myByte);
	add3++;						//increments the address
	if(add3==0xFF){
		add3=0;
		add2++;
		if(add2==0xFF){
			add2=0;
			add1++;
		}
	}
	x++;

}

}
//programs bits in the slected byte to the desired data
void byteProgram ( char d1){
	P2OUT |= SS;	//Forces a falling edge on the chip enable
	P2OUT &= ~SS;	//SS must remain low for the duration of the Byte-Program
	send(0x02);		//8 bit instruction to initiate ByteProgram
	send(add1);		//sends address
	send(add2);		//sends address
	send(add3);		//sends address
	send(d1); 		//sends Din
	P2OUT|= SS;		//disable the CE
	_delay_cycles(5);
}
//multiple bytes programmed without re-issuing the next sequential address location
void aai() {	//auto address increment program
	int i=1;

	P2OUT |= SS;	//Forces a falling edge on the chip enable
	P2OUT &= ~SS;	//SS must remain low for the duration
	send(0xAF);	 	//initiates AAI
	send(0x00);			//send the address bits [A23-A0]
	send(0x00);			//sends address
	send(0x00);			//sends address (LSB)
	send(seed);			//sends the seed of the numbers
	sentWhat[0]=seed;	//adds the seed to the array
	P2OUT|= SS;			//drive the CE# high
	_delay_cycles(5);
	for(i; i<size; i++){  //end of memory
		P2OUT &= ~ SS;
		send(0xAF);		//initiates AAI
		seed= newRandom(seed);		//creates a new seed from the old seed
		sentWhat[i]=seed; //store the sent values
		send(seed);		//sends the value to be written
		P2OUT|= SS;			//drive the CE# high
		_delay_cycles(5);		//time delay to allow writing to occur

	}

	writeDisable();		//write-disable after the last desired byte
	//byte status = readSR();			//verify end of AAI operation
	P2OUT |= SS;		//drive SS high
}

//read from the status register
byte readSR(){ //aka RDSR

	P2OUT |= SS;	//Forces a falling edge on the chip enable
	P2OUT &= ~SS;	//SS must remain low for the durationn of the method
	send(0x05);		//the status register is read
	byte status = receive();		//8 bit instructions is read and stored
	P2OUT |= SS;	//drive SS high so that WREN instruction can be executed
	return status;	//returns byte that was received
	}

//bit banging function to receive an 8 bit instruction

void readMemory(){
	P2OUT |= SS;	//Forces a falling edge on the chip enable
	P2OUT &= ~SS;	//SS must remain low for the durationn of the method
	send(0x03);		//the status register is read
	send(0x00);		//sends starting addres which is 0x000000
	send(0x00);
	send(0x00);		//LSB
	int i=0;
for(i; i<size; i++){
	byte mybyte = receive();
	receivedWhat[i]=mybyte;

}
P2OUT |= SS;

}

byte receive (){

	P2OUT &= ~SS;		//enable the slave servant
	P1OUT |= SCLK;		//drive the clock low

	__delay_cycles(2);	//time delay allowing for the above commands to take action

	byte mybyte = 0;	//temp byte to store value in register

	int j;
	for ( j=0; j<8; j++){ //sends the byte bit by bit

		mybyte <<= 1;		//shift bit over (bit banging)

		if (P1IN & MISO){
			mybyte |= BIT0;			//active high
		}
		else{
			mybyte &= ~BIT0;		//Active low
		}

		P1OUT  &=  ~SCLK;	//drive the clock low
		__delay_cycles(1); 	//time delay allowing for clock to fall

		P1OUT |= SCLK;	//drive the clock high
		__delay_cycles(1);	//time delay allowing for clock to rise

		}

	return mybyte;	//return the  byte in the register
}
//bit banging function to send an 8 bit number
void send (mybyte){

	P2OUT &= ~SS;		//enable the slave select
	P1OUT &=~SCLK;		//drive the clock low
	__delay_cycles(2);	//time delay to allow commands above to take action

	int i;
	for ( i=0; i<8; i++){	//send the byte bit by bit (8 bits in a byte)

		if (mybyte & 0x80){
			P1OUT |= MOSI;		//output is active high
		}
		else{
			P1OUT &= ~MOSI;		//output is active low
		}

		mybyte <<= 1; //shifts mybyte values to the left 1 bit
		__delay_cycles(2);//delays to allow time to shift mybyte

		P1OUT  |=  SCLK;	//drives the clock high
		__delay_cycles(1); //delays to allow time for clock to rise

		P1OUT &=~SCLK;	//drives the clock low
		__delay_cycles(1);	//delays to allow tiem for clock to fall

	}

}


int main(void) {
    WDTCTL = WDTPW | WDTHOLD;				// Stop watchdog timer

    DCOCTL = CALDCO_16MHZ; 					//set the clock frequency to 16MHz
    BCSCTL1 = CALBC1_16MHZ;

    P1DIR |= MOSI + SCLK + BIT0 + BIT6;			//sets pin1's directions
    P2DIR |= WP + HOLD + SS;				//sets pin2's directions
    P1DIR &= ~MISO;

    P1OUT &= ~BIT0;					//starts red LED off
    P1OUT &= ~BIT6;					//starts green LED off

    writeEnableSR();
    writeSR(0);

    P2OUT |= SS;
    volatile byte byte1 = readSR();

    writeEnable();		//need to enable write before clearing(Writing FF to all spots)
    chipErase();		//clear the chip before writing
    writeEnable();		//enabling write...again
    aai();				//Auto incrementing ->writing to memory
    readMemory();		//reading from memory
    check();			//comparing what was written and read



    while(1){	//endless while loop
    	//writeEnable();  for the screenshots

    }

    return 0;
}
