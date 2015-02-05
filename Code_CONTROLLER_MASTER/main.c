//***************************************************************************//
// Bob Wagner																																 //
// Calvin Hareng																														 //
// ECE 315																																	 //
// FINAL PROJECT																														 //
//***************************************************************************//

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "lm4f120h5qr.h"
#include "board_config.h"

/******************************************************************************
 * Defines
 *****************************************************************************/
#define PORTA   0x40004000
#define PORTB   0x40005000
#define PORTC   0x40006000
#define PORTD   0x40007000
#define PORTE   0x40024000
#define PORTF   0x40025000

#define LEFTFWD		0x18	
#define RIGHTFWD	0x04
#define LEFTBACK	0x10
#define RIGHTBACK	0x06

#define YPOS 10
#define XPOS 11

/******************************************************************************
 * Global Variables
 *****************************************************************************/
//  Determines the speed of the robot
extern uint32_t MotorDutyCycle;
extern uint32_t MotorDutyCycle2;

//  Determines the direction of the robot by setting data register for motors.
extern uint32_t dataValue;

extern bool checkDistance;
extern bool opticalCheck;
volatile bool autonomousMode = false;

volatile int finalDistance0 = -1;
volatile int finalDistance1 = -1;
volatile uint8_t yRange; //ranges: 0-4 are negative, 5 neutral
volatile uint8_t xRange; //				 6-10 positive

GPIO_PORT *GPIOPortB = (GPIO_PORT *)PORTB;

/******************************************************************************
 * External Functions
 *****************************************************************************/
extern void PLL_Init(void);
extern bool InitializeUART(uint32_t base, UART_CONFIG *init);
extern bool initializeGPIOPort( uint32_t base, GPIO_CONFIG *init);
extern void initializeSysTick(uint32_t count, bool enableInterrupts);
extern void initializeTimer0(uint32_t count);
extern void initializeTimer1(uint32_t count);

extern char uartRxPoll(uint32_t base);
extern void uartTxPoll(uint32_t base, char *data);
extern void checkUltrasonicSensor0(void);
extern void checkUltrasonicSensor1(void);
extern void checkOpticalSensor(void);
extern void configXBEE(void);
extern bool initUART(uint32_t base);

extern void sendData(bool);

/******************************************************************************
 * START OF CODE
 *****************************************************************************/

uint32_t GetADCval(uint32_t Channel)
{
  uint32_t result;

  ADC0_SSMUX3_R = Channel;      		// Set the channel
  ADC0_ACTSS_R  |= ADC_ACTSS_ASEN3; // Enable SS3
  ADC0_PSSI_R = ADC_PSSI_SS3;     	// Initiate SS3

  while(0 == (ADC0_RIS_R & ADC_RIS_INR3)); // Wait for END of conversion
  result = ADC0_SSFIFO3_R & 0x0FFF;     	 // Read the 12-bit ADC result
  ADC0_ISC_R = ADC_ISC_IN3;         			 // Acknowledge completion

  return result;
}

uint32_t GetADC1val(uint32_t channel){

 uint32_t result;

 ADC1_SSMUX3_R = channel;      	 // Set the channel
 ADC1_PSSI_R = ADC_PSSI_SS3;     // Initiate SS3

 while(0 == (ADC1_RIS_R & ADC_RIS_INR3)); // Wait for END of conversion
 result = ADC1_SSFIFO3_R & 0xffff;     		// Read the 12-bit ADC result
 ADC1_ISC_R = ADC_ISC_IN3;         				// Acknowledge completion
return result;
}

void initializeADC(void)
{
	int i;

  SYSCTL_RCGCADC_R |= SYSCTL_RCGCADC_R1; // Enable ADC 
	// Need to wait a while before the UART is functional
  while(i < 10000) { i++; }

	i = 0;
	
	SYSCTL_RCGC0_R |= 0x00010000;
	SYSCTL_RCGC0_R &= ~0x00000300;
	ADC0_SSPRI_R = 0x3210;
	ADC0_ACTSS_R &= ~0x0008;
	ADC0_EMUX_R &= ~0xF000;
	ADC0_SSCTL3_R = 0x0006;
	ADC0_IM_R &= ~0x0008;
	ADC0_ACTSS_R |= 0x0008;

}

void initializeADC1(int mode){
// Enable ADC1
	
	int i, delay;
	
	SYSCTL_RCGCADC_R |= SYSCTL_RCGCADC_R1;
	for(i = 0; i < 100; i++); //wait
	delay = 0;
	
	SYSCTL_RCGCADC_R &= ~SYSCTL_RCGC0_ADC1SPD_M; //set sample speed 125K
	ADC1_SSPRI_R = ADC_SSPRI_SS3_4TH | ADC_SSPRI_SS2_3RD | ADC_SSPRI_SS1_2ND | ADC_SSPRI_SS0_1ST; //set priority
	ADC1_ACTSS_R &= ~ADC_ACTSS_ASEN3;
	ADC1_EMUX_R &= ~ADC_EMUX_EM3_M;
	ADC1_SSMUX3_R &= ~ADC_SSMUX3_MUX0_M;
	ADC1_SSMUX3_R += mode; //set channel to CHANNEL 3 for optical, CHANNEL 11 for analog x
	ADC1_SSCTL3_R = 0x006;
	ADC1_IM_R &= ~ADC_IM_MASK3;
	ADC1_ACTSS_R |= ADC_ACTSS_ASEN3;
	
}

