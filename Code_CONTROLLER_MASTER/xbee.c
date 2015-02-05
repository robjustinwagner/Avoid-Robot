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
extern bool autonomousMode;
extern uint8_t xRange;
extern uint8_t yRange;

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

	uartTxPoll(UART0, "\n< START XBEE CONFIG >\n");

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
	uartTxPoll(UART5, "ATDL=2305\n\r");
	tester[0] = uartRxPoll(UART5, true);
	tester[1] = uartRxPoll(UART5, true);
	tester[2] = uartRxPoll(UART5, true);
	uartTxPoll(UART0, tester);
	uartTxPoll(UART0, "\n");

	// Setting the Destination ID
	uartTxPoll(UART5, "ATMY=2306\n\r");
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

	// Setting as a digital output (LOW)
	uartTxPoll(UART5, "ATD2=4\n\r");
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
	
}

void sendData(bool change) {
	
	char temp [1];
	char temp2 [1];
	int i;
	
	/* LEGEND */
	////////////////////////////////////////////////////
	//48 corresponds to a level of 0 (neg)						//
	//53 corresponds to a level of 5 (middle/neutral)	//
	//58 corresponds to a level of 10 (pos)						//
	//65 is 'A' which is Autonomous Mode.							//
	////////////////////////////////////////////////////
	
	if(change) {
		
		temp[0] = (char)(xRange + 48); //xRange/yRange is actual level,
		temp2[0] = (char)(yRange + 48); //need to offset by 48 to transmit as char
		
	}
	
	if(autonomousMode) {
		temp[0] = (char)65;	//65 corresponds to 'A'
		uartTxPoll(UART5, temp); // send 'A' to signify we're in autonomousMode
		for(i = 0; i <1000; i++){} //wait
		uartTxPoll(UART5, temp); // xval
	} else {
		uartTxPoll(UART5, temp2); // yval
		for(i = 0; i <1000; i++){} //wait
		uartTxPoll(UART5, temp); // xval
	}

}
