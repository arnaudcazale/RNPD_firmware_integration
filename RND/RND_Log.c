/*******************************************************************************
 * RUNPAD     			                                                       *
 *******************************************************************************
 * Project:		RUNPAD V2
 * Component:	Prototype
 * Element:
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
SemaphoreHandle_t	logMutex = NULL;
QueueHandle_t		logQueue = NULL;
QueueHandle_t		getcQueue = NULL;
static	char		log_buff[LOG_QUEUE_DEPTH][LOG_MSG_MAX_LENGTH];
static TaskHandle_t	logTaskHandle = NULL;

/*******************************************************************************
 * Globals
 *******************************************************************************/

/*******************************************************************************
 * Externals
 *******************************************************************************/

/*******************************************************************************
 * Function     : RND_Log_Putchar
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
void
RND_Log_Putchar( uint8_t c)
{
	LL_USART_TransmitData8( DBG_UART, c);
	while( ! LL_USART_IsActiveFlag_TXE( DBG_UART));
}

/*******************************************************************************
 * Function     : RND_Log_Getchar
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
uint8_t
RND_Log_Getchar( void)
{
uint8_t c;

	xQueueReceive( getcQueue, &c, osWaitForever);
	return c;
}

/*******************************************************************************
 * Function     : RND_Log_Getchar_t
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
t_return
RND_Log_Getchar_t( uint32_t timeout, char *cval)
{
uint8_t c;

	if( xQueueReceive( getcQueue, &c, timeout) == pdPASS)
	{
		*cval = c;
		return E_OK;
	}
	else
		return E_TIMEOUT;
}

/*******************************************************************************
 * Function     : RND_Log_Putword
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
void
RND_Log_Putword( uint16_t w)
{
	while(!LL_USART_IsActiveFlag_TXE(DBG_UART));
	LL_USART_TransmitData8( DBG_UART, w );
	while( ! LL_USART_IsActiveFlag_TXE( DBG_UART));
	LL_USART_TransmitData8( DBG_UART, w >> 8);
}

/*******************************************************************************
 * Function     : RND_Log_Puts
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
void
RND_Log_Puts( char *msg)
{
char *p = msg;

	if(p)
	{
		while( *p)
		{
			/*
			LL_USART_TransmitData8( DBG_UART, *p++);
			while( ! LL_USART_IsActiveFlag_TXE( DBG_UART));
			*/
			if( *p == '\n')
				RND_Log_Putchar('\r');
			RND_Log_Putchar(*p++);
		}
	}
}

/*******************************************************************************
 * Function     : RND_Log_Task
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
void
RND_Log_Task( void  *arg)
{
char *msg;

	(void)arg;

	while( 1)
	{
		if( (logMutex == NULL) ||
			(logQueue == NULL) )
		{
			osDelay( SECOND);
			continue;
		}

		if( xQueueReceive( logQueue, &msg, osWaitForever) == pdPASS)
		{
			if( msg)
			{
				/* send message to logging media */
				RND_Log_Puts( msg);

				/* when using mcurses lib */
				//RND_Cmd_Puts( msg);
			}
		}
	}
}

/*******************************************************************************
 * Function     : RND_msg_log
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
t_return
RND_msg_log( const char *fct, const int line, const char *fmt, ...)
{
va_list	ap;
static	uint8_t	index;
char *p;
t_return	ret = E_ERROR;

	if( xSemaphoreTake( logMutex, osWaitForever) != pdTRUE)
		return ret;

	p = &log_buff[index][0];
	memset( p, 0, LOG_MSG_MAX_LENGTH);

	/* on construit le message */
	va_start( ap, fmt);

#ifdef	TIMESTAMP_MESSAGES
	sprintf( p, "%.8ld: %s@%d(): ", (long)RND_RTC_Get_Rel_TimeStamp(), fct, line);
#else
	sprintf( p, "%s@%d(): ", fct, line);