bool  gpioPortInit(uint32_t baseAddress, uint8_t digitalEnableMask, 
                   uint8_t inputMask, uint8_t pullUpMask)
{
  
	uint32_t delay;
  GPIO_PORT *myPort = (GPIO_PORT *)baseAddress;
  
  // Validate that a correct base address has been passed
  // Turn on the Clock Gating Register
  switch (baseAddress) 
  {
    case PORTA :
      SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R0;
      break;
    case PORTB :
      SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1;
      break;
    case PORTC :
      SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R2;
      break;
    case PORTD :
      SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R3;
      break;
    case PORTE :
      SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4;
      break;
    case PORTF :
      SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;
      break;
    default:
      return false;
  }

  // Delay a bit
  delay = SYSCTL_RCGCGPIO_R;
	
  // Set the Direction Register
	myPort->Direction                   |= inputMask;
  
  // Enable Pull-Up Resistors
  myPort->PullUpSelect                |= pullUpMask;
	
		// Enable Pins as Digital Pins
  myPort->DigitalEnable               |= digitalEnableMask;
  
  // Disable the Alternate Function Select
  myPort->AlternateFunctionSelect      = 0;
	
	if(baseAddress == PORTE){
		myPort->AlternateFunctionSelect = 0x30; // PE4, PE5
		myPort->PortControl |= (GPIO_PCTL_PE4_U5RX | GPIO_PCTL_PE5_U5TX);
	}
	if(baseAddress == PORTA){
		myPort->AlternateFunctionSelect = 0x03; // PA1, PA0
		myPort->PortControl |= (GPIO_PCTL_PA0_U0RX | GPIO_PCTL_PA1_U0TX);
	}
	if(baseAddress == PORTB){
			myPort->AnalogSelectMode |=  0x30;
	}
  
  return true;
	
}

// Sets the duty cycle of the motors
void checkDutyCycle(uint32_t range, uint32_t* dutyCycle) {
	
		if(range == 5){ *dutyCycle = 0; }
		else if(range == 4 || range == 6){ *dutyCycle = 20; }
		else if(range == 3 || range == 7){ *dutyCycle = 40; }
		else if(range == 2 || range == 8){ *dutyCycle = 60; }
		else if(range == 1 || range == 9){ *dutyCycle = 80; }
		else{ *dutyCycle = 100; }

}

// Sets the DATA register for port F, which determines direction of motors.
void setDirection(int xVal, int yVal) {
	
		int POS = 1900;	
		dataValue = 0;
		
	//  Checks if both motors should be moving FWD or BACKWARD
	//			(Robot is moving in straight path)
	if (xVal > POS && xVal < 2150) {
		
			if (yVal > 2100){
				//  Both should be moving forward
				//  Insert code for 
				dataValue = (RIGHTFWD | LEFTFWD);
			} else {
				//  Both should be moving backwards.
				dataValue = (RIGHTBACK | LEFTBACK);
			}
	}
	//  Robot is turning.  To turn, one motor must move one direction
	//			as the other motor moves in the opposite direction.
	else {
		
		if(yVal > POS && xVal > POS) { // both positive
			dataValue = (LEFTFWD | RIGHTBACK); // Using LEFTFWD makes robot go right
		}
		else if (yVal > POS && xVal < POS) { //y+,x-
			dataValue = (RIGHTFWD | LEFTBACK); // Using RIGHTFWD makes robot go left
		}
		else if (yVal < POS && xVal > POS) { //y-,x+
			dataValue = (RIGHTFWD | LEFTBACK); 
		}
		else if (yVal < POS && xVal < POS) { //y-,x-
			dataValue = (LEFTFWD | RIGHTBACK);
		}
		
	}
		
}

bool debounceButton(void) {

	int pbVal;
	int counter;
	bool allOnes = true;
	bool allZeros = true;
	
	for(counter = 0; counter < 100; counter++) {
			
			pbVal = GPIOPortB->Data & PIN_1;
		
			if(pbVal == PIN_1) { allZeros = false; }
			if(pbVal == 0) { allOnes = false; }
		
	}
	
	// Return true (stabilized) if 100 values are consistantly one or zero
	if(allOnes || allZeros) { return true; }
	else { return false; }
	
}

/******************************************************************************
 * START OF MAIN
 *****************************************************************************/

