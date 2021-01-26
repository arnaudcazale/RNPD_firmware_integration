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
#include "math.h"


/*******************************************************************************
 * Locals
 *******************************************************************************/
static t_return	_calc_size	(t_sz_data *p);

/*******************************************************************************
 * Globals
 *******************************************************************************/
uint8_t left_bin [TOTAL_LINES][TOTAL_COL/2];
uint8_t right_bin[TOTAL_LINES][TOTAL_COL/2];

uint8_t matrix_left_bin[TOTAL_LINES][TOTAL_COL];
uint8_t matrix_right_bin[TOTAL_LINES][TOTAL_COL];

/*******************************************************************************
 * Externals
 *******************************************************************************/
#if (SIMULATION==1)
extern uint16_t right_sensor_tab[];
extern uint16_t left_sensor_tab[];
#endif

#if (OPTIMIZE==1)
__OPTIMIZE_START
#endif

/*******************************************************************************
 * Function     : 
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
uint32_t
_calc_means( t_acq_tab in)
{
double mean = 0.0;

	for( register int i = 0; i < TOTAL_SENSORS_PER_SIDE; i++)
		mean += (double)( in[ i]);
	mean /= (double)TOTAL_SENSORS_PER_SIDE;

	return (uint32_t)mean;
}

/*******************************************************************************
 * Function     :
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
uint32_t
_calc_mean_matrix( matrix_t in)
{
double mean = 0.0;

	for( register int i = 0; i < TOTAL_LINES; i++)
	{
		for( register int j = 0; j < TOTAL_COL; j++)
		{
			mean += (double)( in[i][j]);
		}
	}
	mean /= (double)TOTAL_LINES*TOTAL_COL;

	return (uint32_t)mean;
}

/*******************************************************************************
 * Function     :
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
t_return
_binarize( t_acq_tab inTab, t_bin_mat outMat)
{
t_acq_mat *p = (t_acq_mat *)inTab;

	/* calculate means of both matrixes */
	uint32_t means = _calc_means( inTab);

	/* make binary matrixes */
	for( uint16_t i = 0; i < TOTAL_LINES; i++)
	{
		for( uint16_t j = 0; j < TOTAL_COL / 2; j++)
		{
			if( (uint32_t)(*p)[i][j] >= means)
				outMat[i][j] = 1;
			else
				outMat[i][j] = 0;
		}
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
_binarize_matrix( matrix_t inTab, matrix_bin_t outMat)
{
	//matrix_t *p = (matrix_t *)inTab;

	/* calculate means of both matrixes */
	uint32_t means = _calc_mean_matrix( inTab);
	LOG("matrix_mean: = %d\n", means);


	/* make binary matrixes */
	for( uint16_t i = 0; i < TOTAL_LINES; i++)
	{
		for( uint16_t j = 0; j < TOTAL_COL; j++)
		{
			if( (uint32_t)inTab[i][j] > means)
				outMat[i][j] = 1;
			else
				outMat[i][j] = 0;
		}
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
_binarize_matrix_from_noise_margin( matrix_t inTab, matrix_bin_t outMat)
{

	/* calculate means of both matrixes */
	uint32_t noise_margin = 100;
	LOG("noise_margin: = %d\n", noise_margin);


	/* make binary matrixes */
	for( uint16_t i = 0; i < TOTAL_LINES; i++)
	{
		for( uint16_t j = 0; j < TOTAL_COL; j++)
		{
			if( (uint32_t)inTab[i][j] > noise_margin)
				outMat[i][j] = 1;
			else
				outMat[i][j] = 0;
		}
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
_clear_data( t_acq_tab inTab)
{
	for( uint16_t i = 0; i < TOTAL_SENSORS_PER_SIDE; i++)
	{
		inTab[i] = 0;
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
_clear_matrix( matrix_t inTab)
{
	for( uint16_t i = 0; i < TOTAL_LINES; i++)
	{
		for( uint16_t j = 0; j < TOTAL_COL; j++)
		{
			inTab[i][j] = 0;
		}
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
_clear_matrix_bin( matrix_bin_t inTab)
{
	for( uint16_t i = 0; i < TOTAL_LINES; i++)
	{
		for( uint16_t j = 0; j < TOTAL_COL; j++)
		{
			inTab[i][j] = 0;
		}
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
uint8_t
_get_median_line( t_bin_mat in)
{
register int i,j;
uint8_t sum[TOTAL_LINES] = {0};
uint8_t extx[TOTAL_LINES] = {0}, exty[TOTAL_LINES] = {0}, extd[TOTAL_LINES] = {0};
uint8_t extidx = 0;
int8_t dir = 1, pdir = 1;
uint8_t psum = 0;
uint8_t max = 0, maxidx = 0;
uint8_t min = 255, minidx = 0;

	/* make sum tab */
	for( i = 0; i < TOTAL_LINES; i++)
		for( j = 0; j < TOTAL_COL / 2; j++)
			sum[i] += in[i][j];

	/* locate extremums */
	for( i = 0; i < TOTAL_LINES; i++)
	{
		/* current direction */
		if( sum[i] > psum)
			dir = 1;
		else if( sum[i] < psum)
			dir = -1;
		else dir = 0;

		if( dir != pdir)
		{
			extx[extidx] = i;
			exty[extidx] = psum;
			extd[extidx] = (dir<pdir) ? 1 : 0;
			extidx ++;
		}

		psum = sum[i];
		pdir = dir;
	}

	/* find max from top of image */
	for( i = 0; i < extidx; i++)
	{
		if( (exty[i] > max) && (extd[i] == 1) )
		{
			max = exty[i];
			maxidx = i;
		}
	}

	/* find lowest from previous max */
	for( i = maxidx; i < extidx - 1; i++)
	{
		if( (exty[i] <= min) && (extd[i] == 0) )
		{
			min = exty[i];
			minidx = i;
		}
	}

	return extx[minidx];
}

/*******************************************************************************
 * Function     :
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
void
_get_coord_extr_left( t_bin_mat in, uint8_t *xa, uint8_t *ya, uint8_t *xb, uint8_t *yb)
{
register int i,j;
uint8_t tab[TOTAL_LINES] = {0};
uint8_t median = _get_median_line( in);

	for( i = 0; i < TOTAL_LINES; i++)
		for( j = 0; j < TOTAL_COL / 2; j++)
		{
			tab[i] = j;
			if( in[i][j])
				break;
		}

	/* top half -> xa, ya */
	*xa = TOTAL_COL / 2;
	*ya = median;

	for( i = 0; i < median; i++)
	{
		if( tab[i] < *xa)
		{
			*xa = tab[i];
			*ya = i;
		}
	}

	/* botom half -> xb, yb */
	*xb = TOTAL_COL / 2;
	*yb = TOTAL_LINES;

	for( i = median; i < TOTAL_LINES; i++)
	{
		if( tab[i] < *xb)
		{
			*xb = tab[i];
			*yb = i;
		}
	}

	return;
}

/*******************************************************************************
 * Function     :
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
void
_get_coord_extr_axial_left( matrix_bin_t in, uint8_t *xa, uint8_t *ya, uint8_t *xb, uint8_t *yb)
{
register int i,j;
uint8_t sum_tab[TOTAL_LINES] = {0};

	for( i = 0; i < TOTAL_LINES; i++)
	{
		sum_tab[i] = 0;
		for( j = TOTAL_COL - 1; j >= 0  ; j--)
		{
			sum_tab[i] = (TOTAL_COL - 1 ) - j;
			if( in[i][j])
				break;
		}
	}

	/* bottom half -> xa, ya */
	*xa = TOTAL_COL - 1;
	*ya = 0;

	for( i = 0; i < TOTAL_LINES; i++)
	{
		if( sum_tab[i] < *xa)
		{
			*xa = sum_tab[i];
			*ya = i;
			break;         //Find first bottom pixel found is OK
		}
	}
    //Check upper line
	*xa = sum_tab[*ya+1];
	*ya = *ya+1;

	*xa = (TOTAL_COL - 1) - *xa

    LOG("SIZE: Left heel point : %d, %d\n", *xa, *ya);

	/* top half -> xb, yb */
	*xb = TOTAL_COL - 1;
	*yb = TOTAL_LINES - 1;

	for( i = TOTAL_LINES - 1; i >= 0; i--)
	{
		if( (sum_tab[i] < *xb) || (sum_tab[i] == 0))
		{
			*xb = sum_tab[i];
			*yb = i;
			break;
		}
	}
	//Check lower line
	*xb = sum_tab[*yb-1];
	*yb = *yb-1;
	*xb = (TOTAL_COL - 1) - *xb;

	LOG("SIZE: Left toe point : %d, %d\n", *xb, *yb);

	osDelay(100);

	return;
}

/*******************************************************************************
 * Function     :
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
void
_get_coord_extr_axial_right( matrix_bin_t in, uint8_t *xa, uint8_t *ya, uint8_t *xb, uint8_t *yb)
{
register int i,j;
uint8_t sum_tab[TOTAL_LINES] = {0};

	for( i = 0; i < TOTAL_LINES; i++)
	{
		sum_tab[i] = 0;
		for( j = 0; j < TOTAL_COL ; j++)
		{
			sum_tab[i] = j;
			if( in[i][j])
				break;
		}
	}

	/* bottom half -> xa, ya */
	*xa = TOTAL_COL - 1;
	*ya = 0;

	for( i = 0; i < TOTAL_LINES; i++)
	{
		if( sum_tab[i] < *xa)
		{
			*xa = sum_tab[i];
			*ya = i;
			break;         //Find first bottom pixel found is OK
		}
	}
    //Check upper line
	*xa = sum_tab[*ya+1];
	*ya = *ya+1;

    LOG("SIZE: Right heel point : %d, %d\n", *xa, *ya);

	/* top half -> xb, yb */
	*xb = TOTAL_COL - 1;
	*yb = TOTAL_LINES - 1;

	for( i = TOTAL_LINES - 1; i >= 0; i--)
	{
		if( (sum_tab[i] < *xb) || (sum_tab[i] == 0))
		{
			*xb = sum_tab[i];
			*yb = i;
			break;
		}
	}
	//Check lower line
	*xb = sum_tab[*yb-1];
	*yb = *yb-1;

	LOG("SIZE: Right toe point : %d, %d\n", *xb, *yb);

	osDelay(100);

	return;
}


/*******************************************************************************
 * Function     :
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
void
_get_coord_extr_right( t_bin_mat in, uint8_t *xc, uint8_t *yc, uint8_t *xd, uint8_t *yd)
{
register int i,j;
uint8_t tab[TOTAL_LINES] = {0};
uint8_t median = _get_median_line( in);

	for( i = 0; i < TOTAL_LINES; i++)
		for( j = (TOTAL_COL / 2) - 1; j >= 0; j--)
		{
			tab[i] = ((TOTAL_COL / 2) - 1) - j;
			if( in[i][j])
				break;
		}

	/* top half -> xa, ya */
	*xc = (TOTAL_COL / 2) - 1;
	*yc = median;

	for( i = 0; i < median; i++)
	{
		if( tab[i] < *xc)
		{
			*xc = tab[i];
			*yc = i;
		}
	}

	*xc = ((TOTAL_COL / 2) - 1) - *xc;

	/* botom half -> xb, yb */
	*xd = (TOTAL_COL / 2) - 1;
	*yd = TOTAL_LINES;

	for( i = median; i < TOTAL_LINES; i++)
	{
		if( tab[i] < *xd)
		{
			*xd = tab[i];
			*yd = i;
		}
	}

	*xd = ((TOTAL_COL / 2) - 1) - *xd;

	return;
}

/*******************************************************************************
 * Function     :
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
void
_get_hilo_pos( t_bin_mat in, uint8_t *hi, uint8_t *low)
{
register int i,j;
bool found = FALSE;

	/* find hi position */
	for( i = 0; i < TOTAL_LINES; i ++)
	{
		if (found == FALSE)
		{
			for (j = 0; j < TOTAL_COL / 2; j++)
			{
				if (in[i][j])
				{
					found = TRUE;
					*hi = i;
					break;
				}
			}
		}
		else
			break;
	}

	/* find low position */
	found = FALSE;
	for( i = TOTAL_LINES - 1; i >= 0; i --)
	{
		if( found == FALSE)
		{
			for( j = 0; j < TOTAL_COL/2; j++)
			{
				if( in[i][j])
				{
					found = TRUE;
					*low = i;
					break;
				}
			}
		}
		else
			break;
	}

	return;
}

/*******************************************************************************
 * Function     :
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
#define 	OFFSET		130

void
_get_hilo_tab_pos( t_acq_tab tab, uint8_t *hi, uint8_t *low)
{
register int i,j;
bool found = FALSE;
t_acq_mat *in = (t_acq_mat *)tab;

	/* find hi position */
	for( i = 0; i < TOTAL_LINES; i ++)
	{
		if (found == FALSE)
		{
			for (j = 0; j < TOTAL_COL / 2; j++)
			{
				if ((*in)[i][j] > OFFSET)
				{
					found = TRUE;
					*hi = i;
					break;
				}
			}
		}
		else
			break;
	}

	/* find low position */
	found = FALSE;
	for( i = TOTAL_LINES - 1; i >= 0; i --)
	{
		if( found == FALSE)
		{
			for( j = 0; j < TOTAL_COL/2; j++)
			{
				if( (*in)[i][j] > OFFSET)
				{
					found = TRUE;
					*low = i;
					break;
				}
			}
		}
		else
			break;
	}

	return;
}

/*******************************************************************************
 * Function     :
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
#define 	NOISEMARGIN		100

void
_get_hilo_pos_matrix( matrix_t tab, uint8_t *hi, uint8_t *low)
{
register int i,j;
bool found = FALSE;
matrix_t *in = (matrix_t *)tab;

	/* find low position */
	for( i = 0; i < TOTAL_LINES; i ++)
	{
		if (found == FALSE)
		{
			for (j = 0; j < TOTAL_COL; j++)
			{
				if ((*in)[i][j] > NOISEMARGIN)
				{
					found = TRUE;
					*low = i;
					break;
				}
			}
		}
		else
			break;
	}

	/* find hi position */
	found = FALSE;
	for( i = TOTAL_LINES - 1; i >= 0; i --)
	{
		if( found == FALSE)
		{
			for( j = 0; j < TOTAL_COL; j++)
			{
				if( (*in)[i][j] > NOISEMARGIN)
				{
					found = TRUE;
					*hi = i;
					break;
				}
			}
		}
		else
			break;
	}

	LOG("SIZE: heel line : %d\n", *low);
	LOG("SIZE: toe line : %d\n", *hi);

	osDelay(100);

	return;
}

/*******************************************************************************
 * Function     :
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
static t_return
_calc_size( t_sz_data *p)
{
uint8_t xa = 0;
uint8_t ya = 0;
uint8_t xb = 0;
uint8_t yb = 0;
double  a1 = 0;
double offset = 5;
t_return ret;

	/* Left sensor data */
#if (SIMULATION==1)
	_binarize( left_sensor_tab,  left_bin);
#else
	//_binarize( p->data.left,  left_bin);
	_binarize_matrix_from_noise_margin(p->matrix.left, matrix_left_bin);
#endif

	_get_coord_extr_axial_left(matrix_left_bin, &xa, &ya, &xb, &yb);
	_get_hilo_pos_matrix(p->matrix.left, &p->left_hi, &p->left_lo);

	if( (yb - ya) != 0)
	        a1 = atan(((double) xb - (double) xa) / ((double) yb - (double) ya));
	else
		a1 = 0;

	if ( abs(a1*(180/M_PI)) > 25) a1 = 0; //detection problem

	p->left_angle =  (M_PI/2) - a1;
	p->left_size = ( (p->left_hi - p->left_lo) / sin( p->left_angle) ) + offset;

	LOG("SIZE: left_angle : %f\n", p->left_angle*(180/M_PI));
	LOG("SIZE: left_size : %f\n", p->left_size);

	osDelay(100);

	/* right sensor data */
#if (SIMULATION==1)
	_binarize( right_sensor_tab,  right_bin);
#else
	//_binarize( p->data.left,  left_bin);
	_binarize_matrix_from_noise_margin(p->matrix.right, matrix_right_bin);
#endif

	_get_coord_extr_axial_right(matrix_right_bin, &xa, &ya, &xb, &yb);
	_get_hilo_pos_matrix(p->matrix.right, &p->right_hi, &p->right_lo);

	if( (yb - ya) != 0)
	        a1 = atan(((double) xb - (double) xa) / ((double) yb - (double) ya));
	else

		a1 = 0;

	if ( abs(a1*(180/M_PI)) > 25) a1 = 0; //detection problem

	p->right_angle =  (M_PI/2) - a1;
	p->right_size = ( (p->right_hi - p->right_lo) / sin( p->right_angle) ) + offset;

	LOG("SIZE: right_angle : %f\n", p->right_angle*(180/M_PI));
	LOG("SIZE: right_size : %f\n", p->right_size);

	/* take mean value */
	p->size = (p->left_size + p->right_size)/ 2;

	LOG("SIZE: total_size : %f\n", p->size);

	osDelay(100);

	//Clear matrix
	/*_clear_data(p->data.left);
	_clear_data(p->data.right);
	_clear_matrix(p->matrix.left);
	_clear_matrix(p->matrix.right);
	_clear_matrix_bin(matrix_left_bin);
	_clear_matrix_bin(matrix_right_bin);*/

	return E_OK;
}

/*******************************************************************************
 * Function     : RND_Size_Get
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
t_return
RND_Size_Get( t_measure *p)
{
	static uint16_t cnt = 0;
	char filename[24];

	RND_Acq_Multiple_Start(5);
	RND_Print("PENCHEZ-VOUS\nEN AVANT"); 	osDelay(4*SECOND);
	RND_Acq_Multiple_End( &p->d1.data, 5, 10);

	/***************************************************/
	RND_Fill_Dead_Pix(&p->d1.data, &p->d1.matrix);
	RND_Reorder(&p->d1.matrix);
	RND_Fill_Neighboor(&p->d1.matrix);
	RND_send_UART_full_matrix( &p->d1.matrix );
	/***************************************************/

	/* calculate size on average matrix */
	_calc_size( &p->d1);

	sprintf( filename, "Sz1_data_%d.csv\n", cnt);
	RND_USB_Write_Size( &p->d1, filename);

	//RND_Print("NOUVELLE\nMESURE"); 		osDelay( 2*SECOND);
	RND_Print("DEPLACEZ\nVOS\nAPPUIS"); osDelay( 2*SECOND);

	RND_Print( "STABILISEZ\nVOTRE\nPOSITION\n");

	while( RND_Calc_IsStable(5) == FALSE)
		osDelay(100);

	RND_Acq_Multiple_Start(5);
	RND_Print("PENCHEZ-VOUS\nEN AVANT"); osDelay(4*SECOND);
	RND_Acq_Multiple_End( &p->d2.data, 5, 10);

	/***************************************************/
	RND_Fill_Dead_Pix(&p->d2.data, &p->d2.matrix);
	RND_Reorder(&p->d2.matrix);
	RND_Fill_Neighboor(&p->d2.matrix);
	RND_send_UART_full_matrix( &p->d2.matrix );
	/***************************************************/

	/* calculate size on average matrix */
	_calc_size( &p->d2);

	sprintf( filename, "Sz2_data_%d.csv\n", cnt);
	RND_USB_Write_Size( &p->d2, filename);

	p->pointure = (p->d1.size + p->d2.size) / 2.0;
	p->pointure = round(p->pointure*2)/2;
	LOG("SIZE: TOTAL_SIZE : %f\n", p->pointure);

	cnt++;
	return E_OK;
}

#if (OPTIMIZE==1)
__OPTIMIZE_END
#endif

/*** End Of File ***/
