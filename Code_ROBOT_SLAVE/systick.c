/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "lm4f120h5qr.h"
#include "gpio.h"

/******************************************************************************
 * Defines
 *****************************************************************************/
#define PORTF   	0x40025000
#define LEFTFWD		0x18	
#define RIGHTFWD	0x04
#define LEFTBACK	0x10
#define RIGHTBACK	0x06

/******************************************************************************
 * Global Variables
 *****************************************************************************/
//  Initialize the duty cycle.  It has 6 stages. It can either be
//			0, 20, 40, 60, 80, or 100.  There are ONLY 5 stages where the wheels
//			are ACTUALLY spinning (20, 40, ..., 100)
volatile uint32_t MotorDutyCycle = 0;  // Y
volatile uint32_t MotorDutyCycle2 = 0; // X

//  This is the value to be loaded into GPIO Port F DATA register.
//			It determines the direction of the left and right wheels.
volatile uint32_t dataValue = 0;

extern int finalDistance0;
extern int finalDistance1;
extern int frontDistance; //Distance from Optical Sensor
// Distances from Ultrasonic Sensor0(Right) and Sensor1(Left)
extern int leftDistance;
extern int rightDistance;
volatile bool checkDistance = false;  // Every 1/20th of second check 																		
volatile bool checkDistance2 = false; // dist. After 20 cycles, one second.
volatile uint32_t distanceCount = 0;
volatile uint32_t distanceCount2 = 0;

volatile bool turn180 = false; // Is robot turning 180 degrees?
volatile bool turn90Left = false;
volatile bool turn90Right = false;

volatile bool stop = false;
volatile bool objectAhead = false;

extern int M0_SA;
extern int M0_SB;
extern int M1_SA;
extern int M1_SB;
extern uint8_t XANALOG;

extern bool recentered;
extern bool autonomous;
bool straight3Feet = false;

/******************************************************************************
 * External Functions
 *****************************************************************************/
extern void checkDutyCycle(uint32_t range, volatile uint32_t *dutyCycle);

//  Initialize port F, b/c this will be needed to changed the DATA values.
GPIO_PORT *myPortF = (GPIO_PORT *)PORTF;

/******************************************************************************
 * START OF FUNCTIONS
 *****************************************************************************/