#endif
	vsnprintf( &p[strlen(p)], LOG_MSG_MAX_LENGTH, fmt, ap);
	va_end(ap);

	/* on envoi le message */
	if( xQueueSend( logQueue, &p, 100) == errQUEUE_FULL)
		ret = E_ERROR;
	else
	{
		index ++;
		index %= LOG_QUEUE_DEPTH;
		ret = E_OK;
	}

	xSemaphoreGive( logMutex);

	return ret;
}

/*******************************************************************************
 * Function     : RND_LogInIrq
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
void
RND_LogInIrq( void)
{
uint8_t	c;

	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	while( LL_USART_IsActiveFlag_RXNE( DBG_UART))
	{
		/* lecture du caractère */
		c = LL_USART_ReceiveData8( DBG_UART);

		if( getcQueue)
			xQueueSendFromISR( getcQueue, &c, &xHigherPriorityTaskWoken);
	}

	if( LL_USART_IsActiveFlag_ORE( DBG_UART))
		LL_USART_ClearFlag_ORE( DBG_UART);

	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	return;
}

/*******************************************************************************
 * Function     : RND_LogInit
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
t_return
RND_LogInit( void)
{
	/* création mutex */
	logMutex = xSemaphoreCreateMutex();
	if( !logMutex)
		return E_ERROR;

	/* output (to screen) queue */
	logQueue = xQueueCreate( LOG_QUEUE_DEPTH, sizeof( char *));
	if( !logQueue)
		return E_ERROR;

	vQueueAddToRegistry( logQueue, "logOutQueue");

	/* input (from screen) queue */
	getcQueue = xQueueCreate( LOG_GETC_QUEUE_DEPTH, sizeof( char));
	if( !getcQueue)
		return E_ERROR;

	vQueueAddToRegistry( getcQueue, "logInQueue");

	/* enable input interrupt */
	LL_USART_Enable( DBG_UART);
	LL_USART_EnableIT_RXNE( DBG_UART);

	/* on lance la tâche de gestion des logs */
	if( xTaskCreate( RND_Log_Task, "Logging", 512/4, (void *)1, _priority_fix( osPriorityBelowNormal), &logTaskHandle) != pdPASS)
		return E_ERROR;

	return E_OK;
}

/*******************************************************************************
 * Function     : 
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
/*******************************************************************************
 * Function     : RND_SEQ_Sequencer
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
void
RND_send_UART_full_matrix( matrix_full_t *data )
{
	uint16_t STARTMARKER = 0xFFFE;
	uint16_t STOPMARKER = 0xFFFF;

	RND_Log_Putword(STARTMARKER);

	for( uint16_t i = 0; i < TOTAL_LINES; i++)
	{
		for( uint16_t j = 0; j < TOTAL_COL; j++)
		{
			RND_Log_Putword(data->left[i][j]);
		}
		//RND_Log_Putword(data->left[i]);
	}

	for( uint16_t i = 0; i < TOTAL_LINES; i++)
	{
		for( uint16_t j = 0; j < TOTAL_COL; j++)
		{
			RND_Log_Putword(data->right[i][j]);
		}
		//RND_Log_Putword(data->right[i]);
	}

	RND_Log_Putword(STOPMARKER);

	RND_Log_Putchar('\n');

}

/*******************************************************************************
 * Function     : RND_SEQ_Sequencer
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
void
RND_send_UART_side_bin( matrix_bin_t *data )
{
	uint16_t STARTMARKER = 0xFFFE;
	uint16_t STOPMARKER = 0xFFFF;

	RND_Log_Putword(STARTMARKER);

	for( uint16_t i = 0; i < TOTAL_LINES; i++)
	{
		for( uint16_t j = 0; j < TOTAL_COL; j++)
		{
			RND_Log_Putword(*data[i][j]);
		}
		//RND_Log_Putword(data->left[i]);
	}

	RND_Log_Putword(STOPMARKER);

	RND_Log_Putchar('\n');

}

/*** End Of File ***/
