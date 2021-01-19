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
#include "adc.h"
#include "dma.h"

/*******************************************************************************
 * Locals
 *******************************************************************************/

/* full acquisition matrix */
/*_IN_DATA_RAM*/
uint16_t right_sensor[ TOTAL_SENSORS_PER_SIDE];
uint16_t left_sensor [ TOTAL_SENSORS_PER_SIDE];

/* reduced acquisition matrix */
uint16_t right_sensor_red[ TOTAL_SENSORS_PER_SIDE / 4];
uint16_t left_sensor_red [ TOTAL_SENSORS_PER_SIDE / 4];

static uint32_t m_right_sensor[ TOTAL_SENSORS_PER_SIDE];
static uint32_t m_left_sensor [ TOTAL_SENSORS_PER_SIDE];

static uint16_t	sensor_id = 0;

#if	(REMOVE_OFFSET == 1)
static t_acq		offset;
#endif

/* current acquisition type */
static t_ACQ_Type	Current_Acq_Type = t_ACQ_FULL;

/* semaphore to signal end of acquisition */
static SemaphoreHandle_t	acq_end_sem = NULL;

/*******************************************************************************
 * Globals
 *******************************************************************************/

/*******************************************************************************
 * Externals
 *******************************************************************************/
extern ADC_HandleTypeDef hadc1;
extern DAC_HandleTypeDef hdac;

/*******************************************************************************
 * Function     : RND_ACQ_Init
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
static t_return
RND_ACQ_Init( void)
{
static bool init = FALSE;

	/* initialize acquisition buffers */
	switch( Current_Acq_Type)
	{
	case t_ACQ_FULL:
		memset( (void *)right_sensor, 0xff, sizeof( right_sensor));
		memset( (void *)left_sensor, 0xff, sizeof( left_sensor));
		break;

	case t_ACQ_REDUCED:
		memset( (void *)right_sensor_red, 0xff, sizeof( right_sensor_red));
		memset( (void *)left_sensor_red, 0xff, sizeof( left_sensor_red));
		break;

	default:
		break;
	}

	/* initialize DMA and ADC engines */
	if( init == FALSE)
	{
		MX_DMA_Init();
		MX_ADC1_Init();
		init = TRUE;
	}

	return E_OK;
}

/*******************************************************************************
 * Function     : RND_ACQ_Start
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
static t_return
RND_ACQ_Start( void)
{
t_return ret = E_ERROR;

	/* start by sensor 0 */
	sensor_id = 0;

	/* activate DAC output to 3.3V */
	HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 0xfff);

	/* activate line 0 and colgroup 0 */
	RND_MAT_Select_Line(0);
	RND_MAT_Select_Columns(0);

	/* wait 10 µs */
	//	DWT_Delay_us( 100);
	HAL_Delay(3);

	/* start acquisition */
	switch( Current_Acq_Type)
	{
	case t_ACQ_FULL:
		HAL_ADC_Start_DMA(&hadc1, (uint32_t*) left_sensor, 4);
		break;

	case t_ACQ_REDUCED:
		HAL_ADC_Start_DMA(&hadc1, (uint32_t*) left_sensor_red, 4);
		break;

	default:
		break;
	}

	ret = E_OK;
	return ret;
}

/*******************************************************************************
 * Function     : RND_ACQ_Wait_End
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
static t_return
RND_ACQ_Wait_End( void)
{
	if( xSemaphoreTake( acq_end_sem, osWaitForever) == pdPASS)
	{
		/* zero DAC */
		HAL_DAC_SetValue( &hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 0);
		return E_OK;
	}
	else
		return E_ERROR;
}