int main(void){  
	
	volatile unsigned long delay;
	uint32_t i = 0; //  Used for wait loops
	int buttonVal; //value of ps2 pushbutton

  // Initialize the PLLs so the the main CPU frequency is 80MHz
  PLL_Init();
	
  initializeGPIOPort(PORTA, &portA_config);
	initUART(UART0);
	
	while(i < 1000)	// Need to wait a while before the UART is functional
  { i++; }
	i = 0;
	
	uartTxPoll(UART0, "=============================\n\r");
  uartTxPoll(UART0, "          ECE315 Lab\n\r");
  uartTxPoll(UART0, "=============================\n\r");

	// Need to wait a while before the UART is functional
	while(i < 1000)	{ i++; }
	i = 0;
	
	// Initialize PA0 and PA1 for UART0
	gpioPortInit( 
                    PORTA, // port PREV:0x33
                    0x03,  // digitalEnableMask
                    0x02,  // inputMask, (Direction) Tx is output?
                    0x03   //  pullUpMask
              );
	
	//  Intialize PORT B for Pins 0,1,2,3,4,5,6,and 7 (TRIG_0, PS2 pushbutton, Motor_0_SA/SB,XPOS,YPOS, and Motor_1_SA/SB).
	gpioPortInit( 
                    PORTB, // port PREV:0x33
                    0xFF,  // digitalEnableMask
                    0x01,  // inputMask, (Direction)
                    0xFF   //  pullUpMask
              );
	
	//  Initialize PORT E correctly for Pins 0, 1,2,3 (OPTICAL, ECHO_0, TRIG_1, ECHO_1).
	//			Added in PE4, PE5 for UART5
	gpioPortInit( 
                    PORTE, 
                    0x3F, 
                    0x24,
                    0x3F
              );
	
	//  Initialize PORT F correctly for Pins 1,2,3, and 4 (Motor 1 and 2 DIR and EN).
	gpioPortInit( 
                    PORTF, 
                    0x1E, 
                    0x1E,
                    0x1E
              );
							
	initUART(UART5); // Corresponds to Port F
	
	// Need to wait a while before the UART is functional
	while(i < 1000) { i++; }
	i = 0;
	
	/* INITIALIZATIONS */
	initializeSysTick(16000, true); //  Inititalize the systick to interrupt every 5KHz
	initializeADC(); // Uses our initialization routine
	initializeADC1(11); //master, configure for channel x
	//configXBEE(); //don't need to do this every time, unless we are using a different robot
				
	while(1) {
		
		//RANGE: 0x000 - 0xfff
		//dec:	 0 - 4096
		//midpoint:  2048
		//ranges:
		//NEGATIVE
		//0-10%: 0-373	 				0
		//10-20%:	 373-746	 		1
		//20-30%: 746-1119	 		2
		//30-40%:	 1119-1492	 	3
		//40-50%: 1492-1865	 		4
		//MIDDLE:	 1865-2238	 	5
		//POSITIVE	
		//50-60%:	 2238-2611	 	 		 6
		//60-70%:  2611-2984	 			 7
		//70-80%:	 2984-3357			 	 8
		//80-90%:  3357-3730	 			 9
		//90-100%:	 3730-4096	 		10
		//LEFTBACK, LEFTFWD, RIGHTBACK, RIGHTFWD
		// up and to the right are positive
		
		int yVal; //actual ADC values
		int xVal; //actual ADC values
		
		// used in for() loop to find values of X and Y, from 0-10, of PS2 joystick.
		int POS; 
		int NEG;

		// Grab the Digital value from the ADC conversion.
		yVal = GetADCval(YPOS);
		xVal = GetADCval(XPOS);
		
		POS = 373;
		NEG = 0;
		// For loop to determine x and y value of PS2 joystick.
		// Values range from 0-10 with 5 being the middle stage for x & y values.
		for(i = 0; i < 11; i++) {
			
			if(yVal < POS && yVal > NEG){ yRange = i; }
			if(xVal < POS && xVal > NEG){ xRange = i; }

			NEG = NEG + 373;
			POS = POS + 373;
			
	}
	
	//if ps2 pushbutton pressed, enter autonomous mode
	//need array of 1's to effectively debounce pushbutton press/release
	buttonVal = GPIOPortB->Data & PIN_1;
	
	if(debounceButton()) { //determine if it stable (not fluctuation in voltage)

		if(buttonVal == PIN_1) { //button is stable and pressed
			autonomousMode = false;
			uartTxPoll(UART0, "<< Autonomous Mode Entered >>\n\r");
		} else { //button is stable and unpressed
			autonomousMode = true;
			uartTxPoll(UART0, "<< Autonomous Mode Exited >>\n\r");
		}	

	}
		// send data values out to slave (xRange & yRange OR autonomous char).
		sendData(true);
	
	}
	
}

