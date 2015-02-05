/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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
 
#define PIN_0     (1 << 0)
#define PIN_1     (1 << 1)
#define PIN_2     (1 << 2)
#define PIN_3     (1 << 3)
#define PIN_4     (1 << 4)
#define PIN_5     (1 << 5)
#define PIN_6     (1 << 6)
#define PIN_7     (1 << 7)

/******************************************************************************
 * Global Variables
 *****************************************************************************/
volatile bool opticalCheck;
volatile int timeCounter = 0;
extern int finalDistance0;
extern int finalDistance1;

GPIO_PORT *GpioPortB = (GPIO_PORT *)PORTB;
GPIO_PORT *GpioPortE = (GPIO_PORT *)PORTE;
 
/******************************************************************************
 * START OF FUNCTIONS
 *****************************************************************************/
 
 void Timer0Handler() {
	 
	static bool requested0 = false;
	static bool requested1 = false;
	static int count0 = -0;
	static int count1 = 0;
	 
	// increment counter each time.
	if(timeCounter > 1) { timeCounter++; }
	
	// FIRST 10uS has passed, set TRIG_0, TRIG_1 = 0, increment counter for first time.
	// Counter * 10uS tells how many timer cycles have occurred.
	else if(timeCounter == 1) {
		GpioPortB->Data &= ~PIN_0;
		GpioPortE->Data &= ~PIN_2;
		timeCounter++;
	}
	// Needs to come after previous statement, otherwise counter never gets incremented.
	else if(timeCounter == 0) {
		GpioPortB->Data |= PIN_0; // Change to a 1 for 10uS.
		GpioPortE->Data |= PIN_2;
		timeCounter++;
	}

	// Counter has reached 50ms.
	if(timeCounter > 5000){ timeCounter = 0; } //reset

	if((GpioPortB->Data & PIN_0) && !(GpioPortE->Data & PIN_1))  { //trig = 1, echo = 0
		requested0 = true; //data was requested
		count0 = 0;
	}
	if((GpioPortE->Data & PIN_1) && requested0) { //echo = 1, data was requested
		count0++;
	}
	if(!(GpioPortE->Data & PIN_1) && requested0 && count0 > 1) { //echo = 0, data requested
		finalDistance0 = count0 * 170; //multiply high time by velocity of speed (340 M/s) / 2 = 170 M/s)
		count0 = 0;
		requested0 = false; //set data requested to false
	}
	if((GpioPortB->Data & PIN_0) && (GpioPortE->Data & PIN_1)) { //bad case
		requested0 = false;
	}

	if((GpioPortE->Data & PIN_2) && !(GpioPortE->Data & PIN_3))  { //trig = 1, echo = 0
		requested1 = true; //data was requested
	}
	if((GpioPortE->Data & PIN_3) && requested1) { //echo = 1, data was requested
		count1++;
	}
	if(!(GpioPortE->Data & PIN_3) && requested1 && count1 > 1) { //echo = 0, data requested
		finalDistance1 = count1 * 170; //multiply high time by velocity of speed (340 M/s) / 2 = 170 M/s)
		count1 = 0;
		requested1 = false; //set data requested to false
	}
	if((GpioPortE->Data & PIN_2) && (GpioPortE->Data & PIN_3)) { //bad case
		requested1 = false;
	}

	TIMER0_ICR_R = 0x01; //reload timer value

}

void Timer1Handler() {

	opticalCheck = true;
	
	TIMER1_ICR_R = 0x01; //reload timer value

}


//Init timer 0
void initializeTimer0(uint32_t count)
{
	uint32_t wait;
	SYSCTL_RCGCTIMER_R   |=   SYSCTL_RCGCTIMER_R0; // Turn on the clock to timer0
	wait = SYSCTL_RCGC1_R;

	TIMER0_CTL_R    &=  ~TIMER_CTL_TAEN;           // Disable the timer while we are configuring it.
  TIMER0_CFG_R    =   TIMER_CFG_32_BIT_TIMER;    // Configure for 32-bit Mode
  TIMER0_TAMR_R   =   TIMER_TAMR_TAMR_PERIOD;    // Put it into periodic mode
  TIMER0_TAILR_R  =   count;                  	 // Load the count value
	TIMER0_IMR_R    |=  TIMER_IMR_TATOIM;          // Enable Interrupt
	NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF) | 0x40000000; //Priority 2
	NVIC_EN0_R |= 	NVIC_EN0_INT19;								 //Enable timer1A interrupt (and 0B)
  TIMER0_CTL_R    |=  TIMER_CTL_TAEN;            // Turn the timer on

}

//Init timer 1
void initializeTimer1(uint32_t count)
{
	uint32_t wait;
	SYSCTL_RCGCTIMER_R   |=   SYSCTL_RCGCTIMER_R1; // Turn on the clock to timer0
	wait = SYSCTL_RCGC1_R;

	TIMER1_CTL_R    &=  ~TIMER_CTL_TAEN;           // Disable the timer while we are configuring it.
  TIMER1_CFG_R    =   TIMER_CFG_32_BIT_TIMER;    // Configure for 32-bit Mode
  TIMER1_TAMR_R   =   TIMER_TAMR_TAMR_PERIOD;    // Put it into periodic mode
  TIMER1_TAILR_R  =   count;                  	 // Load the count value
	TIMER1_IMR_R    |=  TIMER_IMR_TATOIM;          // Enable Interrupt
	NVIC_PRI4_R 		= (NVIC_PRI4_R&0x00FFFFFF) | 0x40000000; //Priority 2
	NVIC_EN0_R 			|= NVIC_EN0_INT21 | NVIC_EN0_INT22; 		 //Enable timer0A interrupt (and 1B)
  TIMER1_CTL_R    |=  TIMER_CTL_TAEN;            // Turn the timer on
}
