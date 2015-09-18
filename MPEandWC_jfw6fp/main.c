/*
 *
 */
//init variablies->
//long long 4 rishin edges if alst four equal to f then save.//error checking


//Include Section:
#include <msp430.h>
#include "useful.h"
//This code is a skeleton for a Manchester Phase Encoded Transmitter
//It may transmit any number of bits up to 32.
//A separate start bit is sent, with a rising edge at mid-bit.
//After this, normal transmission is sent
//This version defines a 1 as a rising edge at mid-bit
//Transmission rate is 1 mS per bit.
//There is an inter-transmission time of 50 mS, then repeat transmission
//A 500uS clock tick is interrupt is required.

//Constant define section:
#define BITS_IN_TRANSMISSION  32
#define TRANSMIT_PIN          B8_5(P1OUT)
#define INTERWORD_DELAY       50    //This is in units of mS
#define SIZE_OF_RCV_QUE        4 //Must be a power of 2!

//The following defines are for bit width definitions
//The underlying assumptions are"
//The capture timers are updated with a 1uS resolution
//A full bit time is 1 mS nominal
#define VALID_HALF_BIT_MIN    450
#define VALID_HALF_BIT_MAX    550
#define VALID_FULL_BIT_MIN    950
#define VALID_FULL_BIT_MAX   1050
#define MISSING_EDGE_TIMEOUT 1200
#define Red BIT0

//typedef section
enum Transmit_States {StartBit,NormalXmit,InterWord} ;
enum XmitClockPhase  {High,Low} ;
typedef struct {
    unsigned long        Transmit_Data         ;  //This is the data to actually be transmitted
    unsigned long        Transmit_Data_Buffer  ;  //This should be reloaded any time we wish to change what is transmitted.
    unsigned int         Bits_Remaining        ;  //This is the number of bits remaining in the current transmission
    enum XmitClockPhase  Transmit_Clock_Phase  ;  //This gets updated once every 1/2 bit period (500 uS in this case.)
    unsigned int         InterwordTimeout      ;  //This represents a "dead" period between successive transmissions
    enum Transmit_States Transmitter_State     ;  //This is the current state machine state for the transmitter
} TransmitterData ;

//Receiver Definitions and declarations
enum Captured_Edge {Rising,Falling} ;  //these are the 2 types of edges in the received signal
typedef enum Captured_Edge EdgeType ;

typedef struct {
    EdgeType     Edge      ;    // Which type of edge was received
    unsigned int TimeStamp ;    // When we got it.
} Event_Que_Entry ;

typedef struct {
    Event_Que_Entry Events[SIZE_OF_RCV_QUE] ; //What each entry looks like
    unsigned int  QueSize ;                   //Current size of queue
    unsigned int  Get_Index ;                 //Where we get data from
    unsigned int  Put_index ;                 //Where we put new data
} Event_Queue ;

enum Que_Errors {No_Error,Que_Full,Que_Empty} ;
typedef enum Que_Errors QueReturnErrors       ;

//The following typedefs are for the receiver section
typedef enum Rcv_States {Initial_Expect_Rising,Initial_Expect_Falling,MidBit_Expect_Rising,MidBit_Expect_Falling} ReceiverStates ;
typedef struct {
    ReceiverStates  CurrentRcvState      ;   // State for state machine implementation
    unsigned int    RisingEdgeTimeStamp  ;   // Time stamp at leading edge of signal
    unsigned int    FallingEdgeTimeStamp ;   // TIme stamp for falling edge
    unsigned int    PulseWidth           ;   // Difference in time between edges
    unsigned int    MidBitTimeStamp      ;   // Time Stamp of last valid mid-bit transition
    unsigned int    LastEdgeTimeStamp    ;   // When the last edge occured, regardless of polarity
    unsigned long   CurrentRecvdData     ;   // Data that is being shifted in
    unsigned long   LastValidReceived    ;   // Last complete valid word.
    unsigned int    BitsLeftToGet        ;   // Number of bits to go in reception.
}ManchesterReceiver;

typedef enum PulseWidths {Invalid_Width,Valid_HalfBit,Valid_FullBit} PulseWidthStatus ;

