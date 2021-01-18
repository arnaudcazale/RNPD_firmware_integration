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
static t_simple_matrix 	mats[5];
static t_simple_matrix	mats_mean;
static t_simple_matrix	mats_acq;

static void	_transpose_red_to_simple(t_simple_matrix *mat);
static void	_calc_simple_mean		(t_simple_matrix *mean);
static void	_copy_simple_mat		(t_simple_matrix *src, t_simple_matrix *dest);
static bool	_compare_simple_mat		(t_simple_matrix *m1, t_simple_matrix *m2, uint16_t thres);

/*******************************************************************************
 * Globals
 *******************************************************************************/

/*******************************************************************************
 * Externals
 *******************************************************************************/
extern  uint16_t right_sensor[ TOTAL_SENSORS_PER_SIDE];
extern  uint16_t left_sensor [ TOTAL_SENSORS_PER_SIDE];

/* reduced acquisition matrix */
extern  uint16_t right_sensor_red[ TOTAL_SENSORS_PER_SIDE / 4];
extern  uint16_t left_sensor_red [ TOTAL_SENSORS_PER_SIDE / 4];

/*******************************************************************************
 * Function     : RND_Calc_Normalise
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
__OPTIMIZE_START
void
RND_Calc_Normalise( void)
{
uint16_t	max_val = 0;
register int i;

	/* get max value */
	for( i = 0; i < TOTAL_SENSORS_PER_SIDE; i++)
	{
//		left_sensor[i]  = left_sensor[i]  >> 2;
//		right_sensor[i] = right_sensor[i] >> 2;

		if( left_sensor[i] > max_val)
			max_val = left_sensor[i];
		if( right_sensor[i] > max_val)
			max_val = right_sensor[i];
	}

	//LOG("Max value = 0x%.4x\n", max_val);

	/* normalize */
	for( i = 0; i < TOTAL_SENSORS_PER_SIDE; i++)
	{
		left_sensor[i] = (uint16_t)(((float)left_sensor[i] / (float)max_val) * 4095.0);
		right_sensor[i] = (uint16_t)(((float)right_sensor[i] / (float)max_val) * 4095.0);
	}

	return;
}

/*******************************************************************************
 * Function     : RND_Calc_Global_Mean
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
uint16_t
RND_Calc_Global_Mean( t_ACQ_Type type)
{
double mean = 0.0;
register int i;

	switch( type)
	{
	case t_ACQ_FULL:
		for( i = 0; i < TOTAL_SENSORS_PER_SIDE; i++)
			mean += ((double)left_sensor[i] + (double)right_sensor[i]);
		mean /= (double)TOTAL_SENSORS;
		break;

	case t_ACQ_REDUCED:
		for( i = 0; i < TOTAL_SENSORS_RED_PER_SIDE; i++)
			mean += ((double)left_sensor_red[i] + (double)right_sensor_red[i]);
		mean /= (double)192;
		break;

	default:
		break;
	}

	return( (uint16_t)mean);
}

/*******************************************************************************
 * Function     : RND_Calc_IsStable
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
bool
RND_Calc_IsStable( uint16_t iter_limit)
{
static	uint16_t stable_counter = 0;

	if( RND_Calc_MvtDetect() == FALSE)
		stable_counter ++;
	else
		stable_counter = 0;

	if( stable_counter > iter_limit)
	{
		stable_counter = 0;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/*******************************************************************************
 * Function     : RND_Calc_MvtDetect
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
bool
RND_Calc_MvtDetect( void)
{
static uint8_t mat_index = 0;
bool ret = FALSE;

	/* acquired reduced matrix */
	RND_Acq_Frame( t_ACQ_REDUCED);

	/* convert so a simplefied matrix */
	_transpose_red_to_simple( &mats_acq);

	/* compare simplified matrix to means matrix */
	if( _compare_simple_mat( &mats_acq, &mats_mean, 40) == TRUE)
	{
		LOG("Mouvement detected\n");
		ret = TRUE;
	}

	/* copy current to mem */
	_copy_simple_mat( &mats_acq, &mats[mat_index]);
	mat_index ++;
	mat_index %= 5;

	/* recalculate means */
	_calc_simple_mean( &mats_mean);
	return ret;
}

/*******************************************************************************
 * Function     : RND_Calc_IsUserOn
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
bool
RND_Calc_IsUserOn( void)
{
	RND_Acq_Frame(t_ACQ_REDUCED);
	if( RND_Calc_Global_Mean(t_ACQ_REDUCED) > 30)
		return TRUE;
	else
		return FALSE;
}

/*******************************************************************************
 * Function     : RND_Calc_Transpose_Red_Simple
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
static void
_transpose_red_to_simple( t_simple_matrix *mat)
{
register uint16_t line, col;
uint16_t sum = 0;

	for( uint16_t i = 0; i < TOTAL_SENSORS_RED_PER_SIDE; )
	{
		sum += left_sensor_red[i++];
		sum += left_sensor_red[i++];
		sum += left_sensor_red[i++];
		sum += left_sensor_red[i++];
		sum /= 4;

		line = (uint16_t)( (i-1) / 8);
		col  = (uint16_t)(((i-1) % 8) / 4);

		mat->left[line][col] = sum;
		sum = 0;
	}

	for( uint16_t i = 0; i < TOTAL_SENSORS_RED_PER_SIDE; )
	{
		sum += right_sensor_red[i++];
		sum += right_sensor_red[i++];
		sum += right_sensor_red[i++];
		sum += right_sensor_red[i++];
		sum /= 4;

		line = (uint16_t)( (i-1) / 8);
		col  = (uint16_t)(((i-1) % 8) / 4);

		mat->right[line][col] = sum;
		sum = 0;
	}

	return;
}

/*******************************************************************************
 * Function     : RND_Calc_MatSimpleMean
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
static void
_calc_simple_mean( t_simple_matrix *mean)
{
	memset( (void *)mean, 0, sizeof(t_simple_matrix));

	for( uint8_t i = 0; i < 12; i++)
	{
		for( uint8_t k = 0; k < 5; k ++)
		{
			mean->left[i][0] += mats[k].left[i][0];
			mean->left[i][1] += mats[k].left[i][1];
			mean->right[i][0] += mats[k].right[i][0];
			mean->right[i][1] += mats[k].right[i][1];
		}

		mean->left[i][0] /= 5;
		mean->left[i][1] /= 5;
		mean->right[i][0] /= 5;
		mean->right[i][1] /= 5;
	}
}

/*******************************************************************************
 * Function     : _copy_simple_mat
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
static void
_copy_simple_mat( t_simple_matrix *src, t_simple_matrix *dest)
{
	for( uint8_t i = 0; i < 12; i++)
	{
		dest->left[i][0]  = src->left[i][0];
		dest->left[i][1]  = src->left[i][1];
		dest->right[i][0] = src->right[i][0];
		dest->right[i][1] = src->right[i][1];
	}
}

/*******************************************************************************
 * Function     : _compare_simple_mat
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
static bool
_compare_simple_mat( t_simple_matrix *m1, t_simple_matrix *m2, uint16_t thres)
{
	for( uint8_t i = 0; i < 12; i++)
	{
		if( abs( m1->left[i][0]  - m2->left[i][0]) > thres)		return TRUE;
		if( abs( m1->left[i][1]  - m2->left[i][1]) > thres)		return TRUE;
		if( abs( m1->right[i][0] - m2->right[i][0]) > thres)	return TRUE;
		if( abs( m1->right[i][1] - m2->right[i][1]) > thres)	return TRUE;
	}

	return FALSE;
}

__OPTIMIZE_END

/*** End Of File ***/
