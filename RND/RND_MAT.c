/*******************************************************************************
 * RUNPAD     			                                                       *
 *******************************************************************************
 * Project:		RUNPAD V2
 * Component:	Prototype
 * Element:
 * File:
 * Description:
 *******************************************************************************
 * Historique:
 *
 * Version      Date        Author      Description
 * --------------------------------------------------------------------------- *
 * 1.0
 *******************************************************************************/
#define LOGGING

/*******************************************************************************
 * Included Files
 *******************************************************************************/
#include "RND_Main.h"

/*******************************************************************************
 * Locals
 *******************************************************************************/
static t_gpio	gpio[] =
{
		{ LINE_A0_GPIO_Port, LINE_A0_Pin },
		{ LINE_A1_GPIO_Port, LINE_A1_Pin },
		{ LINE_A2_GPIO_Port, LINE_A2_Pin },
		{ LINE_A3_GPIO_Port, LINE_A3_Pin },
		{ MN5_OE_GPIO_Port, MN5_OE_Pin },
		{ MN6_OE_GPIO_Port, MN6_OE_Pin },
		{ MN7_OE_GPIO_Port, MN7_OE_Pin }
};

/*******************************************************************************
 * Globals
 *******************************************************************************/

/*******************************************************************************
 * Externals
 *******************************************************************************/

/*******************************************************************************
 * Function     : RND_MAT_Select_Line
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
void
RND_MAT_Select_Line( uint8_t line)
{
#if	(REAL_HARDWARE)
	if( line < 48)
	{
		if( line < 16)
		{
			_set_line( MN5_OE, GPIO_PIN_RESET);
			_set_line( MN6_OE, GPIO_PIN_SET);
			_set_line( MN7_OE, GPIO_PIN_SET);
		}
		else if( line < 32)
		{
			_set_line( MN5_OE, GPIO_PIN_SET);
			_set_line( MN6_OE, GPIO_PIN_RESET);
			_set_line( MN7_OE, GPIO_PIN_SET);
		}
		else
		{
			_set_line( MN5_OE, GPIO_PIN_SET);
			_set_line( MN6_OE, GPIO_PIN_SET);
			_set_line( MN7_OE, GPIO_PIN_RESET);
		}

		_set_line( LINE_A0, (line &  1    ) ? GPIO_PIN_SET : GPIO_PIN_RESET);
		_set_line( LINE_A1, (line & (1<<1)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
		_set_line( LINE_A2, (line & (1<<2)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
		_set_line( LINE_A3, (line & (1<<3)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	}
	else
	{
		/* if line >= 48, reset all signals to their inactive state */
		_set_line( LINE_A0, GPIO_PIN_RESET);
		_set_line( LINE_A1, GPIO_PIN_RESET);
		_set_line( LINE_A2, GPIO_PIN_RESET);
		_set_line( LINE_A3, GPIO_PIN_RESET);
		_set_line( MN5_OE, GPIO_PIN_SET);
		_set_line( MN6_OE, GPIO_PIN_SET);
		_set_line( MN7_OE, GPIO_PIN_SET);
	}
#endif
	return;
}

/*******************************************************************************
 * Function     : RND_MAT_Select_Columns
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
void
RND_MAT_Select_Columns( uint8_t colgroup)
{
#if	(REAL_HARDWARE)
	switch( colgroup)
	{
		case COL_GROUP_0:
			HAL_GPIO_WritePin(SEL_COL_A_GPIO_Port, SEL_COL_A_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(SEL_COL_B_GPIO_Port, SEL_COL_B_Pin, GPIO_PIN_RESET);
			break;

		case COL_GROUP_1:
			HAL_GPIO_WritePin(SEL_COL_A_GPIO_Port, SEL_COL_A_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(SEL_COL_B_GPIO_Port, SEL_COL_B_Pin, GPIO_PIN_RESET);
			break;

		case COL_GROUP_2:
			HAL_GPIO_WritePin(SEL_COL_A_GPIO_Port, SEL_COL_A_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(SEL_COL_B_GPIO_Port, SEL_COL_B_Pin, GPIO_PIN_SET);
			break;

		case COL_GROUP_3:
			HAL_GPIO_WritePin(SEL_COL_A_GPIO_Port, SEL_COL_A_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(SEL_COL_B_GPIO_Port, SEL_COL_B_Pin, GPIO_PIN_SET);
			break;

		default:
			break;
	}
#endif
	return;
}

/*******************************************************************************
 * Function     : RND_MAT_Init
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
t_return
RND_MAT_Init( void)
{
t_return ret = E_ERROR;

#if	(REAL_HARDWARE)

	_set_line( LINE_A0, GPIO_PIN_RESET);
	_set_line( LINE_A1, GPIO_PIN_RESET);
	_set_line( LINE_A2, GPIO_PIN_RESET);
	_set_line( LINE_A3, GPIO_PIN_RESET);
	_set_line( MN5_OE, GPIO_PIN_SET);
	_set_line( MN6_OE, GPIO_PIN_SET);
	_set_line( MN7_OE, GPIO_PIN_SET);
#endif

	ret = E_OK;

	return ret;
}

/*** End Of File ***/