// ********************************************************************************************
// Function prototype declarations Section
void InitHardware(void) ; //This initializes all hardware subsystems, timer, ports etc.
void InitVariables(void); //All Global Variables are set up by this
void Xmit(TransmitterData * TData) ; //This routine is called every 500 uS by an interrupt handler.
void rcv(void) ;
//Receiver Queue functions:
//This function places a new event in the event queue structure.
//The rising and falling edge detectors should call this handler with the appropriate info!
QueReturnErrors InsertEvent(EdgeType DetectedEdge, unsigned int CapturedTime);

//This is called from within the main loop to see if there are any events on the que, i.e. captured edges.
//Note that it disables interrupts to ensure that data is not overwritten by an interrupter
//If no event is pending, it returns a -1, otherwise it returns an index into the queue that corresponds to the oldest
//event.
int GetEvent(void);
int highsSent;
int highsRec;
int send; //if 0->receieving, if 1->sending
//This functions tests a current pulse width and determines if it is a valid width
PulseWidthStatus TestWidth(unsigned int CurrentPulse);

// ************************************************************************************************

// **************************************************************************************
//Global Variables Section
int gStartCounter;
int bit;
int msCounter;
int startCounter;
int amountsent;
int temp;
int temp2;
// ************************************************************************************
TransmitterData Xmit1 ;  //This declares an instance of the transmitter data structure.
ManchesterReceiver Rcv1 ;
//Receiver Global Variables
Event_Queue Receiver_Events ;

int correct;
int compare;
int compare2;
void main(void) {

//Be sure to stop watchdog timer first!

    WDTCTL = WDTHOLD + WDTPW ;

    InitHardware() ;
    InitVariables() ;

//Enable Global Interrupts after all intializing is done.
    _EINT() ;
    while (1) { //Main code loop here :
        //Xmit(&Xmit1);

    	if(send==0){
        rcv() ; //Call the receiver
        rcv();

        //check if right




        temp= highsSent/2;
        compare = (highsSent -(2*temp));				//different way to to do highsent mod 2
        if((Rcv1.LastValidReceived==0xCCCCCCCC)&&(compare==0)){			//if mod is even and even signal was recieve on the other end
        	correct=1;			//good-> signal was recieved correctly according to even bit parity
        }

        else if((Rcv1.LastValidReceived==0xFFFFFFFF)&&((!(compare==0)))){	//if mod is odd and odd signal was recieve on the other end
        					correct=1;		//good
        }

        else{						//bad

        		correct=0;		//an error occured in transmitting
        }

    }
}

}
//Set up globals, including the transmitter structure.
//Comment Well
void InitVariables(void){
//Here is an example:
	highsSent=0;
	highsRec=0;
	send=0;						//send is 1 if sending, 0 if recieving.
    Xmit1.Bits_Remaining = BITS_IN_TRANSMISSION ;
    Xmit1.Transmit_Data_Buffer = 0xAABBCCDA ;  //
    Xmit1.Transmit_Data = 0xAABBCCDA      ; //This is just sample data, the final application Determines what is to be sent.
    Xmit1.Transmit_Clock_Phase = Low ;
    Xmit1.Transmitter_State = StartBit ;
//etc. .....
    Receiver_Events.Get_Index = 0 ;
    Receiver_Events.Put_index = 0 ;
    Receiver_Events.QueSize   = 0 ;

//etc.........
    Rcv1.CurrentRcvState = Initial_Expect_Rising ;
    Rcv1.CurrentRecvdData =      0 ;
    Rcv1.FallingEdgeTimeStamp =  0 ;
    Rcv1.RisingEdgeTimeStamp =   0 ;
    Rcv1.MidBitTimeStamp =       0 ;
    Rcv1.PulseWidth =            0 ;
    Rcv1.CurrentRecvdData =      0 ;
    Rcv1.LastValidReceived =     0 ;

    amountsent=0;
    startCounter=0;
    msCounter=0;
    TRANSMIT_PIN = Low;
}

