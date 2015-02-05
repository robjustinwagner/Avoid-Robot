/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "lm4f120h5qr.h"
#include "gpio.h"
#include "uart.h"

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

/******************************************************************************
 * Global Variables
 *****************************************************************************/

extern int finalDistance0;
extern int finalDistance1;
volatile int frontDistance = -1;
volatile int leftDistance = -1;
volatile int rightDistance = -1;

extern bool opticalCheck;

extern int timeCounter;

/******************************************************************************
 * External Functions
 *****************************************************************************/
extern char uartRxPoll(uint32_t base);
extern void uartTxPoll(uint32_t base, char *data);

extern uint32_t GetADC1val(uint32_t channel);

/******************************************************************************
 * START OF FUNCTIONS
 *****************************************************************************/

void checkUltrasonicSensor0(){

	uint32_t static avgDistanceU0 = 0;
	uint32_t static avgCountU0 = 0;
	int i;
	
	if(avgCountU0 < 20) {
		
		for(i = 0; i < 1000; i++){}
		avgDistanceU0 += finalDistance0;
		// Set global variable for front distance (inches).
		rightDistance = (int) (finalDistance0/2540);
		avgCountU0++;
		
	}
	// After 20 cycles have occurred, that is one second.
	else {
		
		avgDistanceU0 = avgDistanceU0 / 20; //take average
		avgCountU0 = 0; //reset count
		// Set global variable for front distance (inches).
		rightDistance = (int)(avgDistanceU0/2540);
		
	}
	
	finalDistance0 = -1;
	
}

void checkUltrasonicSensor1() {
	
	
	uint32_t static avgDistanceU1 = 0;
	uint32_t static avgCountU1 = 0;
	int i;
	
	if(avgCountU1 < 20) {
		
		for(i = 0; i < 1000; i++){}
		avgDistanceU1 += finalDistance1;
		// Set global variable for front distance (inches).
		leftDistance = (int) (finalDistance1/2540);
		avgCountU1++;
			
	}
	// After 20 cycles have occurred, that is one second.
	else {
		
		avgDistanceU1 = avgDistanceU1 / 20; //take average
		avgCountU1 = 0; //reset count
		// Set global variable for front distance (inches).
		leftDistance = (int)(avgDistanceU1/2540);
		
	}
	
	finalDistance1 = -1;
	
}

void checkOpticalSensor() {	


	uint32_t static avgDistanceOpt = 0;

	opticalCheck = false;
	//linear regression model we calculated to approximate curve in data sheet
	avgDistanceOpt = (int) (((4*GetADC1val(3))+(-12713))*(-2)/(683));
	frontDistance = avgDistanceOpt;

}
