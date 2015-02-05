/******************************************************************************
 * Includes
 *****************************************************************************/
#include "uart.h"

int i;

/******************************************************************************
 * START OF CODE
 *****************************************************************************/

/*************************************************************************
 * Initializes a UART peripheral
 *
 * This function assumes the the corresponding GPIO ports have already
 * been configured for thier alternate functions and the port control
 * register were configured correctly.
 ************************************************************************/
bool  initUART(uint32_t base)
{
  uint32_t delay;
  UART_PERIPH *myPeriph = (UART_PERIPH *)base;
  
  // Validate that a correct base address has been passed
  // Turn on the Clock Gating Register
  switch (base) 
  {
    case UART0 :
			SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R0;
      break;
    case UART5 :
      SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R5;
      break;
    default:
      return false;
  }

  delay = SYSCTL_RCGC1_R;
  delay = SYSCTL_RCGCUART_R;

  // Disable UART
  myPeriph->UARTControl &= ~UART_CTL_UARTEN; 
  
  // BAUD Rate = 9600
  myPeriph->IntegerBaudRateDiv = 520; 
  myPeriph->FracBaudRateDiv = 53;

  // Configure UART Line control register for 8-n-1, no FIFOs
  myPeriph->LineControl = UART_LCRH_WLEN_8 | UART_LCRH_FEN; 

  // Disable Interrupts
  myPeriph->IntMask = 0;
  
  // Enable UART TX & RX
  myPeriph->UARTControl = UART_CTL_UARTEN | UART_CTL_TXE | UART_CTL_RXE ;

	//wait to ensure UART becomes available
	for(i=0; i<100; i++){}
	
  return true;
}


/****************************************************************************
 * This routine returns a character received from the UART.
 * If blocking is enabled, this routine should not return until data
 * is available. If blocking is disabled and no data is available,
 * this function should return 0.
 ****************************************************************************/
char uartRxPoll(uint32_t base, bool block)
{
  UART_PERIPH *myPeriph = (UART_PERIPH *)base;
  
  if (block)
  {
    while(myPeriph->Flag & UART_FR_RXFE)
    {
      // Wait
    }
     return myPeriph->Data;
  }
  else
  {
    if(myPeriph->Flag & UART_FR_RXFE)
      return 0;
    else
      return myPeriph->Data;
  }
}

/****************************************************************************
 * This routine transmits a character string out the UART 
 * Only the lower 8 bits of the 'data' variable are transmitted.
 ****************************************************************************/
void uartTxPoll(uint32_t base, char *data)
{
  UART_PERIPH *myPeriph = (UART_PERIPH *)base;
  
  if ( data != 0)
  {
    while(*data != '\0')
    {
      while( ((myPeriph->Flag)&(UART_FR_TXFF)) != 0 );
      myPeriph->Data = *data;
      data++;
    }
  }
  return;
}

/****************************************************************************
 * This routine transmits a character out the UART / COM port.
 * Only the lower 8 bits of the 'data' variable are transmitted.
 ****************************************************************************/
void uartTxPollChar(uint32_t base, char data)
{
  UART_PERIPH *myPeriph = (UART_PERIPH *)base;
  
  while( ((myPeriph->Flag)&(UART_FR_TXFF)) != 0 );
  myPeriph->Data = data;

  return;
}
