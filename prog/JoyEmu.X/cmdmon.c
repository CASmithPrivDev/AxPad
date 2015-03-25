/*******************************************************************
 *			Name:						cmdmon.c
 *			Author:					C.Smith.
 *			Purpose:				Command-Monitor for AxPad.
 *			Date-First:			25/03/2015.
 *			Date-Last:			25/03/2015.
 *			Notes:					none.
 *			Changes:				25/03/2015:
 *			
 *											Initial Version.
 *
 *******************************************************************/

//******************************************************************
//	[Includes]
//******************************************************************

#include "cmdmon.h"
#include "basictypes.h"

//******************************************************************
//	[Constants]
//******************************************************************

//External oscillator frequency:

#define SYSCLK							(8000000)

//Baudrate:

#define BAUDRATE2   				(19200)

#define CMDMON_UART_BAUDRATE	(BAUDRATE2)

#define CMDMON_UART_BAUDRATE_REG_VAL	(SYSCLK/32/CMDMON_UART_BAUDRATE -1)
#define CMDMON_UART_BAUDRATE_REG			(U2BRG)

//UART IOs:

#define UART2_TX_TRIS   (TRISFbits.TRISF5)
#define UART2_RX_TRIS   (TRISFbits.TRISF4)

#define CMDMON_UART_TRIS_TX	(UART2_TX_TRIS)
#define CMDMON_UART_TRIS_RX	(UART2_RX_TRIS)

#define CMDMON_UART_TX	(U2TXREG)
#define CMDMON_UART_RX	(U2RXREG)

//U2BRG register value and baudrate mistake calculation:

#define BAUDRATEREG2		(SYSCLK/32/BAUDRATE2-1)
 
#if BAUDRATEREG2 > 255
#error Cannot set up UART2 for the SYSCLK and BAUDRATE.\
Correct values in main.h and uart2.h files.
#endif
 
#define BAUDRATE_MISTAKE (1000*(BAUDRATE2-SYSCLK/32/(BAUDRATEREG2+1))/BAUDRATE2)
#if (BAUDRATE_MISTAKE > 2)||(BAUDRATE_MISTAKE < -2)
#error UART2 baudrate mistake is too big  for the SYSCLK and BAUDRATE2. Correct values in uart2.c file.
#endif 

//UART Mode:

#define CMDMON_UART_MODE			(U2MODE)
#define CMDMON_UART_MODE_BITS	(U2MODEbits)

//UART Status:

#define CMDMON_UART_STATUS			(U2STA)
#define CMDMON_UART_STATUS_BITS	(U2STAbits)

#define CMDMON_UART_INT_FLAG_RX						U2RXIF


//Monitor-Commands:

#define	CMDMON_CMDNAME_MAXLEN			(20)
#define CMDMON_CMDHELP_MAXLEN			(200)

#define CMDMON_CMDTAB_SIZE				(4)

#define CMDMON_CMDARG_MAXARGS			(8)
#define CMDMON_CMDARG_ARGMAXLEN		(20)

//Command-Strings:

#define CMDMON_CMDSTRING_TOKEN_MAXLEN			(15)
#define CMDMON_CMDSTRING_TOKEN_MAXCOUNT		(6)
#define CMDMOND_CMDSTRING_HEAD_CHAR				(':')
#define CMDMON_CMDSTRING_SPACE_CHAR				(' ')

//Command-Arguments:

#define CMDMON_CMDARG_MAXNUMARGS					(4)
#define CMDMON_CMDARG_ARGSTRING_MAXLEN		(20)

//******************************************************************
//	[Macros]
//******************************************************************

#define mcr_cmdmon_uart_SetIntFlagStat(statbit,val)	(IFS1bits.statbit	=val)
#define mcr_cmdmon_uart_GetIntFlagStat(statbit,val)	(val	=IFS1bits.statbit)

//******************************************************************
//  [Data Types]
//******************************************************************

//Command-Monitor Command:

struct t_cmdmon_cmd_st
{
	enum t_cmdmon_cmd_id_e					id;																			//Command's ID.
	t_char													s_cmdName[CMDMON_CMDNAME_MAXLEN];				//Command's Name.
	enum t_cmdmon_return_code_e			(*p_cmdFunc)(t_word,t_char **);					//Command's Function.
	t_char													s_cmdHelp[CMDMON_CMDHELP_MAXLEN];				//Command's Help Information.
	t_uword													cmdExpNumArgs;													//The expected number of Arguments for the Command-Function.
	enum t_cmdmon_arg_type_e				cmdArgTypes[CMDMON_CMDARG_MAXNUMARGS];	//Command-Arguments: conversion types.
};

//Command-Monitor Command-Table:

struct t_cmdmon_cmd_tab_st
{
	t_uword													cmdTabSize;
	struct t_cmdmon_cmd_st					cmdTab[CMDMON_CMDTAB_SIZE];
};

//Unionised Argument Datatype:

union t_cmdmon_arg_type_u
{
	t_uword			uw;
	t_word			w;
	t_char			c;
	t_ubyte			ub;
	t_byte			b;
	t_ulong			ul;
	t_long			l;
	t_char			s[CMDMON_CMDARG_ARGSTRING_MAXLEN];
};

//******************************************************************
//	[Function Prototypes]
//******************************************************************

//*************************************************
//	[UART Functions]
//*************************************************

static enum t_cmdmon_return_code_e				cmdmon_uart_PutChar		(	const t_char 										*	p_char,
																																	enum t_cmdmon_uart_echo_mode_e		uartMode			);
static enum t_cmdmon_return_code_e				cmdmon_uart_GetChar		(	t_char 													* p_char,
																																	enum t_cmdmon_uart_echo_mode_e		uartMode			);
					
static enum t_cmdmon_return_code_e				cmdmon_uart_PutString	(	const t_char 										* pc_string,
																																	enum t_cmdmon_uart_echo_mode_e		uartMode			);

static enum t_cmdmon_return_code_e				cmdmon_uart_GetString	(	t_char 													* p_string,
																																	const t_ubyte 									maxStringLen,
																																	enum t_cmdmon_uart_echo_mode_e	uartMode				);
																																	
//*************************************************
//	[Command-Monitor Functions]
//*************************************************

inline static enum t_cmdmon_return_code_e				cmdmon_ProcessCmd			(	t_char											*	p_cmdString	);
static enum t_cmdmon_return_code_e							cmdmon_DisplayBanner	(	void	);

//*************************************************
//	[Command Functions]
//*************************************************

/*
 * 	Command-Functions carry out the actual work that the User requests through the Serial-Port.
 *	At present there is only a single Function used for demonstrative purpose: cmdmon_CmdDummy.
 */

static enum t_cmdmon_return_code_e							cmdmon_CmdDummy				(	t_word					argc,
																																				t_char 					(*	p_argv)[CMDMON_CMDARG_ARGSTRING_MAXLEN]	);
//*************************************************
//	[Argument Conversion Functions]
//*************************************************

/*
 *	Argument Conversion Functions to convert the User inputted Argument-Strings into required type, e.g. "1234" can be conververted to an-
 *	unsigned Word.
 */

static enum t_cmdmon_return_code_e							cmdmon_conv_ConvArgToType	(	t_char										*		ps_arg,
																																						enum	t_cmdmon_arg_type_e			argType,
																																						union	t_cmdmon_arg_type_u	*		p_argConv	);

//*************************************************
//	[Utility Functions]
//*************************************************

inline void																			util_Nop	(	t_ulong					nopCnt	);

//******************************************************************
//	[Globals]
//******************************************************************

/*
 * Command-Table used to hold all the Commands that the Command-Monitor can execute.
 * (*)	This Table will be updated as further Commands are added to the Command-Monitor.
 * (*)	The size of the Command-Table	must be changed to ensure that the Table includes all the-
 * required Commands by adjusting the CMDMON_CMDTAB_SIZE Constant.
 */ 