void SYSTICKIntHandler(void) {
	
	// Count for if robot is turning. It is universal for turning left & right.
	static int turnCount = 0;
	//  Set the counter which is used for PWM'ing
	static int sysCounter = 0;
	static int drive10feet = 0;
	int temp;
	
	// Clear systick interrupt
  NVIC_ST_CURRENT_R = 0;
	
	// counts for every 5kHz
	distanceCount++;
	if(distanceCount > 250) {
		distanceCount = 0;
		checkDistance = true;
	}
	// counts for every 5kHz
	distanceCount2++;
	if(distanceCount2 > 250) {
		distanceCount2 = 0;
		checkDistance2 = true;
	}
	// If robot is within specified distance, don't let it go forward anymore.
	// Should be within 20cm of object, but it needs some time to process the distance.
	if(frontDistance <= 20 && frontDistance != -1) objectAhead = true; // 25cm
	else if(leftDistance <= 10 && leftDistance != -1) objectAhead = true; // 10 inches
	else if(rightDistance <= 10 && rightDistance != -1) objectAhead = true; // 10 inches
	else objectAhead = false;
	
	// If robot is within 20cm of an object, turn around.
	if(!stop) {
		
		if(autonomous && !objectAhead) {
			
			// Reset sensors so we can now measure out 10 feet with them.
			if(drive10feet == 0) { M0_SA = 0; M1_SA = 0; }
			drive10feet++;
			dataValue = (LEFTFWD | RIGHTFWD);
			if(M0_SA < 800) {
				
				//  Check if counter is greater than the duty cycle
				MotorDutyCycle = 80;
				
				if(sysCounter > MotorDutyCycle) { myPortF->Data = 0x0; } 
				else { // Motor0 is on Right, Motor1 is on Left.
					
					// Do not turn the right wheel.
					if((M0_SA) > (M1_SA)){ myPortF->Data = (dataValue & (~RIGHTFWD | ~RIGHTBACK)); }
					else if((M0_SA) < (M1_SA)){ myPortF->Data = (dataValue & (~LEFTFWD | ~LEFTBACK)); }
					else{ myPortF->Data  = dataValue; }

				}
				
			} else { 
				
				drive10feet = 0; 
				stop = true;
				
			}
			
		} else if(turn180) { // Start turning process
			
			MotorDutyCycle = 100;
			myPortF->Data = (LEFTBACK | RIGHTFWD);
			
			if(!turn180) {
				
				myPortF->Data = (LEFTBACK | RIGHTFWD);
				turn180 = true;
				MotorDutyCycle = 100;
			// It is completing its 180 degree turn.
			} else { 
				
				if(turnCount == 2500) { 
					
					turn180 = false;
					turnCount = 0;
					dataValue = 0x0;
					// Reset Motor sensors
					M0_SA = 0;
					M1_SA = 0;
					
				} else { turnCount++; }
				
			}
		// Completing turn 90 degrees left. NEED TO MAKE SURE IT DOESN'T GET CAUGHT IN PREV. IF STMT
		// Actually turning right
		} else if (turn90Right) {
				
				M0_SA = 0;
				M1_SA = 0;
				// start turning process.
				checkDutyCycle(XANALOG-48, &MotorDutyCycle2);
				if(sysCounter > MotorDutyCycle2) { myPortF->Data = 0x0; }
				else{ myPortF->Data = (RIGHTFWD | LEFTBACK); }
		// Actually turning left
		} else if (turn90Left) {
				
				M0_SA = 0;
				M1_SA = 0;
				// start turning process.
				checkDutyCycle(XANALOG-48, &MotorDutyCycle2);
				if(sysCounter > MotorDutyCycle2){ myPortF->Data = 0x0; }
				else{ myPortF->Data = (LEFTFWD | RIGHTBACK); }
				
		} else if(!objectAhead) {
			// Regular motion
			
				static int feetCount = 0;
			
				// check for if go straight 3 feet.
				if(feetCount == 10000) {
					feetCount = false;
					feetCount = 0;
					stop = true; // stop the robot.
				} else if(straight3Feet){ feetCount++; }
			
				//  Check if counter is greater than the duty cycle
				if(sysCounter > MotorDutyCycle){ myPortF->Data = 0x0; }
				else { // Motor0 is on Right, Motor1 is on Left.
					
					// Do not turn the right wheel.
					if((M0_SA) > (M1_SA)){ myPortF->Data = (dataValue & (~RIGHTFWD | ~RIGHTBACK)); }
					else if((M0_SA) < (M1_SA)){ myPortF->Data = (dataValue & (~LEFTFWD | ~LEFTBACK)); }
					else{ myPortF->Data  = dataValue; }

				}
			// There is an object ahead, and we cannot continue regular motion.	
		} else { myPortF->Data = 0x0; }
		
		// If stop is TRUE, then the motors are turned off.
	} else { myPortF->Data = 0x0; }
	

	if(sysCounter == 100) { sysCounter = 0; } //reset
	else { sysCounter++; } //otherwise, increment
	
	temp = NVIC_ST_CURRENT_R;
	
}

void initializeSysTick(uint32_t count, bool enableInterrupts)
{
  uint32_t  savedVal = NVIC_ST_CTRL_R;
  NVIC_ST_CTRL_R     = 0;            // disable SysTick timer
  NVIC_ST_RELOAD_R   = count - 1;    // Set reload to count-1
  NVIC_ST_CURRENT_R = 0;            // clear the current count
	NVIC_ST_CTRL_R     |= (NVIC_ST_CTRL_ENABLE | NVIC_ST_CTRL_CLK_SRC);
  
  // Enable Interrupts
	if(enableInterrupts) { NVIC_ST_CTRL_R     |= 0x02; }	
}