//This routine manages the actual transmitter and is called every 500uS by a periodic interrupt.
//Comment Well
void Xmit(TransmitterData* TData) {
    enum XmitClockPhase Phase;
//Each 500 uS half bit period constitutes a separate clock "phase" for transmitter purposes.
    if (TData->Transmit_Clock_Phase == Low){
        TData->Transmit_Clock_Phase = High ;
    }
    else TData->Transmit_Clock_Phase = Low ;
    Phase = TData->Transmit_Clock_Phase ;
//Now do state machine
    switch(TData->Transmitter_State){
        case StartBit :					//start bit
            switch(Phase) {
                case Low :

                    if(startCounter==2){
                        TRANSMIT_PIN=1;
                        startCounter--;
                    }
                    else if(startCounter==0){
                        TRANSMIT_PIN=0;
                        startCounter = 3;
                        TData->Transmitter_State=NormalXmit;
                    }


                break ;
                case High :

                    if(startCounter==3){
                        TRANSMIT_PIN = 0;
                        startCounter--;

                    }

                    else if(startCounter==1){
                        TRANSMIT_PIN = 1;
                        startCounter--;
                    }

                break ;
            }
        break ;

        case NormalXmit :
            bit = BIT0 & (TData->Transmit_Data>>(TData->Bits_Remaining-1)); // starts with USB
            switch(Phase) {
                case Low :
                    if(TData->Bits_Remaining<1){
                    	amountsent++;				//a full word was sent once.
                        TData->Transmitter_State=InterWord;

                        }
                    else{
                        if(bit==1){
                            TRANSMIT_PIN = 1;			//sends a high signal
                            highsSent++;
                        }
                        else{
                            TRANSMIT_PIN = 0;			//still low
                        }
                        TData->Bits_Remaining--;
                        //TData->Transmit_Clock_Phase=High;
                        //Phase=TData->Transmit_Clock_Phase;
                    }

                break ;
                case High :
                    if(TData->Bits_Remaining<1){
                    	amountsent++;
                        TData->Transmitter_State=InterWord;

                    }
                    else{
                        if(bit==1){

                            TRANSMIT_PIN = 0;			//need to send a 0 so that can get back up to high
                        }
                        else{
                            TRANSMIT_PIN = 1;
//
                        }

                    }
                break ;
            }
        break ;

        case InterWord :					//time delay between sending of words
            switch(Phase) {

                case Low :
                    TRANSMIT_PIN = 0;
                    msCounter++;
                    TData->Transmit_Clock_Phase=Low;
                    Phase=TData->Transmit_Clock_Phase;
                break ;

                case High :
                    if(msCounter>5){				//5ms time delay

                        TData->Transmitter_State=StartBit;
                        TData->Bits_Remaining=BITS_IN_TRANSMISSION;
                        msCounter=0;
                        if(amountsent<=10){//send 10 times
                        	highsSent=0;
                        }
                        if(amountsent>10){				//sends the signal 10 times then switches to recieving
                               send=0;					//own boolean to decide if sending or recieving
                               P1OUT|=BIT6;				//flag needed for the header board to switch between rec and transmit.
                        }

                        //the below code is needed on the starting recievers end.  S
//                        if(amountsent=1){
//                        	highsRec=highsSent;

//                        temp2= highsRec/2;
//                        compare2 = (highsRec -(2*temp2));
//                        if(compare2==0){
//                                              Xmit1.Transmit_Data=0xCCCCCCCC;//send CCC if transmitted had even number of highs
//                                              Xmit1.Transmit_Data_Buffer= 0xCCCCCCCC;
//
//
//                                       }
//                                     else{
//
//                                    Xmit1.Transmit_Data=0xFFFFFFFF;			//send FFF if transmitted had odd number of highs
//                                    Xmit1.Transmit_Data_Buffer= 0xFFFFFFFF;
//
//                                                       }

//
//
//                        }

                    }
                break ;
            }
        break ;

        default :
            TData->Transmitter_State = StartBit ;
//Other intitialization here.....
        break ;

    }
}