struct t_cmdmon_cmd_tab_st	g_cmdTab =
{
	CMDMON_CMDTAB_SIZE,
	{
		{	cmd_id_dummy			,	"cmddummy"		,	cmdmon_CmdDummy	,	"help: dummy."	,	2	,	{	cmdmon_arg_char	,	
																																										cmdmon_arg_word	,	
																																										cmdmon_arg_inv	,	
																																										cmdmon_arg_inv	}
		},
		{	cmd_id_inv				,	"<invalid>"		,	NULL						,	"<invalid>"			,	0	,	{	cmdmon_arg_inv,
																																										cmdmon_arg_inv,
																																										cmdmon_arg_inv,
																																										cmdmon_arg_inv	}
		},
		{	cmd_id_inv				,	"<invalid>"		,	NULL						,	"<invalid>"			,	0	,	{	cmdmon_arg_inv,
																																										cmdmon_arg_inv,
																																										cmdmon_arg_inv,
																																										cmdmon_arg_inv	}
		},
		{	cmd_id_inv				,	"<invalid>"		,	NULL						,	"<invalid>"			,	0	,	{	cmdmon_arg_inv,
																																										cmdmon_arg_inv,
																																										cmdmon_arg_inv,
																																										cmdmon_arg_inv	}
		},
	}
};

//******************************************************************
//	[Functions]
//******************************************************************

//************************************************************
//	[Initialisation Functions]
//************************************************************

/*******************************************************************
 #  FunctionName:   cmdmon_Init.
 #  Synopsis:       Initialise the Explorer-16.
 #  Input:          None.
 #  Output:         None.
 #  Returns:        cmdmon_success:	will always be returned.
 #  Description:    Initialises the Explorer-16's UART for Baudrate
 #									TX/RX Pin Directions, etc.
 #  Cautions:       None.
 #  Comments:       Call this Function before attempting to use the UART.
 *******************************************************************/

enum t_cmdmon_return_code_e
cmdmon_Init(void)
{
	//Set Tristates for the UART's Input and Output:
	CMDMON_UART_TRIS_TX	=0;	
	CMDMON_UART_TRIS_RX	=1;

	//Set UART's Baudrate:
	CMDMON_UART_BAUDRATE_REG	=CMDMON_UART_BAUDRATE_REG_VAL;

	CMDMON_UART_MODE	=0;

	CMDMON_UART_STATUS	=0;

	CMDMON_UART_MODE_BITS.UARTEN	=1;

	CMDMON_UART_STATUS_BITS.UTXEN	=1;

	mcr_cmdmon_uart_SetIntFlagStat(CMDMON_UART_INT_FLAG_RX,0);

	return(cmdmon_success);
}


//************************************************************
//	[CLI Functions]
//************************************************************

/*******************************************************************
 #  FunctionName:   cmdmon_CLI.
 #  Synopsis:       Presents a Command-Line Interface on a Serial-Port.
 #  Input:          None.
 #  Output:         None.
 #  Returns:        cmdmon_fail:	this should never be returned due to-
 #									the infinite While-Loop.
 #  Description:    Accept User given Command-String that states which-
 #									Command the User wishes to execute along with the-
 #									required Parameters for the Command to use.
 #  Cautions:       call cmdmon_Init before calling this Function.
 #  Comments:       This Function will continue until the Board is reset.
 *******************************************************************/

enum t_cmdmon_return_code_e
cmdmon_CLI(void)
{
	const t_char	lr	='\r';
	const t_char	lf	='\n';	
	t_char				stringBuff[CMDMON_CMDARG_ARGSTRING_MAXLEN];
	const t_char	message[]	={	"\nReceived: "	};

	cmdmon_DisplayBanner();

	while(1)
	{
		//Display the CLI Prompt:
		cmdmon_uart_PutString("\r\n> ",cmdmon_uart_echo_off);
		//Get the User Input and echo the Input:
		cmdmon_uart_GetString(&stringBuff,(CMDMON_CMDARG_ARGSTRING_MAXLEN -1),cmdmon_uart_echo_on);
		//Inform the User what they have requested the CLI to perform:
		cmdmon_uart_PutString	(&message,cmdmon_uart_echo_off);
		cmdmon_uart_PutString	(&stringBuff,cmdmon_uart_echo_off);
		//Process the User Input and attempt to execute the Command requested.

//		util_Nop	(4294967295);
		util_Nop	(1000);

		cmdmon_ProcessCmd			(	&stringBuff	);
	}

	return(cmdmon_fail);
}

