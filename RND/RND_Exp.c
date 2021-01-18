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
static bool initialized = FALSE;
#define hi2c	hi2c1

/*******************************************************************************
 * Globals
 *******************************************************************************/

/*******************************************************************************
 * Externals
 *******************************************************************************/
extern I2C_HandleTypeDef hi2c1;

/*******************************************************************************
 * Function     : _exp_probe
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
static t_return
_exp_probe( void)
{
	t_return ret = E_ERROR;
	HAL_StatusTypeDef hret;

	hret = HAL_I2C_IsDeviceReady(&hi2c, PCAL6408_I2C_ADD, 5, PCAL6408_TIMEOUT);
	switch (hret)
	{
		case HAL_TIMEOUT:
			ret = E_ERROR;
			break;

		case HAL_OK:
			ret = E_OK;
			break;

		case HAL_ERROR:
		case HAL_BUSY:
		default:
			/* should reset the i2c bus and signal the error */
			ret = E_ERROR;
		break;
	}

	return ret;
}

/*******************************************************************************
 * Function     : _exp_write_reg
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
static t_return
_exp_write_reg( uint8_t reg, uint8_t val)
{
	t_return ret = E_ERROR;

	if (HAL_I2C_Mem_Write(&hi2c, PCAL6408_I2C_ADD, (uint16_t) reg,
			I2C_MEMADD_SIZE_8BIT, (uint8_t*) &val, 1, PCAL6408_TIMEOUT) == HAL_OK)
		ret = E_OK;
	else
		RND_I2C_ErrorHandler();

	return ret;
}

/*******************************************************************************
 * Function     : _exp_read_reg
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
static t_return
_exp_read_reg( uint8_t reg, uint8_t *val)
{
	t_return ret = E_ERROR;

	if (HAL_I2C_Mem_Read(&hi2c, PCAL6408_I2C_ADD, (uint16_t) reg,
			I2C_MEMADD_SIZE_8BIT, val, 1, PCAL6408_TIMEOUT) == HAL_OK)
		ret = E_OK;
	else
		RND_I2C_ErrorHandler();

	return ret;
}

/*******************************************************************************
 * Function     : RND_EXP_Init
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
t_return
RND_EXP_Init( void)
{
t_return ret = E_ERROR;

	/* take i2c bus ownership */
	if (RND_I2C_GetAccess() == E_OK)
	{
		/* test expander presence */
		if (_exp_probe() == E_OK)
		{
			/* device is there, configure it */
			/* push pull outputs type */
			if (_exp_write_reg( PCAL6408_OUT_CONF, 0) != E_OK)
				return E_ERROR;

			/* all outputs default to low */
			if (_exp_write_reg( PCAL6408_OUTPUT_PORT, 0) != E_OK)
				return E_ERROR;

			/* all IOs to outputs */
			if (_exp_write_reg( PCAL6408_CONF, 0) != E_OK)
				return E_ERROR;

			initialized = TRUE;
			ret = E_OK;
		}
		else
		{
			LOG("I2C Expander not found\n");
		}

		RND_I2C_GiveAccess();
	}
	else
	{
		LOG("Failed to take I2C ownership\n");
	}

	return ret;
}

/*******************************************************************************
 * Function     : RND_EXP_SetLed
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
t_return
RND_EXP_SetLed( uint8_t ledsts)
{
uint8_t	val;
t_return ret = E_ERROR;

	ledsts &= 0x1f;

	if (RND_I2C_GetAccess() == E_OK)
	{
		if( _exp_read_reg( PCAL6408_OUTPUT_PORT, &val) == E_OK)
		{
			val &= 0xE0;
			val |= ledsts;
			if( _exp_write_reg( PCAL6408_OUTPUT_PORT, val) == E_OK)
				ret = E_OK;
		}
		RND_I2C_GiveAccess();
	}
	else
	{
		LOG("Failed to take I2C ownership\n");
	}

	return ret;
}

/*******************************************************************************
 * Function     : RND_EXP_SetBuzzer
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
t_return
RND_EXP_SetBuzzer( bool state)
{
uint8_t val;
t_return ret = E_ERROR;

	if (RND_I2C_GetAccess() == E_OK)
	{
		if( _exp_read_reg( PCAL6408_OUTPUT_PORT, &val) == E_OK)
		{
			val &= 0xDF;
			val |= (state == TRUE) ? (1<<5) : 0;
			if( _exp_write_reg( PCAL6408_OUTPUT_PORT, val) == E_OK)
				ret = E_OK;
		}
		RND_I2C_GiveAccess();
	}
	else
	{
		LOG("Failed to take I2C ownership\n");
	}

	return ret;
}

/*******************************************************************************
 * Function     : RND_EXP_Beep
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
t_return
RND_EXP_Beep( void)
{
uint8_t val;
t_return ret = E_ERROR;

	if (RND_I2C_GetAccess() == E_OK)
	{
		if( _exp_read_reg( PCAL6408_OUTPUT_PORT, &val) == E_OK)
		{
			val &= 0xDF;
			val |= (1<<5);
			if( _exp_write_reg( PCAL6408_OUTPUT_PORT, val) == E_OK)
			{
				//osDelay(1);
				DWT_Delay_us(300);
				if (_exp_read_reg( PCAL6408_OUTPUT_PORT, &val) == E_OK)
				{
					val &= 0xDF;
					if( _exp_write_reg( PCAL6408_OUTPUT_PORT, val) == E_OK)
					{
						ret = E_OK;
					}
				}
			}
		}
		RND_I2C_GiveAccess();
	}
	else
	{
		LOG("Failed to take I2C ownership\n");
	}

	return ret;
}

/*** End Of File ***/
