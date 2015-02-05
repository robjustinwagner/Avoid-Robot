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

#define	UART0 0x4000C000

#define LEFTFWD		0x18	
#define RIGHTFWD	0x04
#define LEFTBACK	0x10
#define RIGHTBACK	0x06

/******************************************************************************
 * Global Variables
 *****************************************************************************/
volatile int M0_SA = 0;
volatile int M0_SB = 0;
volatile int M1_SA = 0;
volatile int M1_SB = 0;
 
/******************************************************************************
 * START OF FUNCTIONS
 *****************************************************************************/

GPIO_PORT *myPort = (GPIO_PORT *)PORTB;
char msg[40];
 
void Motor0Handler() {
	
	 if((myPort->RawInterruptStatus & PIN_2) == PIN_2){ M0_SA++; }
	 if((myPort->RawInterruptStatus & PIN_3) == PIN_3){ M0_SB++; }
	 if((myPort->RawInterruptStatus & PIN_6) == PIN_6){ M1_SA++; }
	 if((myPort->RawInterruptStatus & PIN_7) == PIN_7){ M1_SB++; }
	 
	 myPort->InterruptClear = 0xFF; // Clears all Interrupts

}
 
 void initMotor0Int() {
	 	 
	 NVIC_PRI0_R = (NVIC_PRI0_R & 0xFFFFFF1F)|(1 << 5);
	 
	 // Set to edge sensitive interrupts.
	 // Interrupt for Motor 0&1 Sensor A&B
	 myPort->InterruptSence &=  (~PIN_2)&(~PIN_3)&(~PIN_6)&(~PIN_7);
	 
	 myPort->InterruptMask |= (PIN_2)|(PIN_3)|(PIN_6)|(PIN_7);
	 
	 myPort->InterruptEvent |= (PIN_2)|(PIN_3)|(PIN_6)|(PIN_7);
	 
	 // Enable Interrupts.
	 NVIC_EN0_R |= NVIC_EN0_INT1;
	 
 }
 