/*******************************************************************
 #  FunctionName:   cmdmon_ProcessCmd.
 #  Synopsis:       Process the User's Command-String.
 #  Input:          p_cmdString:	User inputted Command-String from Serial-Port.
 #  Output:         None.
 #  Returns:        cmdmon_success:					Command given was valid and executed.
 #									cmdmon_fail_args:				Fail due to incorrect number of Arguments-
 #																					inputted.
 #									cmdmon_fail_cmdstring:	Fail due to the Command not being recognised.
 #									cmdmon_fail:						Fail due to the Command failing after execution.
 #  Description:    Tokenise the Command-String, find a matching Command in the-
 #									Command-Table, then convert each of the Tokens to the-
 #									required Types for use by the requested Command.
 #  Cautions:       None.
 #  Comments:       Any Commands that are not in the Command-Table will not be-
 #									executed.
 *******************************************************************/

inline static enum t_cmdmon_return_code_e
cmdmon_ProcessCmd					(	t_char				*	p_cmdString)
{
	t_char											tokenBuff[CMDMON_CMDSTRING_TOKEN_MAXCOUNT][CMDMON_CMDSTRING_TOKEN_MAXLEN];
	t_char											tokDelims[]	=	{	
																							CMDMOND_CMDSTRING_HEAD_CHAR,
																							CMDMON_CMDSTRING_SPACE_CHAR	
																						};
	t_char											cmdTokBuff[CMDMON_CMDSTRING_TOKEN_MAXLEN];
	t_char										*	p_tok								=NULL;
	t_ubyte											currTok							=0;
	t_ubyte											numToks							=0;
	t_ubyte											currCmd							=0;
	enum t_bool_e								cmdTokenValid				=false;
	t_char											cmdArgs[CMDMON_CMDARG_MAXARGS][CMDMON_CMDARG_ARGMAXLEN];
	t_ubyte											cmdArgsFoundCnt			=0;
	enum t_cmdmon_return_code_e	retCode							=cmdmon_fail;

	//Checks:

	if(NULL ==p_cmdString)
	{
		return(cmdmon_fail_args);
	}

	//Command-String Format should be ":command-name arg(s)", check to see if the first Character is the Colon:
	if(CMDMOND_CMDSTRING_HEAD_CHAR !=p_cmdString[0])
	{
		return(cmdmon_fail_cmdstring);
	}

	//Initialise Command-Token-Buffer:
	memset(&cmdTokBuff[0],0,(sizeof(cmdTokBuff[0])*CMDMON_CMDSTRING_TOKEN_MAXLEN));

	//Initialise Token-Buffer:
	//ToDo: Fails for memset of Token-Buffer.
	//memset(&tokenBuff[0],0,(sizeof(tokenBuff[0]) *(CMDMON_CMDSTRING_TOKEN_MAXCOUNT *CMDMON_CMDSTRING_TOKEN_MAXLEN)  ));

	//Tokenise the Command-String:
	p_tok	=strtok(p_cmdString+1,tokDelims);
	if(NULL ==p_tok)
	{
		return(cmdmon_fail);
	}

	strcpy(&cmdTokBuff,p_tok);
	numToks++;

	//Search the Command-Table for the Command-Token found:
	cmdTokenValid				=false;

	for(currCmd=0;g_cmdTab.cmdTabSize >currCmd;currCmd++)
	{
		if(0	==strcmp(&cmdTokBuff,&g_cmdTab.cmdTab[currCmd].s_cmdName))
		{
			cmdTokenValid				=true;	//Command-String was found in the Command-Table.
			break;
		}
	}

	//Could the Command-String not be found in the Command-Table:
	if(false	==cmdTokenValid)
	{
		return(cmdmon_fail_cmdstring);
	}

	//Extract the rest of the Tokens from the Command-String:
	currTok	=0;
	cmdArgsFoundCnt	=0;
	while(NULL !=p_tok)
	{
		p_tok	=strtok(NULL,tokDelims);
		if(NULL !=p_tok)
		{
			strcpy((&tokenBuff),p_tok);
			currTok++;

			cmdmon_uart_PutString	((&tokenBuff),cmdmon_uart_echo_on);

			util_Nop	(1000);
			strcpy(&cmdArgs[cmdArgsFoundCnt],p_tok);
			cmdArgsFoundCnt++;
		}
	}

	//Pass the Argument-Tokens to the command-Function and then execute the Command:
	if(cmdmon_success !=(retCode	=g_cmdTab.cmdTab[currCmd].p_cmdFunc(cmdArgsFoundCnt,&cmdArgs)))
	{
		return(cmdmon_fail);
	}

	return(cmdmon_success);
}