/*******************************************************************************
 * Function     : HAL_ADC_ErrorCallback
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
void
HAL_ADC_ErrorCallback( ADC_HandleTypeDef* hadc)
{
	//while(1)	osDelay(1);
}

/*******************************************************************************
 * Function     : RND_ACQ_Init
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
void
HAL_ADC_ConvCpltCallback( ADC_HandleTypeDef *hadc)
{
uint8_t line = 0;
uint8_t colgroup = 0;
BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if( hadc == &hadc1)
	{
		/* force stop current DMA */
		hadc1.DMA_Handle->State = HAL_DMA_STATE_BUSY;
		HAL_ADC_Stop_DMA( &hadc1);

		if( Current_Acq_Type == t_ACQ_FULL)
		{
			/* next sensor */
			sensor_id += 4;

			if( sensor_id < TOTAL_SENSORS)
			{
				/* get line and column group and set phys lines */
				line     = LINE_FROM_INDEX( sensor_id);
				colgroup = COL_GROUP_FROM_INDEX( sensor_id);
				RND_MAT_Select_Line( line);
				RND_MAT_Select_Columns( colgroup);

				/* wait proper amount of time for signal to settle */
	//			DWT_Delay_us(100);
				HAL_Delay(3);

				if (sensor_id < TOTAL_SENSORS_PER_SIDE)
				{
					/* start acquisition on left side */
					HAL_ADC_Start_DMA( &hadc1, (uint32_t*) (&left_sensor[sensor_id]), 4);
				}
				else
				{
					/* start acquisition on right side */
					HAL_ADC_Start_DMA( &hadc1, (uint32_t*) (&right_sensor[sensor_id - TOTAL_SENSORS_PER_SIDE]), 4);
				}
			}
			else
			{
				/* signal end of acquisition */
				xSemaphoreGiveFromISR( acq_end_sem, &xHigherPriorityTaskWoken);
			}
		}
		else if (Current_Acq_Type == t_ACQ_REDUCED)
		{
			sensor_id += 4;

			if( sensor_id < TOTAL_SENSORS_RED)
			{
				uint16_t real_sensor_id = ((uint16_t)(sensor_id / 8) * 32) + (sensor_id % 8);

				line     = LINE_FROM_INDEX( real_sensor_id);
				colgroup = COL_GROUP_FROM_INDEX( real_sensor_id);
				RND_MAT_Select_Line( line);
				RND_MAT_Select_Columns( colgroup);

				/* wait proper amount of time for signal to settle */
				//DWT_Delay_us(100);
				HAL_Delay(1);

				if( sensor_id < TOTAL_SENSORS_RED_PER_SIDE)
				{
					HAL_ADC_Start_DMA( &hadc1, (uint32_t*) (&left_sensor_red[sensor_id]), 4);
				}
				else
				{
					HAL_ADC_Start_DMA( &hadc1, (uint32_t*) (&right_sensor_red[sensor_id - TOTAL_SENSORS_RED_PER_SIDE]), 4);
				}
			}
			else
			{
				/* signal end of acquisition */
				xSemaphoreGiveFromISR( acq_end_sem, &xHigherPriorityTaskWoken);
			}
		}
	}

	return;
}

/*******************************************************************************
 * Function     : RND_Acq_Frame
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
t_return
RND_Acq_Frame( t_ACQ_Type type)
{
t_return ret = E_ERROR;

	/* register acquisition type */
	Current_Acq_Type = type;

	/* Acquire one frame */
	RND_ACQ_Init();

	if( RND_ACQ_Start() == E_OK)
	{
		if( RND_ACQ_Wait_End() == E_OK)
			ret = E_OK;
	}

	return ret;
}

#if (OPTIMIZE==1)
__OPTIMIZE_START
#endif
/*******************************************************************************
 * Function     : RND_Acq_Multiple
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
t_return
RND_Acq_Multiple( t_acq *p, uint8_t n_acq)
{
t_return ret = E_ERROR;

	if( n_acq > 20)
		return ret;

	memset( (void *)m_right_sensor, 0, sizeof( m_right_sensor));
	memset( (void *)m_left_sensor, 0, sizeof( m_left_sensor));

	/* full matrix acquisition */
	for( uint8_t i = 0; i < n_acq; i++)
	{
		LOG("Acquiring frame %d\n", i);
		RND_EXP_Beep();
		if( RND_Acq_Frame( t_ACQ_FULL) == E_OK)
		{
			for( uint16_t k = 0; k < TOTAL_SENSORS_PER_SIDE; k++)
			{
				m_right_sensor[k] += (uint32_t)right_sensor[k];
				m_left_sensor[k]  += (uint32_t)left_sensor[k];
			}
		}
		else
		{
			LOG("Acquisition %d failed\n", i);
			goto end;
		}
	}

	/* leverage */
	for( uint16_t k = 0; k < TOTAL_SENSORS_PER_SIDE; k++)
	{
		m_right_sensor[k] /= n_acq;
		m_left_sensor[k] /= n_acq;
	}

