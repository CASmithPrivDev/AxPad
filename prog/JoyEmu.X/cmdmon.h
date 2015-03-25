#ifndef CMDMON_H
#define CMDMON_H

/*******************************************************************
 *      Name:						cmdmon.h
 *      Author:					C.Smith.
 *      Purpose:				Command-Monitor for AxPad.
 *      Date-First:			25/03/2015.
 *      Date-Last:			25/03/2015.
 *      Notes:					none.
 *      Changes:				25/03/2015:
 *
 *      								Initial Version.
 *
 *******************************************************************/

/*******************************************************************
*	[Includes]
********************************************************************/

/*******************************************************************
*	[Constants]
********************************************************************/

/*******************************************************************
*  [Macros]
********************************************************************/

/*******************************************************************
*  [Data Types]
********************************************************************/

enum t_cmdmon_return_code_e
{
	cmdmon_success	=	0x0000,
	cmdmon_fail			=	0x8000,
	cmdmon_fail_args,
	cmdmon_fail_cmdstring
};

enum t_cmdmon_uart_echo_mode_e
{
	cmdmon_uart_echo_on,
	cmdmon_uart_echo_off
};

enum t_cmdmon_cmd_id_e
{
	cmd_id_dummy,
	cmd_id_inv		=	0xFFFF
};

enum t_cmdmon_arg_type_e
{
	cmdmon_arg_byte		=	0x0000,
	cmdmon_arg_ubyte,
	cmdmon_arg_word,
	cmdmon_arg_uword,
	cmdmon_arg_long,
	cmdmon_arg_ulong,
	cmdmon_arg_char,
	cmdmon_arg_string,
	cmdmon_arg_inv		=	0xFFFF
};

/*******************************************************************
*  [Globals]
********************************************************************/

/*******************************************************************
*  [Function Prototypes]
********************************************************************/

enum t_cmdmon_return_code_e
cmdmon_Init(void);

enum t_cmdmon_return_code_e
cmdmon_CLI(void);



#endif /*	CMDMON_H	*/