//************************************************************
//	[UART Functions]
//************************************************************

/*******************************************************************
 #  FunctionName:   cmdmon_uart_GetChar.
 #  Synopsis:       Receive a single Character into the UART from the Serial-Port.
 #  Input:          p_char:		Character-Buffer Reference.
 #									uartMode:	UART Echo-Mode:On/Off.
 #  Output:         p_char:		Character-Buffer contains the Character received.
 #  Returns:        cmdmon_success:			Received a Character into the UART over-
 #																			the Serial-Port.
 #									cmdmon_fail_args:		Character-Buffer Reference was Null.
 #  Description:    Continue to wait until the UART Interrupts with a Character-
 #									received Event.
 #  Cautions:       None.
 #  Comments:       This Function will loop until it receives a Character.
 *******************************************************************/

static enum t_cmdmon_return_code_e
cmdmon_uart_GetChar(	t_char 													* p_char,
											enum t_cmdmon_uart_echo_mode_e		uartMode			)
{
	t_char tmpData	=0x00;

	//Checks:

	if(NULL ==p_char)
	{
		return(cmdmon_fail_args);
	}

	//Wait for Received Data:
	while(0 ==mcr_cmdmon_uart_GetIntFlagStat(CMDMON_UART_INT_FLAG_RX,tmpData));
	
	tmpData	=CMDMON_UART_RX;	//Receive the Character from the UART.
	*p_char	=tmpData;					//Copy the received Character the Character-Buffer.

	//Should the Character receieved be echoed to the UART's Output?
	if(cmdmon_uart_echo_on ==uartMode)
	{
		cmdmon_uart_PutChar(p_char,cmdmon_uart_echo_off);	//Echo the Character back.
	}

	//Clear the Receive Interrupt-Flag:
	mcr_cmdmon_uart_SetIntFlagStat(CMDMON_UART_INT_FLAG_RX,0);

	return(cmdmon_success);
}

/*******************************************************************
 #  FunctionName:   cmdmon_uart_PutChar.
 #  Synopsis:       Transmit a single Character from the UART over the Serial-Port.
 #  Input:          p_char: 		pointer to the Character to send to the UART.
 #									uartMode:		UART Echo-Mode: On/Off.
 #  Output:         None.
 #  Returns:        cmdmon_success: 	Character transmitted from the UART over the-
 #																		Serial-Port.
 #									cmdmon_fail_args:	Transmit Buffer containing the Character to-
 #																		transmit is Null.
 #  Description:    Write a Character to the UART and transmit over the Serial-Port.
 #  Cautions:       Function has Static Scope.
 #  Comments:       This Function will loop until it has transmitted a Character.
 *******************************************************************/

static enum t_cmdmon_return_code_e
cmdmon_uart_PutChar(	const t_char 										* p_char,
											enum t_cmdmon_uart_echo_mode_e		uartMode			)
{
	t_char	tmpChar	=0;

	//Checks:
	if(NULL ==p_char)
	{
		return(cmdmon_fail_args);
	}

	// wait for empty buffer  
	while(1 ==CMDMON_UART_STATUS_BITS.UTXBF);
	
	CMDMON_UART_TX	=(*p_char);	//Transmit the Character to the UART Port.
/*
	if(cmdmon_uart_echo_on ==uartMode)
	{
		cmdmon_uart_PutChar(p_char,cmdmon_uart_echo_off);	//Echo the Character back.
	}
*/

	return(cmdmon_success);
}

void DummyPutchar(char data)
{
	  while(U2STAbits.UTXBF == 1);
      U2TXREG = data;
}

/*******************************************************************
 #  FunctionName:   None.
 #  Synopsis:       None.
 #  Input:          None.
 #  Output:         None.
 #  Returns:        None.
 #  Description:    None.
 #  Cautions:       None.
 #  Comments:       None.
 *******************************************************************/
static enum t_cmdmon_return_code_e
cmdmon_uart_PutString(	const t_char * pc_string,
												enum t_cmdmon_uart_echo_mode_e		uartMode			)
{
	//Checks:
	