#if (REMOVE_OFFSET == 1)
	/* remove offset */
	for( uint16_t k = 0; k < TOTAL_SENSORS_PER_SIDE; k++)
	{
//		m_left_sensor[k] -= offset.left[k];
//		m_right_sensor[k] -= offset.right[k];

		m_left_sensor[k]  = (offset.left[k]  > m_left_sensor[k])  ? 0 : m_left_sensor[k]  - offset.left[k];
		m_right_sensor[k] = (offset.right[k] > m_right_sensor[k]) ? 0 : m_right_sensor[k] - offset.right[k];
	}
#endif

	/* copy back to output buffer */
	for( uint16_t k = 0; k < TOTAL_SENSORS_PER_SIDE; k++)
	{
		(*p).left[k]  = (uint16_t)m_left_sensor[k];
		(*p).right[k] = (uint16_t)m_right_sensor[k];
	}

	ret = E_OK;

end:
	return ret;
}

/*******************************************************************************
 * Function     : RND_Acq_Multiple_Start
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
t_return
RND_Acq_Multiple_Start( uint8_t n_acq)
{
t_return ret = E_ERROR;

	if( n_acq > 20)
		return ret;

	/* clear internal destination */
	memset( (void *)m_right_sensor, 0, sizeof( m_right_sensor));
	memset( (void *)m_left_sensor, 0, sizeof( m_left_sensor));

	/* full matrix acquisition */
	for( uint8_t i = 0; i < n_acq; i++)
	{
		LOG("Acquiring frame %d\n", i);
		RND_EXP_Beep();
		if( RND_Acq_Frame( t_ACQ_FULL) == E_OK)
		{
			for( uint16_t k = 0; k < TOTAL_SENSORS_PER_SIDE; k++)
			{
				m_right_sensor[k] += (uint32_t)right_sensor[k];
				m_left_sensor[k]  += (uint32_t)left_sensor[k];
			}
		}
		else
		{
			LOG("Acquisition %d failed\n", i);
			goto end;
		}
	}

end:
	return ret;
}

/*******************************************************************************
 * Function     : RND_Acq_Multiple_End
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
t_return
RND_Acq_Multiple_End( t_acq *p, uint8_t acq_done, uint8_t total_acq)
{
t_return ret = E_ERROR;

	if( (total_acq > 20) || (acq_done > 20) || (acq_done > total_acq) )
		return ret;

	for( uint8_t i = acq_done; i < total_acq; i++)
	{
		LOG("Acquiring frame %d\n", i);
		RND_EXP_Beep();
		if( RND_Acq_Frame( t_ACQ_FULL) == E_OK)
		{
			for( uint16_t k = 0; k < TOTAL_SENSORS_PER_SIDE; k++)
			{
				m_right_sensor[k] += (uint32_t)right_sensor[k];
				m_left_sensor[k]  += (uint32_t)left_sensor[k];
			}
		}
		else
		{
			LOG("Acquisition %d failed\n", i);
			goto end;
		}
	}

	/* leverage */
	for( uint16_t k = 0; k < TOTAL_SENSORS_PER_SIDE; k++)
	{
		m_right_sensor[k] /= total_acq;
		m_left_sensor[k] /= total_acq;
	}

	/* copy back to output buffer */
	for( uint16_t k = 0; k < TOTAL_SENSORS_PER_SIDE; k++)
	{
		(*p).left[k] = (uint16_t)m_left_sensor[k];
		(*p).right[k] = (uint16_t)m_right_sensor[k];
	}

	ret = E_OK;

end:
	return ret;
}

#if (OPTIMIZE==1)
__OPTIMIZE_END
#endif