//This should be called Frequently from the main loop.
void rcv(void){
    int index ;
    PulseWidthStatus PWidth ;
    unsigned int CurrentTime ;
    Event_Que_Entry Current_Event ;
    CurrentTime = TA1R ; //Get the approximate current timestamp.
    if ((CurrentTime - Rcv1.LastEdgeTimeStamp) > MISSING_EDGE_TIMEOUT ){//Here we have had no transmissions in a while
        Rcv1.CurrentRcvState = Initial_Expect_Rising ;
    }
    index = GetEvent() ;
    if (index != -1 ) { //Here we have an edge to deal with, -1 indicates no event in queue
        Current_Event.Edge = Receiver_Events.Events[(unsigned int)index].Edge ;
        Current_Event.TimeStamp = Receiver_Events.Events[(unsigned int)index].TimeStamp ;
        //Now insert receiver state machine here
        Rcv1.LastEdgeTimeStamp = Current_Event.TimeStamp ; //This marks the last time that we got any time stamp at all
        switch (Rcv1.CurrentRcvState){
            // Ignore transitions until we get a rising one.
            case Initial_Expect_Rising :
                if (Current_Event.Edge == Rising) { //Leading edge of initial lead-in bit
                    Rcv1.CurrentRcvState = Initial_Expect_Falling ;
                    Rcv1.RisingEdgeTimeStamp = Current_Event.TimeStamp ;

                   // highsRec++;
                }
            break ;
            case Initial_Expect_Falling :
                if (Current_Event.Edge == Rising){
                    Rcv1.CurrentRcvState = Initial_Expect_Rising ; //Out of sequence start over
                }
                else {
                    Rcv1.FallingEdgeTimeStamp = Current_Event.TimeStamp ;  //Figure out when it happens
                    Rcv1.PulseWidth = Rcv1.FallingEdgeTimeStamp - Rcv1.RisingEdgeTimeStamp ; // And Test validity
                    PWidth = TestWidth(Rcv1.PulseWidth) ;
                    if (PWidth == Valid_FullBit) { //Here we have a valid full initial bit
                        Rcv1.CurrentRecvdData = 0 ; // Start all over for receiver
                        Rcv1.MidBitTimeStamp = Rcv1.FallingEdgeTimeStamp ; // By definition at mid bit....
                        Rcv1.BitsLeftToGet = BITS_IN_TRANSMISSION ;
                        Rcv1.CurrentRcvState = MidBit_Expect_Rising ; //Next bit is start of "real" data
                    }
                    else Rcv1.CurrentRcvState = Initial_Expect_Rising ; //Likely a noise pulse, start over
                }
            break ;
            case MidBit_Expect_Falling :
                if (Current_Event.Edge == Rising) { //Out of sequence - start over
                    Rcv1.CurrentRcvState = Initial_Expect_Rising ;
                }
                else {
                    Rcv1.FallingEdgeTimeStamp = Current_Event.TimeStamp ;
                    Rcv1.PulseWidth = Rcv1.FallingEdgeTimeStamp - Rcv1.MidBitTimeStamp ; // Get width relative to last mid-bit
                    PWidth = TestWidth(Rcv1.PulseWidth) ;
                    if (PWidth == Valid_HalfBit) { //Here we have a half-bit, phasing transition
                        Rcv1.CurrentRcvState = MidBit_Expect_Rising ; // Got to expect a rising edge at mid-bit
                    }
                    else {
                        if (PWidth == Valid_FullBit) {    // Rising Edge at mid-bit , clock in a 1
                            Rcv1.CurrentRecvdData <<= 1 ; // Room for new bit
                            //
                            --Rcv1.BitsLeftToGet ;
                            if (Rcv1.BitsLeftToGet == 0){ //All done Start over

                                Rcv1.LastValidReceived = Rcv1.CurrentRecvdData ; //Buffer up last received value
                                P1OUT |= Red;									//turn on the red LED when a signal is receieved
//-------------------------------------------
                                Xmit1.Bits_Remaining=BITS_IN_TRANSMISSION;
                                Xmit1.Transmitter_State=StartBit;

                                Xmit1.Transmit_Data= Rcv1.LastValidReceived;//send CCC if transmitted had even number of highs
                                											//send the word that was just received.  In order to count the amount of highs in the signal
																			//this signal will then be checked if it was correct or not

                                Xmit1.Transmit_Data_Buffer= Rcv1.LastValidReceived;
                                Xmit1.Transmit_Clock_Phase = Low ;
                                send=1;
                                //P1OUT&=~BIT6;
          //-------------------------------------------
                                //have received something.... need to send a message back!!!

                                Rcv1.CurrentRcvState = Initial_Expect_Rising ;
                            }
                            else {
                                Rcv1.MidBitTimeStamp = Rcv1.FallingEdgeTimeStamp ; //New mark for mid-bit
                                Rcv1.CurrentRcvState = MidBit_Expect_Rising    ; //And Expect a rising edge
                            }
                        }
                        else{
                            Rcv1.CurrentRcvState = Initial_Expect_Rising ; // Bad pulse width
                        }
                    }
                }
            break ;
            //We arrived here from a valid mid-bit transition previously
            case MidBit_Expect_Rising :
                if (Current_Event.Edge == Falling) { //Out of sequence - start over
                    Rcv1.CurrentRcvState = Initial_Expect_Rising ;
                }
                else {
                    Rcv1.RisingEdgeTimeStamp = Current_Event.TimeStamp ;
                    Rcv1.PulseWidth = Rcv1.RisingEdgeTimeStamp - Rcv1.MidBitTimeStamp ; // Get width relative to last mid-bit
                    PWidth = TestWidth(Rcv1.PulseWidth) ;
                    if (PWidth == Valid_HalfBit) { //Here we have a half-bit, phasing transition
                        Rcv1.CurrentRcvState = MidBit_Expect_Falling ; // Got to expect a falling edge at mid-bit
                    }
                    else {
                        if (PWidth == Valid_FullBit) {    // Rising Edge at mid-bit , clock in a 1
                        	highsRec++;



                            Rcv1.CurrentRecvdData <<= 1 ; // Room for new bit
                            Rcv1.CurrentRecvdData |= 0x01 ;
                            --Rcv1.BitsLeftToGet ;


                            if (Rcv1.BitsLeftToGet == 0){ //All done Start over
                                Rcv1.LastValidReceived = Rcv1.CurrentRecvdData ; //Buffer up last received value
                                P1OUT |= Red;

                        //-------------------------------------------
                                Xmit1.Bits_Remaining= BITS_IN_TRANSMISSION;
                                Xmit1.Transmitter_State=StartBit;

                                Xmit1.Transmit_Data= Rcv1.LastValidReceived;//send CCC if transmitted had even number of highs
                                Xmit1.Transmit_Data_Buffer= Rcv1.LastValidReceived;
                                Xmit1.Transmit_Clock_Phase = Low ;
                                send=1;
                                //P1OUT&=~BIT6;
                         //-------------------------------------------
                                Rcv1.CurrentRcvState = Initial_Expect_Rising ;
                            }
                            else {
                                Rcv1.MidBitTimeStamp = Rcv1.RisingEdgeTimeStamp ; //New mark for mid-bit
                                Rcv1.CurrentRcvState = MidBit_Expect_Falling    ; //And Expect a falling edge
                            }
                        }
                        else{
                            Rcv1.CurrentRcvState = Initial_Expect_Rising ; // Bad pulse width
                        }
                    }
                }
            break ;
            default:
                Rcv1.CurrentRcvState = Initial_Expect_Rising ;
            break ;
        }
    }

}

