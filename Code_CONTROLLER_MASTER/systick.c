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
volatile uint32_t MotorDutyCycle = 100;
volatile uint32_t MotorDutyCycle2 = 100;

//  This is the value to be loaded into GPIO Port F DATA register.
//			It determines the direction of the left and right wheels.
volatile uint32_t dataValue = (LEFTFWD | RIGHTFWD);

extern int frontDistance;//Distance from Optical Sensor
//Distances from Ultrasonic Sensor0(Right) and Sensor1(Left)
extern int leftDistance;
extern int rightDistance;
volatile bool checkDistance = false;  // Every 1/20th of second check 																		
volatile bool checkDistance2 = false; // dist. After 20 cycles, one second.
volatile uint32_t distanceCount = 0;
volatile uint32_t distanceCount2 = 0;

volatile extern uint8_t XANALOG;
volatile extern uint8_t YANALOG;

// Is robot turning 180 degrees?
bool turn180 = false;
bool turn90Left = false;
bool turn90Right = false;

bool straight3Feet = false;
bool stop = false;

//  Initialize port F, b/c this will be needed to changed the DATA values.
GPIO_PORT *myPortF = (GPIO_PORT *)PORTF;

/******************************************************************************
 * START OF FUNCTIONS
 *****************************************************************************/

void SYSTICKIntHandler(void)
{
	
	//  Set the counter which is used for PWM'ing
	static int sysCounter = 0;	
	int temp;
	
	// Clear systick interrupt
  NVIC_ST_CURRENT_R = 0;
	
	// sysCounter goes up to 100 b/c it's used for
		// PWM'ing the data signals driving the motors.
		// If the duty cycle is less than the current sysCounter value,
		// then the motors will be off, otherwise they will be on.
	if(sysCounter == 100){ sysCounter = 0; }
	else{ sysCounter++; }

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