	if(NULL ==pc_string)
	{
		return(cmdmon_fail_args);
	}

	while(NULL !=(*pc_string))
	{
		cmdmon_uart_PutChar(pc_string++,uartMode);
	}

	return(cmdmon_success);
}

/*******************************************************************
 #  FunctionName:   cmdmon_uart_GetString.
 #  Synopsis:       None.
 #  Input:          pc_string,	String Buffer to write the received String to.
 #  Output:         None.
 #  Returns:        None.
 #  Description:    None.
 #  Cautions:       None.
 #  Comments:       None.
 *******************************************************************/
static enum t_cmdmon_return_code_e
cmdmon_uart_GetString	(	t_char 													* p_string,
												const t_ubyte 										maxStringLen,
												enum t_cmdmon_uart_echo_mode_e		uartMode			)
{
	t_ubyte		rxCharCnt								=0;			//Characters received Count.
	enum			t_bool_e	rxCont				=true;	//Flag: Continue to receive Characters over the UART (break on a terminating Character).
	t_char		tmpCharBuff							=0;			//Buffer to hold Characters received from the UART temporarily before appending to the String.

	//Checks:
	
	if(NULL ==p_string)
	{
		return(cmdmon_fail_args);
	}

	if(0	==maxStringLen)
	{
		return(cmdmon_fail_args);
	}

	//Receive the Characters, stop on Terminator-Characters:
	while((true ==rxCont) &&(rxCharCnt <maxStringLen))
	{
		//Checks:
		if(rxCharCnt ==maxStringLen)
		{
			rxCont	=false;
			*(p_string +rxCharCnt) ='\0';	//Add a NULL to terminate the String.
		}
		else
		{
			if(cmdmon_uart_GetChar(&tmpCharBuff,uartMode) ==cmdmon_fail_args)
			{
				rxCont	=false;
				*(p_string +rxCharCnt) ='\0';	//Add a NULL to terminate the String.
				return(cmdmon_fail);
			}

			if('\r' ==tmpCharBuff)	//Terminating Character Input with a Return.
			{
				rxCont	=false;
				*(p_string +rxCharCnt) ='\0';	//Add a NULL to terminate the String.
				return(cmdmon_success);
			}
			else	//Standard Character to append to the String.
			{
				*(p_string +rxCharCnt) =tmpCharBuff;	//Add the last received Character to the String.
				rxCharCnt++;
			}
		}
	}
	return(cmdmon_fail);
}

//************************************************************
//	[Command Functions]
//************************************************************

/*******************************************************************
 #  FunctionName:   cmdmon_CmdDummy.
 #  Synopsis:       None.
 #  Input:          None.
 #  Output:         None.
 #  Returns:        None.
 #  Description:    None.
 #  Cautions:       None.
 #  Comments:       None.
 *******************************************************************/
static enum t_cmdmon_return_code_e
cmdmon_CmdDummy			(	t_word 		argc,
											t_char 					(*	p_argv)[CMDMON_CMDARG_ARGSTRING_MAXLEN]	)
{
	t_char											argChar		=0;
	t_word											argWord		=0;
	t_uword											currCmd		=0;
	enum t_bool_e								cmdFound	=false;
	union	t_cmdmon_arg_type_u		argConv;

	memset(&argConv,0,sizeof(union t_cmdmon_arg_type_u));

	cmdmon_uart_PutString	("\ndebug: @cmdmon_CmdDummy.",cmdmon_uart_echo_off);

	//Find the Command in the Command-Table:
	for(currCmd	=0;currCmd	<g_cmdTab.cmdTabSize;currCmd++)
	{
		if(cmd_id_dummy	==g_cmdTab.cmdTab[currCmd].id)
		{
			cmdFound	=true;
			break;
		}
	}	

	if(false	==cmdFound)
	{
		return(cmdmon_fail);
	}

	if(argc	!=g_cmdTab.cmdTab[currCmd].cmdExpNumArgs)
	{
		return(cmdmon_fail_args);
	}

	argChar	=0;
	cmdmon_conv_ConvArgToType	(p_argv[0],g_cmdTab.cmdTab[currCmd].cmdArgTypes[0],&argConv);
	argChar	=argConv.c;
		
	argWord	=0;
	cmdmon_conv_ConvArgToType	(p_argv[1],g_cmdTab.cmdTab[currCmd].cmdArgTypes[1],&argConv);
	argWord	=argConv.w;

	argWord++;

	return(cmdmon_success);
}