//This function places a new event in the event queue structure.
//The rising and falling edge detectors should call this handler with the appropriate info!
QueReturnErrors InsertEvent(EdgeType DetectedEdge, unsigned int CapturedTime){
    QueReturnErrors rval ;
    unsigned int putindex ;
    rval = No_Error ;
    if (Receiver_Events.QueSize == 4) {//Here Que is already full
        rval = Que_Full ;
    }
    else { //Here we can insert a new event
        ++Receiver_Events.QueSize ;
        putindex = Receiver_Events.Put_index ;
        Receiver_Events.Events[putindex].Edge = DetectedEdge ;
        Receiver_Events.Events[putindex].TimeStamp = CapturedTime ;
        ++putindex ;
        putindex &= SIZE_OF_RCV_QUE-1 ; //Note the constant must always be a power of 2!
        Receiver_Events.Put_index = putindex ;
    }
    return rval ;
}

//This is called from within the main loop to see if there are any events on the que, i.e. captured edges.
//Note that it disables interrupts to ensure that data is not overwritten by an interrupter
//if the return value is negative, then the queue is empty, else it returns an index to the oldest event in the
//queue
int GetEvent(void){

    int rval ;
    unsigned int getindex ;
    rval = -1 ;
    asm("  PUSH.B SR") ;

    _DINT() ;
    if (Receiver_Events.QueSize == 0) { //Nothing to be had!
        rval = -1 ;
    }
    else {
        getindex = Receiver_Events.Get_Index ;
        rval = (int)(getindex) ;
        ++getindex ;
        getindex &= SIZE_OF_RCV_QUE-1 ; //Note the constant must always be a power of 2!
        Receiver_Events.Get_Index = getindex ;
        --Receiver_Events.QueSize ;
    }
    asm("  POP.B SR");
    return rval ;
}

