/*******************************************************************
 *      Name:						main.h
 *      Author:					C.Smith.
 *      Purpose:				.
 *      Date-First:			17/03/2015.
 *      Date-Last:			23/03/2015.
 *      Notes:					none.
 *      Changes:				17/03/2015:
 *
 *      								Initial Version.
 *      								
 *******************************************************************/

/*******************************************************************
*	[Includes]
********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <xc.h>

#include "pic18f4550config.h"
#include "uart_driver.h"
#include "delays.h"

/*******************************************************************
*	[Board Configuration Settings]
********************************************************************/
/*
#pragma config FOSC=INTOSCIO_EC
#pragma config BORV=3
#pragma config WDT=OFF
#pragma config CPB=OFF
#pragma config CPD=OFF
*/

/*******************************************************************
*	[Constants]
********************************************************************/

#define JE_DUMMY_BAUD	(51)

/****************************************************
*	[Test-Pins]
*****************************************************/
/*
#define EN_TEST_DDR			DDRA
#define EN_TESTPORT			PORTA
*/

/*******************************************
*	[Test-Pin 1]
*******************************************/
/*
#define EN_TEST_PIN_01	PA7
*/

/*******************************************
*	[Test-Pin 2]
*******************************************/
/*
#define EN_TEST_PIN_02	PA6
*/

/****************************************************
*	[Error-Pins]
*****************************************************/

/*******************************************
*	[Error-Pin 1]
*******************************************/
/*
#define EN_ERROR_PIN_01		PA5
#define EN_ERROR_DDR			DDRA
#define EN_ERRORPORT			PORTA
*/


/*******************************************************************
*  [Data Types]
********************************************************************/

/*******************************************************************
*  [Globals]
********************************************************************/

/*******************************************************************
*  [Macros]
********************************************************************/

/*******************************************************************
*	[ISR]
********************************************************************/

/*******************************************************************
*  [Function Prototypes]
********************************************************************/

/*******************************************************************
*  [Functions]
********************************************************************/


/*
 * 
 */


/*
char uart_tx(char *data, unsigned char size);
char uartROM_tx(const char *data, unsigned char size);
char uart_rx(char *data, unsigned char cmd);
void uart_tx_callback(void);
void uart_rx_callback(void);
void initUARTDriver(char baudHighnLow, unsigned int baudRate);
char uartGetStatus(unsigned char rxtx);

*/

/*******************************************************************
 *  FunctionName:   None.
 *  Synopsis:       None.
 *  Input:          None.
 *  Output:         None.
 *  Returns:        None.
 *  Description:    None.
 *  Cautions:       None.
 *  Comments:       None.
 *******************************************************************/
void
JE_SquareWaveTestPin1	(	void	)
{
	TRISD	=0xFF;	/*	Output.	*/

	while(1)
	{
		//LATDbits.LATD2	=1;	/*	Output.	*/
		LATD=0xFF;
		Delay10TCYx(1000);
		LATD=0x00;
		//LATDbits.LATD2	=0;
		Delay10TCYx(1000);
	}
}

/*******************************************************************
 *  FunctionName:   main.
 *  Synopsis:       None.
 *  Input:          None.
 *  Output:         None.
 *  Returns:        None.
 *  Description:    None.
 *  Cautions:       None.
 *  Comments:       None.
 *******************************************************************/
int
main(int argc, char** argv)
{
	/*	[Init]	*/
//	initUARTDriver((USART_TX_INT_OFF|USART_RX_INT_OFF|USART_BRGH_LOW),JE_DUMMY_BAUD);

	JE_SquareWaveTestPin1();

    return (-1);
}

/*******************************************************************
 *  FunctionName:   None.
 *  Synopsis:       None.
 *  Input:          None.
 *  Output:         None.
 *  Returns:        None.
 *  Description:    None.
 *  Cautions:       None.
 *  Comments:       None.
 *******************************************************************/