/*******************************************************************************
 * Function     : RND_Calc_Init
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
t_return
RND_Calc_Init( void)
{
	/* create semaphore to signal acquisition end */
	acq_end_sem = xSemaphoreCreateBinary();
	if( acq_end_sem == NULL)
		return E_ERROR;

	/* Start DAC */
	HAL_DAC_Start( &hdac, DAC_CHANNEL_1);
	HAL_DAC_SetValue( &hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 0);

#if	(REMOVE_OFFSET == 1)
	/* acquire offset matrix */
	memset( (void *)offset.left, 0, sizeof(offset.left));
	memset( (void *)offset.right, 0, sizeof(offset.right));
	if( RND_Acq_Multiple( &offset, 5) != E_OK)
	{
		LOG("Failed to acquire offset matrix\n");
		return E_ERROR;
	}
#endif

	return E_OK;
}


/*******************************************************************************
 * Function     :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
t_return
RND_Fill_Dead_Pix (t_acq *data, matrix_full_t *matrix)
{

	bool toggle = FALSE;

	for(int i = 0; i < TOTAL_LINES ; i++)
	{
		if(!toggle)
		{
			for(int j = 0; j < (TOTAL_COL/2)-1; j++)
			{
				matrix->left[i][j*2] = data->left[(i* (TOTAL_COL/2) ) + j];
				matrix->left[i][(j*2)+1] = 0;
			}
			toggle = TRUE;
		}else
		{
			for(int j = 0; j < (TOTAL_COL/2)-1; j++)
			{
				matrix->left[i][j*2] = 0;
				matrix->left[i][(j*2)+1] = data->left[(i* (TOTAL_COL/2) ) + j];
			}
			toggle = FALSE;
		}
	}

	for(int i = 0; i < TOTAL_LINES ; i++)
	{
		if(!toggle)
		{
			for(int j = 0; j < (TOTAL_COL/2)-1; j++)
			{
				matrix->right[i][j*2] = 0;
				matrix->right[i][(j*2)+1] = data->right[(i* (TOTAL_COL/2) ) + j];
			}
			toggle = TRUE;
		}else
		{
			for(int j = 0; j < (TOTAL_COL/2)-1; j++)
			{
				matrix->right[i][j*2] = data->right[(i* (TOTAL_COL/2) ) + j];
				matrix->right[i][(j*2)+1] = 0;
			}
			toggle = FALSE;
		}
	}

	return E_OK;
}


/*******************************************************************************
 * Function     :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
t_return
RND_Reorder (matrix_full_t *matrix)
{
	matrix_full_t *temp = (matrix_full_t*)malloc(sizeof(matrix_full_t));
	memcpy(temp, matrix, sizeof(matrix_full_t));

	for(uint8_t i = 0; i < TOTAL_LINES; i++)
	{
		for(uint8_t j = 0; j < TOTAL_COL; j++)
		{
			matrix->left[i][j] = temp->left[TOTAL_LINES - 1 - i][j];
			matrix->right[i][j] = temp->right[TOTAL_LINES - 1 - i][j];
		}
	}

	return E_OK;
}

/*******************************************************************************
 * Function     :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
t_return
RND_Fill_Neighboor (matrix_full_t *matrix)
{
	bool toggle = FALSE;
	int mean_data;
	matrix_full_t *temp = (matrix_full_t*)malloc(sizeof(matrix_full_t));
	memcpy(temp, matrix, sizeof(matrix_full_t));

	//LEFT SIDE
	for(int i = 0; i < TOTAL_LINES ; i++)
	{
		if(!toggle)
		{
			for(int j = 0; j < TOTAL_COL; j = j+2)
			{
				//Si 1ere ligne et 1ere colonne
				if( (i == 0) && (j == 0) )
				{
					mean_data = ( temp->left[i+1][j] + temp->left[i][j+1] ) / 2;
					matrix->left[i][j] = mean_data;
				}else if( (i == 0) && (j != 0) )
				{
					mean_data = ( temp->left[i][j-1] + temp->left[i+1][j] + temp->left[i][j+1] ) / 3;
					matrix->left[i][j] = mean_data;

				//Sinon Si 1ere colonne
				}else if( (i != 0) && (j == 0) )
				{
					mean_data = ( temp->left[i+1][j] + temp->left[i-1][j] + temp->left[i][j+1] ) / 3;
					matrix->left[i][j] = mean_data;
				}else if( (i != 0) && (j != 0) )
				{
					mean_data = ( temp->left[i][j-1] + temp->left[i+1][j] + temp->left[i-1][j] + temp->left[i][j+1] ) / 4;
					matrix->left[i][j] = mean_data;
				}
			}

			toggle = TRUE;

		}else
		{
			for(int j = 1; j < TOTAL_COL; j = j+2)
			{
				if( (i == 47) && (j == 15) )
				{
					mean_data = ( temp->left[i][j-1] + temp->left[i-1][j] ) / 2;
					matrix->left[i][j] = mean_data;
				}else if( (i == 47) && (j != 15) )
				{
					mean_data = ( temp->left[i][j-1] + temp->left[i-1][j] + temp->left[i][j+1] ) / 3;
					matrix->left[i][j] = mean_data;

				//Sinon Si dernière colonne
				}else if( (i != 47) && (j == 15) )
				{
					mean_data = ( temp->left[i+1][j] + temp->left[i][j-1] + temp->left[i-1][j] ) / 3;
					matrix->left[i][j] = mean_data;
				}else if( (i != 47) && (j != 15) )
				{
					mean_data = ( temp->left[i][j-1] + temp->left[i+1][j] + temp->left[i-1][j] + temp->left[i][j+1] ) / 4;
					matrix->left[i][j] = mean_data;
				}
			}

			toggle = FALSE;
		}
	}

	//RIGHT SIDE
	for(int i = 0; i < TOTAL_LINES ; i++)
	    {
	        if(!toggle)
	        {
	            //qDebug() << "i"<< i;
	            for(int j = 1; j < TOTAL_COL; j = j+2)
	            {
	                //qDebug() << "j"<< j;
	                //Si 1ere ligne et 1ere colonne
	                if( (i == 0) && (j == 15) )
	                {
	                    mean_data = ( temp->right[i+1][j] + temp->right[i][j-1] ) / 2;
	                    matrix->right[i][j] = mean_data;
	                }else if( (i == 0) && (j != 15) )
	                {
	                    mean_data = ( temp->right[i][j-1] + temp->right[i+1][j] + temp->right[i][j+1] ) / 3;
	                    matrix->right[i][j] = mean_data;

	                //Sinon Si 1ere colonne
	                }else if( (i != 0) && (j == 15) )
	                {
	                    mean_data = ( temp->right[i+1][j] + temp->right[i-1][j] + temp->right[i][j-1] ) / 3;
	                    matrix->right[i][j] = mean_data;
	                }else if( (i != 0) && (j != 15) )
	                {
	                    mean_data = ( temp->right[i][j-1] + temp->right[i+1][j] + temp->right[i-1][j] + temp->right[i][j+1] ) / 4;
	                    matrix->right[i][j] = mean_data;
	                }
	            }

	            toggle = TRUE;

	        }else
	        {
	            for(int j = 0; j < TOTAL_COL; j = j+2)
	            {
	                if( (i == 47) && (j == 0) )
	                {
	                    mean_data = ( temp->right[i][j+1] + temp->right[i-1][j]) / 2;
	                    matrix->right[i][j] = mean_data;
	                }else if( (i == 47) && (j != 0) )
	                {
	                    mean_data = ( temp->right[i][j-1] + temp->right[i-1][j] + temp->right[i][j+1] ) / 3;
	                    matrix->right[i][j] = mean_data;

	                //Sinon Si dernière colonne
	                }else if( (i != 47) && (j == 0) )
	                {
	                    mean_data = ( temp->right[i+1][j] + temp->right[i][j+1] + temp->right[i-1][j] ) / 3;
	                    matrix->right[i][j] = mean_data;
	                }else if( (i != 47) && (j != 0) )
	                {
	                    mean_data = ( temp->right[i][j-1] + temp->right[i+1][j] + temp->right[i-1][j] + temp->right[i][j+1] ) / 4;
	                    matrix->right[i][j] = mean_data;
	                }
	            }

	            toggle = FALSE;
	        }
	    }

	return E_OK;
}


/*** End Of File ***/