//*************************************************
//	[Argument Conversion Functions]
//*************************************************

/*******************************************************************
 #  FunctionName:   cmdmon_conv_ConvArgToType.
 #  Synopsis:       None.
 #  Input:          None.
 #  Output:         None.
 #  Returns:        None.
 #  Description:    None.
 #  Cautions:       None.
 #  Comments:       None.
 *******************************************************************/
//ToDo: Should I use a Parameter to convert each type of Argument or have individual conversion-Functions?
static enum t_cmdmon_return_code_e
cmdmon_conv_ConvArgToType	(	t_char										*		ps_arg,
														enum	t_cmdmon_arg_type_e			argType,
														union	t_cmdmon_arg_type_u	*		p_argConv	)
{
	switch(argType)
	{
		case	cmdmon_arg_byte:
		{
			p_argConv->b	=atoi(ps_arg);
			break;
		}
		case	cmdmon_arg_ubyte:
		{
			p_argConv->ub	=atoi(ps_arg);
			break;
		}
		case	cmdmon_arg_word:
		{
			p_argConv->w	=atoi(ps_arg);
			break;
		}
		case	cmdmon_arg_uword:
		{
			p_argConv->uw	=atoi(ps_arg);
			break;
		}
		case	cmdmon_arg_long:
		{
			p_argConv->l	=atol(ps_arg);
			break;
		}
		case	cmdmon_arg_ulong:
		{
			p_argConv->ul	=atol(ps_arg);
			break;
		}
		case	cmdmon_arg_char:
		{
			p_argConv->c	=ps_arg[0];
			break;
		}
		case	cmdmon_arg_string:
		{
			strcpy(p_argConv->s,ps_arg);
			break;
		}
		case	cmdmon_arg_inv:
		{
			break;
		}
		default:
		{
			return(cmdmon_fail_args);
		}
	}

	return(cmdmon_success);
}

//************************************************************
//	[Utility Functions]
//************************************************************

/*******************************************************************
 #  FunctionName:   None.
 #  Synopsis:       None.
 #  Input:          None.
 #  Output:         None.
 #  Returns:        None.
 #  Description:    None.
 #  Cautions:       None.
 #  Comments:       None.
 *******************************************************************/
static enum t_cmdmon_return_code_e
cmdmon_DisplayBanner(void)
{
	t_char	banner[3][100]=
	{
		"\r\n*****************************************",
		"\r\n*            Command-Monitor            *",
		"\r\n*****************************************"
	};
	
	t_ubyte	bannerLine	=0;

	for(bannerLine=0;bannerLine <3;bannerLine++)
	{
		cmdmon_uart_PutString	(	&banner[bannerLine],cmdmon_uart_echo_off );
	}

	return(cmdmon_fail);
}

//*************************************************
//	[Utility Functions]
//*************************************************

/*******************************************************************
 #  FunctionName:   None.
 #  Synopsis:       None.
 #  Input:          None.
 #  Output:         None.
 #  Returns:        None.
 #  Description:    None.
 #  Cautions:       None.
 #  Comments:       None.
 *******************************************************************/

inline void
util_Nop	(	t_ulong					nopCnt	)
{
	t_ulong			cnt	=0;

	for(cnt=0;cnt<nopCnt;cnt++);

}


/*******************************************************************
 #  FunctionName:   None.
 #  Synopsis:       None.
 #  Input:          None.
 #  Output:         None.
 #  Returns:        None.
 #  Description:    None.
 #  Cautions:       None.
 #  Comments:       None.
 *******************************************************************/
enum t_cmdmon_return_code_e
cmdmon_(void)
{
	return(cmdmon_fail);
}

/*******************************************************************
 #  FunctionName:   None.
 #  Synopsis:       None.
 #  Input:          None.
 #  Output:         None.
 #  Returns:        None.
 #  Description:    None.
 #  Cautions:       None.
 #  Comments:       None.
 *******************************************************************/