//This functions tests a current pulse width and determines if it is a valid width
PulseWidthStatus TestWidth(unsigned int CurrentPulse){
    PulseWidthStatus rval ;
    rval = Invalid_Width ;
    if ((CurrentPulse >= VALID_HALF_BIT_MIN) && (CurrentPulse <= VALID_HALF_BIT_MAX)) {
        rval = Valid_HalfBit ;
    }
    else {
        if ((CurrentPulse >= VALID_FULL_BIT_MIN) && (CurrentPulse <= VALID_FULL_BIT_MAX)){
            rval = Valid_FullBit ;
        }
    }
    return rval ;
}


//Functions called via an  interrupt
//This is called every 500uS by the timer A0 interrupt function
void ihandler(void) {
//Do whatever needs to be done on a periodic basis here:
	if(send==1){
    Xmit(&Xmit1);
	}
}

//This called by the capture routine on the rising edge of the input signal
void risingedge(void) {
    InsertEvent(Rising, TA1CCR0) ;  //Insert this event into event Queue
}

void fallingedge(void){
    InsertEvent(Falling, TA1CCR1) ; //Insert this event into event Queue.
}


//Defines for initializiation of various subsystems
//Clock System Initialization
void BCSplus_initial(void) ;
//Timer Initial..
void Timer0_A3_initial(void) ;
void Timer1_A3_initial(void) ;
//All hardware initializing is done here.

//Comment Well!
void InitHardware(void) {

//Set up ports here :

// End of port setup/
    BCSplus_initial()   ; //get clock going - 8 mhz rate
    Timer0_A3_initial() ;
    Timer1_A3_initial() ;
    P1DIR|=BIT5;
    P2SEL|=BIT1;
    P2SEL|=BIT0;
    P2DIR&=~(BIT0 + BIT1);
    P2OUT|=(BIT0 + BIT1);
    P1DIR|=BIT6;
    P1OUT&=~BIT6; 			// |= for receiving, &=~ for transmit
    //P1OUT|=BIT6;			//this line is needed if recieving first
    P1DIR |=  Red;					//set the green and red LED direction to output without changing the other values in P1DIR.
    P1OUT &= ~Red;					//start with the Red LED off

}





// Interrupt Handlers
#pragma vector=TIMER0_A0_VECTOR
__interrupt void periodicTimerA0Interrupt(void){
    /* Capture Compare Register 0 ISR Hook Function Name */
    ihandler();

    /* No change in operating mode on exit */
}


/*
 *  ======== Timer1_A3 Interrupt Service Routine ========
 */
#pragma vector=TIMER1_A0_VECTOR
__interrupt void timerCaptureRisingInterrupt(void){
    /* Capture Compare Register 0 ISR Hook Function Name */
    risingedge();
    /* No change in operating mode on exit */
}

/*
 *  ======== Timer1_A3 Interrupt Service Routine ========
 */
#pragma vector=TIMER1_A1_VECTOR
__interrupt void timerCaptureFallingInterrupt(void){
    switch (__even_in_range(TA1IV, TA1IV_TAIFG))    // Efficient switch-implementation
    {
        case TA1IV_TACCR1:
            /* Capture Compare Register 1 ISR Hook Function Name */
            fallingedge();
            /* No change in operating mode on exit */
            break;
        case TA1IV_TACCR2:
            break;
        case TA1IV_TAIFG:
            break;
    }
}

/*
 *  ======== BCSplus_init ========
 *  Initialize MSP430 Basic Clock System
 */
