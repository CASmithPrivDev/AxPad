#ifndef	PIC18F4550CONFIG_H
#define	PIC18F4550CONFIG_H

/*******************************************************************
 *      Name:						pic18f4550config.h
 *      Author:					C.Smith.
 *      Purpose:				Configure the PIC18F4550 for use in the Joypad Emulator Project.
 *      Date-First:			17/03/2015.
 *      Date-Last:			17/03/2015.
 *      Notes:					none.
 *      Changes:				17/03/2015:
 *
 *      								Initial Version.
 *
 *******************************************************************/

/*******************************************************************
*	[Includes]
********************************************************************/

/*******************************************************************
*	[Board Configuration Settings]
********************************************************************/

/*
 PLL
 */

#pragma	config	PLLDIV=1	/*	No prescale.	*/

/*
FOSC =	Oscillator Selection bits

HS	HS oscillator (HS)
XTPLL_XT	XT oscillator, PLL enabled (XTPLL)
ECPLLIO_EC	EC oscillator, PLL enabled, port function on RA6 (ECPIO)
INTOSC_XT	Internal oscillator, XT used by USB (INTXT)
INTOSC_EC	Internal oscillator, CLKO function on RA6, EC used by USB (INTCKO)
ECPLL_EC	EC oscillator, PLL enabled, CLKO function on RA6 (ECPLL)
EC_EC	EC oscillator, CLKO function on RA6 (EC)
ECIO_EC	EC oscillator, port function on RA6 (ECIO)
XT_XT	XT oscillator (XT)
HSPLL_HS	HS oscillator, PLL enabled (HSPLL)
INTOSC_HS	Internal oscillator, HS oscillator used by USB (INTHS)
INTOSCIO_EC	Internal oscillator, port function on RA6, EC used by USB (INTIO)
*/

#pragma	config	FOSC=INTOSC_XT	/*	Using USB's clock option for internal oscillator for now.	*/

/*
WDTPS =	Watchdog Timer Postscale Select bits
16384	1:16384
128	1:128
4	1:4
4096	1:4096
2048	1:2048
1024	1:1024
256	1:256
32	1:32
32768	1:32768
16	1:16
8	1:8
1	1:1
8192	1:8192
512	1:512
2	1:2
64	1:64
WDT =	Watchdog Timer Enable bit
ON	WDT enabled
OFF	WDT disabled (control is placed on the SWDTEN bit)
*/

#pragma	config	WDT=OFF	/*	No Watchdog for now.	*/

/*
BORV =	Brown-out Reset Voltage bits
0	Maximum setting
1	
2	
3	Minimum setting
BOR =	Brown-out Reset Enable bits
SOFT	Brown-out Reset enabled and controlled by software (SBOREN is enabled)
ON	Brown-out Reset enabled in hardware only (SBOREN is disabled)
OFF	Brown-out Reset disabled in hardware and software
ON_ACTIVE	Brown-out Reset enabled in hardware only and disabled in Sleep mode (SBOREN is disabled)
*/

#pragma	config	BOR=OFF	/*	No Brown-Out enabled.	*/

/*
PBADEN =	PORTB A/D Enable bit
ON	PORTB<4:0> pins are configured as analog input channels on Reset
OFF	PORTB<4:0> pins are configured as digital I/O on Reset
*/

#pragma	config	PBADEN=OFF	/*	All PortB ports are used as Digital I/O.	*/

/*
LVP =	Single-Supply ICSP Enable bit
ON	Single-Supply ICSP enabled
OFF	Single-Supply ICSP disabled
*/

//#pragma	config	LVP=OFF	/*	low-voltage programming: disabled.	*/
#pragma	config	LVP=ON	/*	low-voltage programming: disabled.	*/

/*
Dedicated In-Circuit Debug/Programming Port (ICPORT) Enable bit:
ICPRT = OFF	:	ICPORT disabled.
ICPRT = ON	:	ICPORT enabled.
*/

#pragma	config	ICPRT = ON

/*******************************************************************
*	[Constants]
********************************************************************/

/*******************************************************************
*  [Macros]
********************************************************************/

/*******************************************************************
*  [Data Types]
********************************************************************/

/*******************************************************************
*  [Globals]
********************************************************************/

/*******************************************************************
*  [Function Prototypes]
********************************************************************/

#endif	/*	PIC18F4550CONFIG_H	*/

