/*******************************************************************************
 * RUNPAD     			                                                       *
 *******************************************************************************
 * Project:		RUNPAD V2
 * Component:	Prototype
 * Element:		Gestion des LEDs
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

/*******************************************************************************
 * Locals
 *******************************************************************************/
static TaskHandle_t		ledTaskHandle = NULL;
//static void		RND_LedSet				(uint8_t val);

/*******************************************************************************
 * Globals
 *******************************************************************************/
QueueHandle_t			ledQueue = NULL;

/*******************************************************************************
 * Externals
 *******************************************************************************/

/*******************************************************************************
 * Function     : LedTask
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
static void
LedTask( void *p)
{
	t_LED_Cmd cmd = T_ETEINT;
	static uint8_t led = 1;

	while(TRUE)
	{
		/* do we have a new command ? */
		if( xQueueReceive( ledQueue, &cmd, 0) == pdPASS)
		{
			/* handle new command */
			led = 1;
		}

		switch( cmd)
		{
			case T_ETEINT:
#if (REAL_HARDWARE)
				RND_EXP_SetLed( 0);
#else
				BSP_LED_Off( LED1);
				BSP_LED_Off( LED2);
				BSP_LED_Off( LED3);
				BSP_LED_Off( LED4);
#endif
				osDelay(200);
				break;

			case T_CHENILLARD_RAPIDE:
			case T_CHENILLARD_LENT:

#if (REAL_HARDWARE)
				led <<= 1;
				if( led == (1<<5))
					led = 1;
				RND_EXP_SetLed( led);
				osDelay(cmd == T_CHENILLARD_RAPIDE ? 50 : 300);
				RND_EXP_SetLed( 0);
#else
				RND_LedSet( led);
				osDelay(20);
				RND_LedSet( 0);
				led <<= 1;
				if( led == (1<<4))
					led = 1;
#endif
				osDelay(cmd == T_CHENILLARD_RAPIDE ? 50 : 1);
				break;

			case T_CHENILLARD_LENT_INVERSE:

#if (REAL_HARDWARE)
				//RND_EXP_SetLed( led);
				led >>= 1;
				if( led == 0)
					led = 1 << 4;
				RND_EXP_SetLed( led);
#else
				RND_LedSet( led);
				osDelay(20);
				RND_LedSet( 0);
				led >>= 1;
				if( led == 0)
					led = 1 << 3;
#endif
				osDelay( 500);
				break;

			case T_ALLUMAGE_FIXE:
#if (REAL_HARDWARE)
				RND_EXP_SetLed(0x1f);
#else
				BSP_LED_On( LED1);
				BSP_LED_On( LED2);
				BSP_LED_On( LED3);
				BSP_LED_On( LED4);
#endif
				osDelay(200);
				break;

			default:
				break;
		}
	}
}

/*******************************************************************************
 * Function     : RND_Led_SendCommand
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
t_return
RND_Led_SendCommand( t_LED_Cmd cmd)
{
	if( xQueueSend( ledQueue, &cmd, 100) != pdPASS)
		return E_ERROR;
	else
		return E_OK;
}

/*******************************************************************************
 * Function     : RND_LedInit
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
t_return
RND_LedInit( void)
{
	/* initialize devboard LEDs */
	BSP_LED_Init(LED1);		BSP_LED_Off(LED1);
	BSP_LED_Init(LED2);		BSP_LED_Off(LED2);
	BSP_LED_Init(LED3);		BSP_LED_Off(LED3);
	BSP_LED_Init(LED4);		BSP_LED_Off(LED4);

#if	(REAL_HARDWARE)
	/* initialisation de l'état des LEDs */
	RND_EXP_SetLed(0);
#endif

	/* create led behavior commadn queue */
	ledQueue = xQueueCreate( 4, sizeof( t_LED_Cmd *));
	if( ledQueue == NULL)
		return E_ERROR;

	/* create led task */
	if( xTaskCreate( LedTask, "LED", 2048/4, (void *)1, _priority_fix(osPriorityBelowNormal), &ledTaskHandle) != pdPASS)
		return E_ERROR;

	return E_OK;
}

/*******************************************************************************
 * Function     : RND_LedSet
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
//static void
//RND_LedSet( uint8_t val)
//{
//	val &= 0x0f;
//	(val & (1<<0)) ? BSP_LED_On(LED1) : BSP_LED_Off(LED1);
//	(val & (1<<1)) ? BSP_LED_On(LED2) : BSP_LED_Off(LED2);
//	(val & (1<<2)) ? BSP_LED_On(LED3) : BSP_LED_Off(LED3);
//	(val & (1<<3)) ? BSP_LED_On(LED4) : BSP_LED_Off(LED4);
//}

/*******************************************************************************
 * Function     :
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/


/*** End Of File ***/
