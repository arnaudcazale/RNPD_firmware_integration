/*******************************************************************************
 * RUNPAD                                                              *
 *******************************************************************************
 * Project:		RUNPAD V2
 * Component:	Prototype
 * Element:
 * File:
 * Description: Envoi chaine de caractère à afficher
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
static	char	printBuffers[PRINT_BUFFER_LEN][NB_PRINT_BUFFERS];

/*******************************************************************************
 * Globals
 *******************************************************************************/
QueueHandle_t 	printQueue = NULL;

/*******************************************************************************
 * Externals
 *******************************************************************************/

/*******************************************************************************
 * Function     : RND_Print_Init
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
t_return
RND_Print_Init( void)
{
	/* create print queue */
	printQueue = xQueueCreate( NB_PRINT_BUFFERS, sizeof( char *));
	if( !printQueue)
	{
		LOG("Failed to create queue for printing messages on screen\n");
		return E_ERROR;
	}

	return E_OK;
}

/*******************************************************************************
 * Function     : 
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
t_return
RND_Print( const char *fmt, ...)
{
va_list			ap;
static uint8_t	index = 0;
char 			*p;
t_return		ret = E_ERROR;

	p = &printBuffers[index][0];
	memset( p, 0, PRINT_BUFFER_LEN);

	/* on construit le message */
	va_start( ap, fmt);

	vsnprintf( p, PRINT_BUFFER_LEN, fmt, ap);

	va_end(ap);

//	LOG("asked to print \"%s\"\n", p);

	if (strlen(p))
	{
		/* on l'envoi sur la queue */
		if ( xQueueSend( printQueue, &p, 5 * SECOND) == pdPASS)
		{
			index++;
			index %= NB_PRINT_BUFFERS;
			ret = E_OK;
		}
	}

	return ret;
}

/*** End Of File ***/