void BCSplus_initial(void)
{
    /*
     * Basic Clock System Control 2
     *
     * SELM_0 -- DCOCLK
     * DIVM_0 -- Divide by 1
     * ~SELS -- DCOCLK
     * DIVS_0 -- Divide by 1
     * ~DCOR -- DCO uses internal resistor
     *
     * Note: ~<BIT> indicates that <BIT> has value zero
     */
    BCSCTL2 = SELM_0 + DIVM_0 + DIVS_0;

    if (CALBC1_8MHZ != 0xFF) {
        /* Adjust this accordingly to your VCC rise time */
        __delay_cycles(100000);

        // Follow recommended flow. First, clear all DCOx and MODx bits. Then
        // apply new RSELx values. Finally, apply new DCOx and MODx bit values.
        DCOCTL = 0x00;
        BCSCTL1 = CALBC1_8MHZ;      /* Set DCO to 8MHz */
        DCOCTL = CALDCO_8MHZ;
    }

    /*
     * Basic Clock System Control 1
     *
     * XT2OFF -- Disable XT2CLK
     * ~XTS -- Low Frequency
     * DIVA_0 -- Divide by 1
     *
     * Note: ~XTS indicates that XTS has value zero
     */
    BCSCTL1 |= XT2OFF + DIVA_0;

    /*
     * Basic Clock System Control 3
     *
     * XT2S_0 -- 0.4 - 1 MHz
     * LFXT1S_2 -- If XTS = 0, XT1 = VLOCLK ; If XTS = 1, XT1 = 3 - 16-MHz crystal or resonator
     * XCAP_1 -- ~6 pF
     */
    BCSCTL3 = XT2S_0 + LFXT1S_2 + XCAP_1;
}

// *****************************************************************************************

/*
 *  ======== Timer0_A3_init ========
 *  Initialize MSP430 Timer0_A3 timer
 */
void Timer0_A3_initial(void)
{
     /*
         * TA0CCTL0, Capture/Compare Control Register 0
         *
         * CM_0 -- No Capture
         * CCIS_0 -- CCIxA
         * ~SCS -- Asynchronous Capture
         * ~SCCI -- Latched capture signal (read)
         * ~CAP -- Compare mode
         * OUTMOD_0 -- PWM output mode: 0 - OUT bit value
         *
         * Note: ~<BIT> indicates that <BIT> has value zero
         */
        TA0CCTL0 = CM_0 + CCIS_0 + OUTMOD_0 + CCIE;

        /* TA0CCR0, Timer_A Capture/Compare Register 0 */
        TA0CCR0 = 499;

        /*
         * TA0CTL, Timer_A3 Control Register
         *
         * TASSEL_2 -- SMCLK
         * ID_3 -- Divider - /8
         * MC_1 -- Up Mode
         */
        TA0CTL = TASSEL_2 + ID_3 + MC_1;
}

/*
 *  ======== Timer1_A3_init ========
 *  Initialize MSP430 Timer1_A3 timer
 */
void Timer1_A3_initial(void)
{
    /*
         * TA1CCTL0, Capture/Compare Control Register 0
         *
         * CM_1 -- Rising Edge
         * CCIS_0 -- CCIxA
         * SCS -- Sychronous Capture
         * ~SCCI -- Latched capture signal (read)
         * CAP -- Capture mode
         * OUTMOD_0 -- PWM output mode: 0 - OUT bit value
         *
         * Note: ~SCCI indicates that SCCI has value zero
         */
        TA1CCTL0 = CM_1 + CCIS_0 + SCS + CAP + OUTMOD_0 + CCIE;

        /*
         * TA1CCTL1, Capture/Compare Control Register 1
         *
         * CM_2 -- Falling Edge
         * CCIS_0 -- CCIxA
         * SCS -- Sychronous Capture
         * ~SCCI -- Latched capture signal (read)
         * CAP -- Capture mode
         * OUTMOD_0 -- PWM output mode: 0 - OUT bit value
         *
         * Note: ~SCCI indicates that SCCI has value zero
         */
        TA1CCTL1 = CM_2 + CCIS_0 + SCS + CAP + OUTMOD_0 + CCIE;

        /*
         * TA1CCTL2, Capture/Compare Control Register 2
         *
         * CM_2 -- Falling Edge
         * CCIS_0 -- CCIxA
         * SCS -- Sychronous Capture
         * ~SCCI -- Latched capture signal (read)
         * CAP -- Capture mode
         * OUTMOD_0 -- PWM output mode: 0 - OUT bit value
         *
         * Note: ~SCCI indicates that SCCI has value zero
         */
        TA1CCTL2 = CM_2 + CCIS_0 + SCS + CAP + OUTMOD_0;

        /*
         * TA1CTL, Timer_A3 Control Register
         *
         * TASSEL_2 -- SMCLK
         * ID_3 -- Divider - /8
         * MC_2 -- Continuous Mode
         */
        TA1CTL = TASSEL_2 + ID_3 + MC_2;
}
