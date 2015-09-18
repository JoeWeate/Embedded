# Embedded
Programming done on a MSP430; Various projects


Electronic level- uses a gyroscope to and 8 LEDs to determine when the msp430 is level on three axises. Based off the tilt of the MSP430, the LEDs will light towards the side that needs to be raised.  
This lab also uses Pulse Width Modulation to light the LEDs that don't need to be raised as high less.  

PWW-Pulse Width Modulation- uses a green and red LED.  While one turns on slowly, the other is turning off slowly.  When a button is pressed (interrupt), the pulse rate changes.  Pressing the button again reverts the pulse rate.  

Switch Debouncing- When a button is pressed, there is bouncing that occurs that causes miscalcualtions.  This code abolishes that error.  Used in every project that utilizes the button to allow for accurate results.  

Motor Controller- The msp430 is programmed to run a motor n times clockwise, and n times counterclockwise, autocorrecting the algorithm to fix for errors that could occur in the turns.

MPE and WC- manchester Phase encoding and Wireless Control.  Sends data wirelessly to another MSP430 embedded system.  The codes that are sent are encoding using manchester phase encoding.  If the encoding scheme is not followed, the message is thrown out.  
Final- When a rotary encoder is turned one way, a motor is moved in the same direction through wirelessly sending data.  Simulates a remote control car.  

