/*******************************************************************************
 * RUNPAD     			                                                       *
 *******************************************************************************
 * Project:		RUNPAD V2
 * Component:	Prototype
 * Element:		Gestion du display
 * File:
 * Description:
 *******************************************************************************
 * History:
 *
 * Version      Date        Author      Description
 * --------------------------------------------------------------------------- *
 * 1.0			03/08/2020	LC			Initial
 *******************************************************************************/
#define LOGGING

/*******************************************************************************
 * Included Files
 *******************************************************************************/
#include "RND_Main.h"
#include "otm8009a/otm8009a.h"

/*******************************************************************************
 * Locals
 *******************************************************************************/

/*******************************************************************************
 * Globals
 *******************************************************************************/

/*******************************************************************************
 * Externals
 *******************************************************************************/

/*******************************************************************************
 * Function     : RND_SetDisplay_Off
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
void
RND_SetDisplay_Off( void)
{
	uint8_t cmd[] = { OTM8009A_CMD_DISPOFF, 0};
	DSI_IO_WriteCmd( 0, cmd);
}

/*******************************************************************************
 * Function     : RND_SetDisplay_On
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
void
RND_SetDisplay_On( void)
{
	uint8_t cmd[] = { OTM8009A_CMD_DISPON, 0};
	DSI_IO_WriteCmd( 0, cmd);
}

/*******************************************************************************
 * Function     : RND_SetDisplay_Sleep
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
void
RND_SetDisplay_Sleep( bool val)
{
	uint8_t	sleepin[]  = { OTM8009A_CMD_SLPIN, 0};
	uint8_t	sleepout[] = { OTM8009A_CMD_SLPOUT, 0};

	if( val == TRUE)
	{
		osDelay(120);
		DSI_IO_WriteCmd( 0, sleepin);
		osDelay(5);
	}
	else
	{
		osDelay(120);
		DSI_IO_WriteCmd( 0, sleepout);
		osDelay(5);
	}
}

/*******************************************************************************
 * Function     : RND_SetDisplay_CABC_OFF
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
void
RND_SetDisplay_CABC_OFF( void)
{
	uint8_t cmd[] = { OTM8009A_CMD_WRCABC, 0x00};	/* param = 0 -> CABC Off */
	DSI_IO_WriteCmd( 0, cmd);
}

/*******************************************************************************
 * Function     : RND_SetDisplay_Backlight
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
void
RND_SetDisplay_Backlight( bool val)
{
	uint8_t cmd_off[] = { OTM8009A_CMD_WRCTRLD, 0 };
	uint8_t cmd_on[]  = { OTM8009A_CMD_WRCTRLD, 0x2c };
	if( val == TRUE)
		DSI_IO_WriteCmd( 0, cmd_on);
	else
		DSI_IO_WriteCmd( 0, cmd_off);
}

/*** End Of File ***/
