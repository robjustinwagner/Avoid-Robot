/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "lm4f120h5qr.h"

/******************************************************************************
 * Defines
 *****************************************************************************/
#define	UART0 0x4000C000
#define UART1 0x4000D000
#define UART2 0x4000E000
#define UART3 0x4000F000
#define UART4 0x40010000
#define UART5 0x40011000
#define UART6 0x40012000
#define UART7 0x40013000

/******************************************************************************
 * Global Variables
 *****************************************************************************/
volatile bool recentered;
volatile bool autonomous = false;
volatile uint8_t XANALOG;
volatile uint8_t YANALOG;
extern bool turn180;
extern bool turn90Left;
extern bool turn90Right;
extern bool stop;

/******************************************************************************
 * External Functions
 *****************************************************************************/
extern char uartRxPoll(uint32_t base, bool block);
extern void uartTxPoll(uint32_t base, char *data);

/******************************************************************************
 * START OF CODE
 *****************************************************************************/

void configXBEE() {
	
	// used as a char array to configure and test the Xbee.
	char tester[50];

	uartTxPoll(UART0, "\n< START XBEE CONFIG >\n\r");

	// Used to see if XBee is speaking.
	uartTxPoll(UART5, "+++");
	// Tester reads in the response to "+++", it should
	// read in the two chars "OK" followed by a NULL
	// character to terminate the String.
	tester[0] = uartRxPoll(UART5, true);
	tester[1] = uartRxPoll(UART5, true);
	tester[2] = uartRxPoll(UART5, true);
	// Print the received message on UART0 to validate that
	// UART5 is working.
	uartTxPoll(UART0, tester);
	uartTxPoll(UART0, "\n");

	// Setting the Channel
	uartTxPoll(UART5, "ATCH=17\n\r");
	tester[0] = uartRxPoll(UART5, true);
	tester[1] = uartRxPoll(UART5, true);
	tester[2] = uartRxPoll(UART5, true);
	uartTxPoll(UART0, tester);
	uartTxPoll(UART0, "\n");

	// Setting the Personal Area Network ID
	uartTxPoll(UART5, "ATID=7777\n\r");
	tester[0] = uartRxPoll(UART5, true);
	tester[1] = uartRxPoll(UART5, true);
	tester[2] = uartRxPoll(UART5, true);
	uartTxPoll(UART0, tester);
	uartTxPoll(UART0, "\n");

	// Setting the Source ID
	uartTxPoll(UART5, "ATDL=2306\n\r");
	tester[0] = uartRxPoll(UART5, true);
	tester[1] = uartRxPoll(UART5, true);
	tester[2] = uartRxPoll(UART5, true);
	uartTxPoll(UART0, tester);
	uartTxPoll(UART0, "\n");

	// Setting the Destination ID
	uartTxPoll(UART5, "ATMY=2305\n\r");
	tester[0] = uartRxPoll(UART5, true);
	tester[1] = uartRxPoll(UART5, true);
	tester[2] = uartRxPoll(UART5, true);
	uartTxPoll(UART0, tester);
	uartTxPoll(UART0, "\n");

	// Saves the Configuration
	uartTxPoll(UART5, "ATWR\n\r");
	tester[0] = uartRxPoll(UART5, true);
	tester[1] = uartRxPoll(UART5, true);
	tester[2] = uartRxPoll(UART5, true);
	uartTxPoll(UART0, tester);
	uartTxPoll(UART0, "\n");

	// Setting as a digital Input
	uartTxPoll(UART5, "ATD2=3\n\r");
	tester[0] = uartRxPoll(UART5, true);
	tester[1] = uartRxPoll(UART5, true);
	tester[2] = uartRxPoll(UART5, true);
	uartTxPoll(UART0, tester);
	uartTxPoll(UART0, "\n");

	// Saves the Configuration
	uartTxPoll(UART5, "ATWR\n\r");
	tester[0] = uartRxPoll(UART5, true);
	tester[1] = uartRxPoll(UART5, true);
	tester[2] = uartRxPoll(UART5, true);
	uartTxPoll(UART0, tester);
	uartTxPoll(UART0, "\n");

	uartTxPoll(UART0, "< XBEE CONFIGURED >\n\r");

}

void checkUART() {
	
	uint8_t tempY;
	uint8_t tempX;
	
	tempY = uartRxPoll(UART5, true);
	tempX = uartRxPoll(UART5, true);
	
	/* LEGEND */
	////////////////////////////////////////////////////
	//48 corresponds to a level of 0 (neg)						//
	//53 corresponds to a level of 5 (middle/neutral)	//
	//58 corresponds to a level of 10 (pos)						//												//
	//65 is 'A' which is Autonomous Mode.							//
	////////////////////////////////////////////////////
	
	if(tempY == 65 || tempX == 65) {
		
		if(autonomous) {
			stop = false;
			autonomous = false;
			while(tempY == 65 || tempX == 65) {
				tempY = uartRxPoll(UART5, true);
				tempX = uartRxPoll(UART5, true);
			}
		} else {
			autonomous = true;
			while(tempY == 65 || tempX == 65){
					tempY = uartRxPoll(UART5, true);
					tempX = uartRxPoll(UART5, true);
			}
		}
		
	} else {
		
		// Discard improper values.
		if(tempY < 66 && tempX < 66) { 
			YANALOG = tempY;
			XANALOG = tempX;
		} else {
			XANALOG = tempY;
			YANALOG = tempX;
		}
		
		if(YANALOG <= 2+48 && recentered) {
			turn180 = true;
			recentered = false;
		}
		if(YANALOG == 5+48 && XANALOG == 5+48) {
			recentered = true;
			turn90Left = false;
			turn90Right = false;
		}
		if(XANALOG > 5+48) { //L
			turn90Left = true;
			recentered = false;
		}
		if(XANALOG < 5+48) { //R
			turn90Right = true;
			recentered = false;
		}
	
	}
		
}
