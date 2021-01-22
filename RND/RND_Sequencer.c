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
#define _ButtonPressed() 		(BSP_PB_GetState( BUTTON_WAKEUP) != 0)
#define	_WaitButtonPress()		do { while( ! BSP_PB_GetState( BUTTON_WAKEUP) ) \
								osDelay(100); osDelay(200);} while(0);
#define _InitButton()			BSP_PB_Init( BUTTON_WAKEUP, BUTTON_MODE_GPIO);

/*******************************************************************************
 * Globals
 *******************************************************************************/

/*******************************************************************************
 * Externals
 *******************************************************************************/

/*******************************************************************************
 * Function     : RND_SEQ_Step_T1
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
static t_seq_state
RND_SEQ_Step_T1( void)
{
	RND_Print("start"); osDelay(10);
	LOG("Sequencer state = T1\n");

	RND_Led_SendCommand( T_ETEINT);

	/* print runpad logo */
	osDelay( 4*SECOND);

	return SEQ_T2;
}

/*******************************************************************************
 * Function     : RND_SEQ_Step_T2
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
static t_seq_state
RND_SEQ_Step_T2( void)
{
uint8_t count = 20;

	RND_Print("main"); osDelay(10);

	LOG("Sequencer state = T2\n");

	RND_Led_SendCommand( T_CHENILLARD_LENT);

	RND_Print( "SCANNEZ LE\nQR CODE\n"); osDelay(10);

	RND_SetDisplay_Backlight( TRUE);

	osDelay(3*SECOND);

	while( count)
	{
		if( RND_Calc_IsUserOn() == TRUE)
			return SEQ_T3;

		osDelay(100);
		count --;
	}

	return SEQ_T7;
}

/*******************************************************************************
 * Function     : RND_SEQ_Step_T3
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
static t_seq_state
RND_SEQ_Step_T3( void)
{
	RND_Print("main"); osDelay(10);
	LOG("Sequencer state = T3\n");

	RND_Led_SendCommand( T_ALLUMAGE_FIXE);
	RND_Print( "STABILISEZ\nVOTRE\nPOSITION\n");

	/* wait user is stable on device */
	while( RND_Calc_IsStable(5) == FALSE)
	{
		osDelay(100);
		if( RND_Calc_IsUserOn() == FALSE)
			return SEQ_T2;
	}

//#define TEST_VERSION
#ifdef	TEST_VERSION
	static uint16_t cnt = 0;
	static char filename[24];

	static t_gvt_data gvt_data = {0};
	while( TRUE)
	{
		RND_Print("Start");
		osDelay(2*SECOND);

		memset( (void *)&gvt_data, 0, sizeof(gvt_data));
		RND_Gvt_Get( &gvt_data);

		if (gvt_data.pronation == NEUTRE_t)
			RND_Print("TYPE DE\nFOULÉE\nNEUTRE");
		else if (gvt_data.pronation == CONTROL_t)
			RND_Print("TYPE DE\nFOULÉE\nCONTROL");
		else if (gvt_data.pronation == SUPINAL_t)
			RND_Print("TYPE DE\nFOULÉE\nSUPINAL");
		osDelay(1 * SECOND);
		RND_Print("GRAVITY\nDROP\n%d", gvt_data.igvt);
		osDelay(1 * SECOND);

		sprintf( filename, "GVT_%d.csv", cnt++);
		RND_USB_Write_Gvt( &gvt_data, filename);
	}

#else

	static uint16_t	cnt = 0;
	static char filename[24];

	/* Pronation et Gravity drop */
	static t_gvt_data gvt_data = {0};
	RND_Gvt_Get( &gvt_data);

	//sprintf( filename, "GVT_Data_%d.csv\n", cnt);
	//RND_USB_Write_Gvt( &gvt_data, filename);

	/* Pointure */
	static t_measure pointure = {0};
	RND_Size_Get( &pointure);

	for (uint8_t i = 0; i < 2; i++)
	{
		if (gvt_data.pronation == NEUTRE_t)
			RND_Print("FOULÉE\nNEUTRE");
		else if (gvt_data.pronation == NEUTRE_TENDANCE_CONTROL_t)
			RND_Print("FOULÉE\nNEUTRE/CONT");
		else if (gvt_data.pronation == NEUTRE_TENDANCE_SUPINAL_t)
					RND_Print("FOULÉE\nNEUTRE/SUP");
		else if (gvt_data.pronation == CONTROL_t)
			RND_Print("FOULÉE\nCONTROL");
		else if (gvt_data.pronation == SUPINAL_t)
			RND_Print("FOULÉE\nSUPINAL");

		osDelay(3 * SECOND);
		RND_Print("POINTURE\nRUNNING\n%4.1f", pointure.pointure);
		osDelay(3 * SECOND);
		RND_Print("GRAVITY\nDROP\n%d", gvt_data.igvt);
		osDelay(3 * SECOND);
	}
	cnt ++;


#endif

	return SEQ_T4;
}

/*******************************************************************************
 * Function     : RND_SEQ_Step_T4
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
static t_seq_state
RND_SEQ_Step_T4( void)
{
	RND_Print("main"); osDelay(10);
	LOG("Sequencer state = T4\n");

	/* wait user gets off device */
	while( RND_Calc_IsUserOn() == TRUE)
		osDelay(600);

	RND_Led_SendCommand( T_ETEINT);

	return SEQ_T7;
}

/*******************************************************************************
 * Function     : RND_SEQ_Step_T5
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
static t_seq_state
RND_SEQ_Step_T5( void)
{
	return SEQ_T6;
}

/*******************************************************************************
 * Function     : RND_SEQ_Step_T6
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
static t_seq_state
RND_SEQ_Step_T6( void)
{
	RND_Print("start"); osDelay(10);

	LOG("Sequencer state = T6\n");

	RND_Led_SendCommand( T_ETEINT);

	/* print runpad logo */
	osDelay( 4*SECOND);

	return SEQ_T3;
}

/*******************************************************************************
 * Function     : RND_SEQ_Step_T7
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
static t_seq_state
RND_SEQ_Step_T7( void)
{
uint8_t cnt = 0;

	LOG("Sequencer state = T7\n");

	RND_Led_SendCommand( T_CHENILLARD_RAPIDE);

	_goto_low_power();

	while( (RND_Calc_IsUserOn() == FALSE) && (cnt < 15))
	{
		osDelay(SECOND);
		cnt ++;
	}

	_resume_from_low__power();


	if( cnt < 15)
		return SEQ_T6;
	else
		return SEQ_T2;
}

/*******************************************************************************
 * Function     : RND_SEQ_Init
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
t_return
RND_SEQ_Init( void)
{
	_InitButton();
	return E_OK;
}

/*******************************************************************************
 * Function     : RND_SEQ_Sequencer
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
void
RND_SEQ_Sequencer( void)
{
static t_seq_state seq_state = SEQ_T1;

	while( TRUE)
		switch (seq_state)
		{
			case SEQ_T1:	seq_state = RND_SEQ_Step_T1(); 		break;
			case SEQ_T2:	seq_state = RND_SEQ_Step_T2();		break;
			case SEQ_T3:	seq_state = RND_SEQ_Step_T3();		break;
			case SEQ_T4:	seq_state = RND_SEQ_Step_T4();		break;
			case SEQ_T5:	seq_state = RND_SEQ_Step_T5();		break;
			case SEQ_T6:	seq_state = RND_SEQ_Step_T6();		break;
			case SEQ_T7:	seq_state = RND_SEQ_Step_T7();		break;
			default:		break;
		}
}

/*** End Of File ***/
