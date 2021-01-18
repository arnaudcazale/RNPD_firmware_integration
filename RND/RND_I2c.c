/*******************************************************************************
 * RUNPAD     			                                                       *
 *******************************************************************************
 * Project:		RUNPAD V2
 * Component:	Prototype
 * Element:		Gestion i2c
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
static 	bool				initialized = FALSE;
static	SemaphoreHandle_t	i2c_Mutex;

/*******************************************************************************
 * Globals
 *******************************************************************************/

/*******************************************************************************
 * Externals
 *******************************************************************************/

/*******************************************************************************
 * Function     : RND_I2C_Init
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
t_return
RND_I2C_Init( void)
{
	if( initialized == FALSE)
	{
		/* create mutex */
		i2c_Mutex = xSemaphoreCreateMutex();
		//i2c_Mutex = xSemaphoreCreateBinary();
		if( !i2c_Mutex)
		{
			LOG("Failed to create i2c mutex\n");
			return E_ERROR;
		}

		initialized = TRUE;
	}
	else
	{
		LOG("I2c already initialized\n");
		return E_ERROR;
	}

	return E_OK;
}

/*******************************************************************************
 * Function     : RND_I2C_GetAccess
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
t_return
RND_I2C_GetAccess(void)
{
	if (initialized != TRUE)
		return E_ERROR;

	if ( xSemaphoreTake( i2c_Mutex, I2C_TIMEOUT) != pdTRUE)
	{
		LOG("Failed to take I2C Semaphore\n");
		return E_ERROR;
	}
	return E_OK;
}

/*******************************************************************************
 * Function     : RND_I2C_GiveAcess
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
t_return
RND_I2C_GiveAccess( void)
{
	if (initialized != TRUE)
		return E_ERROR;

	xSemaphoreGive(i2c_Mutex);
	return E_OK;
}

/*******************************************************************************
 * Function     : RND_I2C_ErrorHandler
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
void
RND_I2C_ErrorHandler( void)
{
	LOG("I2C error\n");

//	while(TRUE)
//	{
//		LOG("I2C error\n");
//		osDelay(1000);
//	}
	return;
}

/*** End Of File ***/
