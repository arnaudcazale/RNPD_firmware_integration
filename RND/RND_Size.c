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
static t_return
_calc_size( t_sz_data *p)
{
uint8_t xa, ya, xb, yb;
uint8_t xc, yc, xd, yd;
double  a1 = 0, a2 = 0;
double xy_ratio = 16/6.6;

	/* Left sensor data */

#if (SIMULATION==1)
	_binarize( left_sensor_tab,  left_bin);
#else
	_binarize( p->data.left,  left_bin);
#endif

	_get_coord_extr_left(  left_bin, &xa, &ya, &xb, &yb);
	_get_coord_extr_right( left_bin, &xc, &yc, &xd, &yd);

	xa *= xy_ratio;
	xb *= xy_ratio;
	xc *= xy_ratio;
	xd *= xy_ratio;

	if( (yb - ya) != 0)
		a1 = atan(((double) xb - (double) xa) / ((double) yb - (double) ya));
	else
		a1 = 0;

	/* angle in degres :
	 * a1 = a1 * (180/M_PI);
	 */

	if( (yd - yc) != 0)
		a2 = atan(((double) xd - (double) xc) / ((double) yd - (double) yc));
	else
		a2 = 0;

	/* angle in degres :
	 * a2 = a2 * (180/M_PI);
	 */

	p->left_angle = (a1 + a2) / 2.0;

	/* right sensor data */

#if (SIMULATION==1)
	_binarize( right_sensor_tab, right_bin);
#else
	_binarize( p->data.right, right_bin);
#endif

	_get_coord_extr_left(  right_bin, &xa, &ya, &xb, &yb);
	_get_coord_extr_right( right_bin, &xc, &yc, &xd, &yd);

	xa *= xy_ratio;
	xb *= xy_ratio;
	xc *= xy_ratio;
	xd *= xy_ratio;

	if( (yb - ya) != 0)
		a1 = atan(((double) xb - (double) xa) / ((double) yb - (double) ya));
	else
		a1 = 0;

	/* angle in degres :
	 * a1 = a1 * (180/M_PI);
	 */

	if( (yd - yc) != 0)
		a2 = atan(((double) xd - (double) xc) / ((double) yd - (double) yc));
	else
		a2 = 0;

	/* angle in degres :
	 * a2 = a2 * (180/M_PI);
	 */

	/* take median value of angle */
	p->right_angle = (a1 + a2) / 2.0;

	/* left size */
	//_get_hilo_pos( left_bin, &hi, &lo);

#if (SIMULATION==1)
	_get_hilo_tab_pos( left_sensor_tab, &hi, &lo);
#else
	_get_hilo_tab_pos( p->data.left, &p->left_hi, &p->left_lo);
#endif
	p->left_size = (p->left_lo - p->left_hi + 3) / cos( p->left_angle);
	LOG("left: hi=%d, lo=%d, gap=%d, angle=%4.2f, size=%4.2f\n",
			p->left_hi, p->left_lo, p->left_lo-p->left_hi, p->left_angle *(180/M_PI), p->left_size);

	p->left_angle *= (180/M_PI);

	/* right size */
	//_get_hilo_pos( right_bin, &hi, &lo);

#if (SIMULATION==1)
	_get_hilo_tab_pos( right_sensor_tab, &hi, &lo);
#else
	_get_hilo_tab_pos( p->data.right, &p->right_hi, &p->right_lo);
#endif

	p->right_size = (p->right_lo - p->right_hi + 3) / cos( p->right_angle);
	LOG("right: hi=%d, lo=%d, gap=%d, angle=%4.2f, size=%4.2f\n",
			p->right_hi, p->right_lo, p->right_lo-p->right_hi, p->right_angle *(180/M_PI), p->right_size);

	p->right_angle *= (180/M_PI);

	/* take max value */
	p->size = _MAX( p->left_size, p->right_size);
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
	//RND_Fill_Dead_Pix(&p->d1.data, &p->d1.matrix);
	//RND_Reorder(&p->d1.matrix);
	//RND_Fill_Neighboor(&p->d1.matrix);
	//RND_send_UART( &p->d1.matrix );
	/***************************************************/

	/* calculate size on average matrix */
	_calc_size( &p->d1);

	sprintf( filename, "Sz1_data_%d.csv\n", cnt);
	RND_USB_Write_Size( &p->d1, filename);

	//RND_Calc_AcqCsv( data, p, "data1.csv");

	//RND_Print("NOUVELLE\nMESURE"); 		osDelay( 2*SECOND);
	RND_Print("DEPLACEZ\nVOS\nAPPUIS"); osDelay( 2*SECOND);

	RND_Print( "STABILISEZ\nVOTRE\nPOSITION\n");

	while( RND_Calc_IsStable(5) == FALSE)
		osDelay(100);

	RND_Acq_Multiple_Start(5);
	RND_Print("PENCHEZ-VOUS\nEN AVANTS"); osDelay(4*SECOND);
	RND_Acq_Multiple_End( &p->d2.data, 5, 10);

	_calc_size( &p->d2);

	sprintf( filename, "Sz2_data_%d.csv\n", cnt);
	RND_USB_Write_Size( &p->d2, filename);

	p->pointure = (p->d1.size + p->d2.size) / 2.0;

	cnt++;
	return E_OK;
}

#if (OPTIMIZE==1)
__OPTIMIZE_END
#endif

/*** End Of File ***/
