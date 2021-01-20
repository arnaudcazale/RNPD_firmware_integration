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
//static t_acq_mat	left_filtered;
//static t_acq_mat	right_filtered;

static matrix_t	matrix_left_filtered;
static matrix_t	matrix_right_filtered;

#if (OPTIMIZE==1)
__OPTIMIZE_START
#endif

/*******************************************************************************
 * Globals
 *******************************************************************************/

/*******************************************************************************
 * Externals
 *******************************************************************************/
extern uint8_t left_bin [TOTAL_LINES][TOTAL_COL/2];
extern uint8_t right_bin[TOTAL_LINES][TOTAL_COL/2];

extern uint8_t matrix_left_bin[TOTAL_LINES][TOTAL_COL];
extern uint8_t matrix_right_bin[TOTAL_LINES][TOTAL_COL];


#if (SIMULATION==1)
extern uint16_t right_sensor_tab[];
extern uint16_t left_sensor_tab[];
#endif

/*******************************************************************************
 * Function     :
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
typedef struct
{
	uint8_t 	index;
	uint8_t		start_line;
	uint8_t 	end_line;
	uint8_t		n_lines;
}	t_line_zone;

typedef struct
{
	uint8_t		index;
	uint8_t		start_col;
	uint8_t		end_col;
	uint8_t		n_cols;
}	t_col_zone;

int compare_n_lines( const void *a, const void *b)
{
	uint8_t	n_line_a = ((t_line_zone *)a)->n_lines;
	uint8_t	n_line_b = ((t_line_zone *)b)->n_lines;
	return (n_line_a < n_line_b) - (n_line_a > n_line_b);
}

int compare_index( const void *a, const void *b)
{
	uint8_t index_a = ((t_line_zone *)a)->index;
	uint8_t index_b = ((t_line_zone *)b)->index;
	return (index_a > index_b) - (index_a < index_b);
}

int	compare_n_cols( const void *a, const void *b)
{
	uint8_t n_col_a = ((t_col_zone *)a)->n_cols;
	uint8_t n_col_b = ((t_col_zone *)b)->n_cols;
	return (n_col_a < n_col_b) - (n_col_a > n_col_b);
}

/*******************************************************************************
 * Function     : 
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
void
_filter_mat( t_acq_tab inTab, t_bin_mat bin, t_acq_mat outMat)
{
t_acq_mat *p = (t_acq_mat *)inTab;
register int i,j;

	for( i = 0; i < TOTAL_LINES; i++)
		for( j = 0; j < TOTAL_COL/2; j++)
			outMat[i][j] = bin[i][j] ? (*p)[i][j] : 0;

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
_filter_matrix( matrix_t inTab, matrix_bin_t bin, matrix_t outMat)
{
	matrix_t *p = (matrix_t *)inTab;
	register int i,j;

	for( i = 0; i < TOTAL_LINES; i++)
		for( j = 0; j < TOTAL_COL; j++)
			outMat[i][j] = bin[i][j] ? (*p)[i][j] : 0;

	return;
}

/*******************************************************************************
 * Function     :
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
uint32_t
_sum_mat( t_acq_mat in, uint8_t start_line, uint8_t end_line)
{
uint32_t sum = 0;
register int i,j;

	for( i = start_line; i < end_line; i++)
		for( j = 0; j < TOTAL_COL / 2; j++)
			sum += in[i][j];

	return sum;
}

/*******************************************************************************
 * Function     :
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
uint32_t
_sum_matrix( matrix_t in, uint8_t start_line, uint8_t end_line)
{
uint32_t sum = 0;
register int i,j;

	for( i = start_line; i < end_line; i++)
		for( j = 0; j < TOTAL_COL; j++)
			sum += in[i][j];

	return sum;
}
/*******************************************************************************
 * Function     :
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
void
_cal_pron( t_acq_mat in, uint8_t start_line, uint8_t end_line, uint32_t *lpres, uint32_t *rpres)
{
	uint32_t lp, rp;
	register int i,j;
	uint8_t top_i = start_line, top_j = TOTAL_COL/2;
	uint8_t bottom_i = end_line, bottom_j = TOTAL_COL/2;
	bool found = FALSE;
	double a,b;

		lp = rp = 0;

		for( i = start_line; (i < end_line) && (found == FALSE); i++)
		{
			for( j = 0; j < TOTAL_COL/2; j++)
			{
				if( in[i][j])
				{
					top_i = i;
					top_j = j;
					found = TRUE;
					break;
				}
			}
		}

		found = FALSE;
		for( i = end_line - 1; (i >= start_line) && (found == FALSE); i--)
		{
			for( j = 0; j < TOTAL_COL/2; j++)
			{
				if( in[i][j])
				{
					bottom_i = i;
					bottom_j = j;
					found = TRUE;
					break;
				}
			}
		}

		a = ((double)bottom_j - (double)top_j) / ((double)bottom_i - (double)top_i);
		b = (double)top_j - (a * (double)top_i);

		for( i = start_line; i < end_line; i++)
		{
			for( j = 0; j < TOTAL_COL/2; j++)
			{
				if( in[i][j])
				{
					if( (double)j < ((double)i * a) + b)
						lp += in[i][j];
					else
					if( (double)j > ((double)i * a) + b)
						rp += in[i][j];
				}
			}
		}

		*lpres += lp;
		*rpres += rp;
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
_cal_pron2( t_acq_mat in, t_point *A, t_point *B, uint32_t *lpres, uint32_t *rpres)
{
double a, b;
uint8_t median = (A->line + B->line) / 2;

	//*lpres = 0; *rpres = 0;

	a = ((double)B->col - (double)A->col) / ((double)B->line - (double)A->line);
	b =  (double)A->col - (a * (double)A->line);

	//LOG("a = %7.4f, b = %7.4f\n", a, b);

	for( uint8_t i = 0; i < median; i ++)
	{
		double tcol = (double)i * a + b;

		for( uint8_t j = 0; j < TOTAL_COL / 2; j ++)
		{
			if( (double)j < tcol)
				*lpres += in[i][j];
			else
				*rpres += in[i][j];
		}
	}

	return;
}

//***********************************************************************
//
//
//***********************************************************************
t_return
RND_Gvt_Get3_left( t_acq_mat in, t_point *A, t_point *B)
{
bool found = FALSE;

	for( int8_t i = 0; (i < TOTAL_LINES) && (found == FALSE); i++)
		for( int8_t j = TOTAL_COL/2 - 1; j >= 0; j--)
			if( in[i][j])
			{
				A->line = i;
				A->col  = j;
				found   = TRUE;
				break;
			}

	found = FALSE;

	for( int8_t i = TOTAL_LINES -1; (i >= 0) && (found == FALSE); i--)
		for( int8_t j = TOTAL_COL/2 - 1; j >= 0; j--)
			if( in[i][j])
			{
				B->line = i;
				B->col  = j;
				found   = TRUE;
				break;
			}

	return TRUE;
}

//***********************************************************************
//
//
//***********************************************************************
t_return
RND_Gvt_Get_Axis_Left( matrix_t in, t_point *A, t_point *B)
{
bool found = FALSE;

	//Find first toe point
	for( int8_t i = TOTAL_LINES -1; (i >= 0) && (found == FALSE); i--)
		for( int8_t j = TOTAL_COL - 1; j >= 0; j--)
			if( in[i][j])
			{
				A->line = i;
				A->col  = j;
				found   = TRUE;
				break;
			}

	found = FALSE;

	//Find first heel point
	for( int8_t i = 0; (i < TOTAL_LINES) && (found == FALSE); i++)
		for( int8_t j = TOTAL_COL - 1; j >= 0; j--)
			if( in[i][j])
			{
				B->line = i;
				B->col  = j;
				found   = TRUE;
				break;
			}

	LOG("Left toe point: = %d, %d\n", A->line, A->col);
	LOG("Left heel point: = %d, %d\n", B->line, B->col);

	return TRUE;
}

//***********************************************************************
//
//
//***********************************************************************
t_return
RND_Gvt_Get3_right( t_acq_mat in, t_point *A, t_point *B)
{
bool found = FALSE;

	for( int8_t i = 0; (i < TOTAL_LINES) && (found == FALSE); i++)
		for( int8_t j = 0; j < TOTAL_COL/2; j++)
			if( in[i][j])
			{
				A->line = i;
				A->col  = j;
				found   = TRUE;
				break;
			}

	found = FALSE;

	for( int8_t i = TOTAL_LINES -1; (i >= 0) && (found == FALSE); i--)
		for( int8_t j = 0; j < TOTAL_COL/2; j++)
			if( in[i][j])
			{
				B->line = i;
				B->col  = j;
				found   = TRUE;
				break;
			}

	return TRUE;
}

//***********************************************************************
//
//
//***********************************************************************
t_return
RND_Gvt_Get_Axis_Right( matrix_t in, t_point *A, t_point *B)
{
bool found = FALSE;

	//Find first toe point
	for( int8_t i = TOTAL_LINES -1; (i >= 0) && (found == FALSE); i--)
		for( int8_t j = 0; j < TOTAL_COL; j++)
			if( in[i][j])
			{
				A->line = i;
				A->col  = j;
				found   = TRUE;
				break;
			}

	found = FALSE;

	//Find first heel point
	for( int8_t i = 0; (i < TOTAL_LINES) && (found == FALSE); i++)
		for( int8_t j = 0; j < TOTAL_COL; j++)
			if( in[i][j])
			{
				B->line = i;
				B->col  = j;
				found   = TRUE;
				break;
			}

	LOG("Right toe point: = %d, %d\n", A->line, A->col);
	LOG("Right heel point: = %d, %d\n", B->line, B->col);

	return TRUE;
}

t_return
RND_Gvt_Get2( t_acq_tab *p, t_point *A, t_point *B)
{
t_return ret = E_ERROR;

uint32_t	line_sum[TOTAL_LINES];
uint32_t	col_sum[TOTAL_COL/2];
uint32_t	moy;
uint8_t		index;
uint16_t	val;

t_line_zone	zx[10];
t_col_zone	zy[10];

	/* somme des lignes */
	memset( (void *)line_sum, 0, sizeof(line_sum));
	for( uint8_t i = 0; i < TOTAL_LINES; i++)
		for( uint8_t j = 0; j < TOTAL_COL/2; j++)
			line_sum[i] += (*p)[INDEX_FROM_LINE_COL(i,j)];

	/* moyenne des sommes des lignes */
	moy = 0;
	for( uint8_t i = 0; i < TOTAL_LINES; i++)
		moy += line_sum[i];
	moy /= TOTAL_LINES;

	/* recherche zones */

	memset( (void *)zx, 0, sizeof(zx));
	val = index = 0;
	for( uint8_t i = 0; i < TOTAL_LINES; i++)
	{
		if( (line_sum[i] > moy) && (val < moy))
		{
			zx[index].index = index;
			zx[index].start_line = i;
		}
		else
		if( ((line_sum[i] < moy) || (i==TOTAL_LINES-1)) && (val > moy))
		{
			zx[index].end_line = i;
			zx[index].n_lines = zx[index].end_line - zx[index].start_line;
//			LOG("line_zone[%d] -> from line %d to line %d\n",
//					index, zx[index].start_line, zx[index].end_line);
			index ++;
		}
		val = line_sum[i];
	}

	if( index > 2)
	{
		/* sort and take the two biggest zones */
		qsort( (void *)zx, index, sizeof(t_line_zone), compare_n_lines);
		qsort( (void *)zx, 2, sizeof(t_line_zone), compare_index);
		index = 2;
	}
	else if( index <= 1)
	{
		LOG("Pb positionnement (line)\n");
		return E_ERROR;
	}

//	LOG("Zone haute lignes [%d à %d]\n", zx[0].start_line, zx[0].end_line);
//	LOG("Zone basse lignes [%d à %d]\n", zx[1].start_line, zx[1].end_line);

	/* zone 1 */
	/* somme des colonnes */
	memset( (void *)col_sum, 0, sizeof(col_sum));
	for( uint8_t j = 0; j < TOTAL_COL/2; j++)
		for( uint8_t i = zx[0].start_line; i < zx[0].end_line; i ++)
			col_sum[j] += (*p)[INDEX_FROM_LINE_COL(i,j)];

	/* calcul moyenne */
	moy = 0;
	for( uint8_t i = 0; i < TOTAL_COL/2; i++)
		moy += col_sum[i];
	moy /= (TOTAL_COL/2);

	/* recherche zone */
	memset( (void *)zy, 0, sizeof(zy));
	val = index = 0;
	for( uint8_t i = 0; i < TOTAL_COL/2; i++)
	{
		if( (col_sum[i] > moy) && (val < moy))
		{
			zy[index].index = index;
			zy[index].start_col = i;
		}
		else
		if( ((col_sum[i] < moy)||(i == (TOTAL_COL/2)-1)) && (val > moy))
		{
			zy[index].end_col = i;
			zy[index].n_cols = zy[index].end_col - zy[index].start_col;
//			LOG("col_zone[%d] -> from col %d to col %d\n",
//					index, zy[index].start_col, zy[index].end_col);
			index ++;
		}
		val = col_sum[i];
	}

	if( index > 1)
	{
		/* sort and take biggest one */
		qsort( (void *)zy, index, sizeof(t_col_zone), compare_n_cols);
		qsort( (void *)zy, index, sizeof(t_col_zone), compare_index);
		index = 1;
	}
	else if( !index)
	{
		LOG("Pb positionement (col zone 1)\n");
		return E_ERROR;
	}

//	LOG("Zone haute cols [%d à %d]\n", zy[0].start_col, zy[0].end_col);

	A->line = (zx[0].start_line + zx[0].end_line) / 2;
	A->col  = (zy[0].start_col + zy[0].end_col) / 2;

	/* zone 2 */
	/* somme des colonnes */
	memset( (void *)col_sum, 0, sizeof(col_sum));
	for( uint8_t j = 0; j < TOTAL_COL/2; j++)
		for( uint8_t i = zx[1].start_line; i < zx[1].end_line; i ++)
			col_sum[j] += (*p)[INDEX_FROM_LINE_COL(i,j)];

	/* calcul moyenne */
	moy = 0;
	for( uint8_t i = 0; i < TOTAL_COL/2; i++)
		moy += col_sum[i];
	moy /= (TOTAL_COL/2);

	/* recherche zones */
	memset( (void *)zy, 0, sizeof(zy));
	val = index = 0;
	for( uint8_t i = 0; i < TOTAL_COL/2; i++)
	{
		if( (col_sum[i] > moy) && (val < moy))
		{
			zy[index].index = index;
			zy[index].start_col = i;
		}
		else
		if( ((col_sum[i] < moy)||(i == (TOTAL_COL/2)-1)) && (val > moy))
		{
			zy[index].end_col = i;
			zy[index].n_cols = zy[index].end_col - zy[index].start_col;
//			LOG("col_zone[%d] -> from col %d to col %d\n",
//					index, zy[index].start_col, zy[index].end_col);
			index ++;
		}
		val = col_sum[i];
	}

	if( index > 1)
	{
		/* sort and take biggest one */
		qsort( (void *)zy, index, sizeof(t_col_zone), compare_n_cols);
		qsort( (void *)zy, index, sizeof(t_col_zone), compare_index);
		index = 1;
	}
	else if (!index)
	{
		LOG("Pb positionement (col zone 2)\n");
		return E_ERROR;
	}

//	LOG("Zone basse cols [%d à %d]\n", zy[0].start_col, zy[0].end_col);

	B->line = (zx[1].start_line + zx[1].end_line) / 2;
	B->col  = (zy[0].start_col + zy[0].end_col) / 2;

//	LOG("->Point Haut (%d,%d)\n", A->line, A->col);
//	LOG("->Point Bas  (%d,%d)\n", B->line, B->col);
	osDelay(100);

	return ret;
}

t_return
RND_Gvt_Get_Zones( matrix_t *p, t_point *A, t_point *B)
{
t_return ret = E_ERROR;

uint32_t	line_sum[TOTAL_LINES];
uint32_t	col_sum[TOTAL_COL];
uint32_t	moy;
uint8_t		index;
uint16_t	val;

t_line_zone	zx[10];
t_col_zone	zy[10];

	/* somme des lignes */
	memset( (void *)line_sum, 0, sizeof(line_sum));
	for( uint8_t i = 0; i < TOTAL_LINES; i++)
		for( uint8_t j = 0; j < TOTAL_COL; j++)
			line_sum[i] += (*p)[i][j];

	/* moyenne des sommes des lignes */
	moy = 0;
	for( uint8_t i = 0; i < TOTAL_LINES; i++)
		moy += line_sum[i];
	moy /= TOTAL_LINES;

	/* recherche zones */

	memset( (void *)zx, 0, sizeof(zx));
	val = index = 0;
	for( uint8_t i = 0; i < TOTAL_LINES; i++)
	{
		if( (line_sum[i] > moy) && (val < moy))
		{
			zx[index].index = index;
			zx[index].start_line = i;
		}
		else
		if( ((line_sum[i] < moy) || (i==TOTAL_LINES-1)) && (val > moy))
		{
			zx[index].end_line = i;
			zx[index].n_lines = zx[index].end_line - zx[index].start_line;
//			LOG("line_zone[%d] -> from line %d to line %d\n",
//					index, zx[index].start_line, zx[index].end_line);
			index ++;
		}
		val = line_sum[i];
	}

	if( index > 2)
	{
		/* sort and take the two biggest zones */
		qsort( (void *)zx, index, sizeof(t_line_zone), compare_n_lines);
		qsort( (void *)zx, 2, sizeof(t_line_zone), compare_index);
		index = 2;
	}
	else if( index <= 1)
	{
		LOG("Pb positionnement (line)\n");
		return E_ERROR;
	}

//	LOG("Zone haute lignes [%d à %d]\n", zx[0].start_line, zx[0].end_line);
//	LOG("Zone basse lignes [%d à %d]\n", zx[1].start_line, zx[1].end_line);

	/* zone 1 */
	/* somme des colonnes */
	memset( (void *)col_sum, 0, sizeof(col_sum));
	for( uint8_t j = 0; j < TOTAL_COL; j++)
		for( uint8_t i = zx[0].start_line; i < zx[0].end_line; i ++)
			col_sum[j] += (*p)[i][j];

	/* calcul moyenne */
	moy = 0;
	for( uint8_t i = 0; i < TOTAL_COL; i++)
		moy += col_sum[i];
	moy /= (TOTAL_COL);

	/* recherche zone */
	memset( (void *)zy, 0, sizeof(zy));
	val = index = 0;
	for( uint8_t i = 0; i < TOTAL_COL; i++)
	{
		if( (col_sum[i] > moy) && (val < moy))
		{
			zy[index].index = index;
			zy[index].start_col = i;
		}
		else
		if( ((col_sum[i] < moy)||(i == (TOTAL_COL)-1)) && (val > moy))
		{
			zy[index].end_col = i;
			zy[index].n_cols = zy[index].end_col - zy[index].start_col;
//			LOG("col_zone[%d] -> from col %d to col %d\n",
//					index, zy[index].start_col, zy[index].end_col);
			index ++;
		}
		val = col_sum[i];
	}

	if( index > 1)
	{
		/* sort and take biggest one */
		qsort( (void *)zy, index, sizeof(t_col_zone), compare_n_cols);
		qsort( (void *)zy, index, sizeof(t_col_zone), compare_index);
		index = 1;
	}
	else if( !index)
	{
		LOG("Pb positionement (col zone 1)\n");
		return E_ERROR;
	}

//	LOG("Zone haute cols [%d à %d]\n", zy[0].start_col, zy[0].end_col);

	A->line = (zx[0].start_line + zx[0].end_line) / 2;
	A->col  = (zy[0].start_col + zy[0].end_col) / 2;

	/* zone 2 */
	/* somme des colonnes */
	memset( (void *)col_sum, 0, sizeof(col_sum));
	for( uint8_t j = 0; j < TOTAL_COL; j++)
		for( uint8_t i = zx[1].start_line; i < zx[1].end_line; i ++)
			col_sum[j] += (*p)[i][j];

	/* calcul moyenne */
	moy = 0;
	for( uint8_t i = 0; i < TOTAL_COL; i++)
		moy += col_sum[i];
	moy /= (TOTAL_COL);

	/* recherche zones */
	memset( (void *)zy, 0, sizeof(zy));
	val = index = 0;
	for( uint8_t i = 0; i < TOTAL_COL; i++)
	{
		if( (col_sum[i] > moy) && (val < moy))
		{
			zy[index].index = index;
			zy[index].start_col = i;
		}
		else
		if( ((col_sum[i] < moy)||(i == (TOTAL_COL)-1)) && (val > moy))
		{
			zy[index].end_col = i;
			zy[index].n_cols = zy[index].end_col - zy[index].start_col;
//			LOG("col_zone[%d] -> from col %d to col %d\n",
//					index, zy[index].start_col, zy[index].end_col);
			index ++;
		}
		val = col_sum[i];
	}

	if( index > 1)
	{
		/* sort and take biggest one */
		qsort( (void *)zy, index, sizeof(t_col_zone), compare_n_cols);
		qsort( (void *)zy, index, sizeof(t_col_zone), compare_index);
		index = 1;
	}
	else if (!index)
	{
		LOG("Pb positionement (col zone 2)\n");
		return E_ERROR;
	}

//	LOG("Zone basse cols [%d à %d]\n", zy[0].start_col, zy[0].end_col);

	B->line = (zx[1].start_line + zx[1].end_line) / 2;
	B->col  = (zy[0].start_col + zy[0].end_col) / 2;

	LOG("->A (%d,%d)\n", A->line, A->col);
	LOG("->B  (%d,%d)\n", B->line, B->col);

	osDelay(100);

	return ret;
}

t_return
_cal_pron3( t_acq_tab *p, uint32_t *lpres, uint32_t *rpres)
{
uint32_t	line_sum[TOTAL_LINES];
uint32_t	col_sum[TOTAL_COL/2];
uint32_t	moy;
uint8_t		index;
uint16_t	val;
t_line_zone	zx[10];
t_col_zone	zy[10];

	/* summ up lines */
	memset( (void *)line_sum, 0, sizeof(line_sum));
	for( uint8_t i = 0; i < TOTAL_LINES; i++)
		for( uint8_t j = 0; j < TOTAL_COL/2; j++)
			line_sum[i] += (*p)[INDEX_FROM_LINE_COL(i,j)];

	/* Get the average of lines */
	moy = 0;
	for( uint8_t i = 0; i < TOTAL_LINES; i++)
		moy += line_sum[i];
	moy /= TOTAL_LINES;

	/* recherche zones */

	memset( (void *)zx, 0, sizeof(zx));
	val = index = 0;
	for( uint8_t i = 0; i < TOTAL_LINES; i++)
	{
		if( (line_sum[i] > moy) && (val < moy))
		{
			zx[index].index = index;
			zx[index].start_line = i;
		}
		else
		if( ((line_sum[i] < moy) || (i==TOTAL_LINES-1)) && (val > moy))
		{
			zx[index].end_line = i;
			zx[index].n_lines = zx[index].end_line - zx[index].start_line;
	//			LOG("line_zone[%d] -> from line %d to line %d\n",
	//					index, zx[index].start_line, zx[index].end_line);
			index ++;
		}
		val = line_sum[i];
	}

	/* if more than 2 zones, sort so as to keep the two biggest and order from top */
	if( index > 2)
	{
		/* sort and take the two biggest zones */
		qsort( (void *)zx, index, sizeof(t_line_zone), compare_n_lines);
		qsort( (void *)zx, 2, sizeof(t_line_zone), compare_index);
		index = 2;
	}
	else if( index <= 1)
	{
		LOG("Pb positionnement (line)\n");
		return E_ERROR;
	}

	/* handle highest zone */
	/* sum up columns */
	memset( (void *)col_sum, 0, sizeof(col_sum));
	for( uint8_t j = 0; j < TOTAL_COL/2; j++)
		for( uint8_t i = zx[0].start_line; i < zx[0].end_line; i ++)
			col_sum[j] += (*p)[INDEX_FROM_LINE_COL(i,j)];

	/* get the average */
	moy = 0;
	for( uint8_t i = 0; i < TOTAL_COL/2; i++)
		moy += col_sum[i];
	moy /= (TOTAL_COL/2);

	/* recherche zone */
	memset( (void *)zy, 0, sizeof(zy));
	val = index = 0;
	for( uint8_t i = 0; i < TOTAL_COL/2; i++)
	{
		if( (col_sum[i] > moy) && (val < moy))
		{
			zy[index].index = index;
			zy[index].start_col = i;
		}
		else
		if( ((col_sum[i] < moy)||(i == (TOTAL_COL/2)-1)) && (val > moy))
		{
			zy[index].end_col = i;
			zy[index].n_cols = zy[index].end_col - zy[index].start_col;
//			LOG("col_zone[%d] -> from col %d to col %d\n",
//					index, zy[index].start_col, zy[index].end_col);
			index ++;
		}
		val = col_sum[i];
	}

	/* if more than one, sort and order so as to keep the biggest one */
	if( index > 1)
	{
		/* sort and take biggest one */
		qsort( (void *)zy, index, sizeof(t_col_zone), compare_n_cols);
		qsort( (void *)zy, index, sizeof(t_col_zone), compare_index);
		index = 1;
	}
	else if( !index)
	{
		LOG("Pb positionement (col zone 1)\n");
		return E_ERROR;
	}

	/* top zone is in ZX[0] and zy */
//	LOG("PRO zone is lines [%d,%d] and columns [%d,%d]\n",
//			zx[0].start_line, zx[0].end_line,
//			zy[0].start_col,  zy[0].end_col);

	//*lpres = *rpres = 0;
	for( uint8_t i = zx[0].start_line; i < zx[0].end_line; i++)
	for( uint8_t j = zy[0].start_col; j < zy[0].end_col ; j++)
	{
		if( zy[0].n_cols % 2 == 0)
		{
			if( j < zy[0].start_col + (zy[0].n_cols / 2))
				*lpres += (*p)[INDEX_FROM_LINE_COL(i,j)];
			else
				*rpres += (*p)[INDEX_FROM_LINE_COL(i,j)];
		}
		else
		{
			if( j <= zy[0].start_col + (zy[0].n_cols / 2))
				*lpres += (*p)[INDEX_FROM_LINE_COL(i,j)];
			if( j >= zy[0].start_col + (zy[0].n_cols / 2))
				*rpres += (*p)[INDEX_FROM_LINE_COL(i,j)];
		}
	}

	return E_OK;
}

//*********************************************************
//
//
//*********************************************************
t_return
_cal_pron4( t_acq_tab *p, bool isleft, int8_t *dev)
{
uint32_t	line_sum[TOTAL_LINES];
uint32_t	col_sum[TOTAL_COL/2];
uint32_t	moy;
uint8_t		index;
uint16_t	val;
t_line_zone	zx[10];
t_col_zone	zy[10];

	/* summ up lines */
	memset( (void *)line_sum, 0, sizeof(line_sum));
	for( uint8_t i = 0; i < TOTAL_LINES; i++)
		for( uint8_t j = 0; j < TOTAL_COL/2; j++)
			line_sum[i] += (*p)[INDEX_FROM_LINE_COL(i,j)];

	/* Get the average of lines */
	moy = 0;
	for( uint8_t i = 0; i < TOTAL_LINES; i++)
		moy += line_sum[i];
	moy /= TOTAL_LINES;

	/* recherche zones */

	memset( (void *)zx, 0, sizeof(zx));
	val = index = 0;
	for( uint8_t i = 0; i < TOTAL_LINES; i++)
	{
		if( (line_sum[i] > moy) && (val < moy))
		{
			zx[index].index = index;
			zx[index].start_line = i;
		}
		else
		if( ((line_sum[i] < moy) || (i==TOTAL_LINES-1)) && (val > moy))
		{
			zx[index].end_line = i;
			zx[index].n_lines = zx[index].end_line - zx[index].start_line;
	//			LOG("line_zone[%d] -> from line %d to line %d\n",
	//					index, zx[index].start_line, zx[index].end_line);
			index ++;
		}
		val = line_sum[i];
	}

	/* if more than 2 zones, sort so as to keep the two biggest and order from top */
	if( index > 2)
	{
		/* sort and take the two biggest zones */
		qsort( (void *)zx, index, sizeof(t_line_zone), compare_n_lines);
		qsort( (void *)zx, 2, sizeof(t_line_zone), compare_index);
		index = 2;
	}
	else if( index <= 1)
	{
		LOG("Pb positionnement (line)\n");
		return E_ERROR;
	}

	/* handle highest zone */
	/* sum up columns */
	memset( (void *)col_sum, 0, sizeof(col_sum));
	for( uint8_t j = 0; j < TOTAL_COL/2; j++)
		for( uint8_t i = zx[0].start_line; i < zx[0].end_line; i ++)
			col_sum[j] += (*p)[INDEX_FROM_LINE_COL(i,j)];

	/* get the average */
	moy = 0;
	for( uint8_t i = 0; i < TOTAL_COL/2; i++)
		moy += col_sum[i];
	moy /= (TOTAL_COL/2);

	/* recherche zone */
	memset( (void *)zy, 0, sizeof(zy));
	val = index = 0;
	for( uint8_t i = 0; i < TOTAL_COL/2; i++)
	{
		if( (col_sum[i] > moy) && (val < moy))
		{
			zy[index].index = index;
			zy[index].start_col = i;
		}
		else
		if( ((col_sum[i] < moy)||(i == (TOTAL_COL/2)-1)) && (val > moy))
		{
			zy[index].end_col = i;
			zy[index].n_cols = zy[index].end_col - zy[index].start_col;
//			LOG("col_zone[%d] -> from col %d to col %d\n",
//					index, zy[index].start_col, zy[index].end_col);
			index ++;
		}
		val = col_sum[i];
	}

	/* if more than one, sort and order so as to keep the biggest one */
	if( index > 1)
	{
		/* sort and take biggest one */
		qsort( (void *)zy, index, sizeof(t_col_zone), compare_n_cols);
		qsort( (void *)zy, index, sizeof(t_col_zone), compare_index);
		index = 1;
	}
	else if( !index)
	{
		LOG("Pb positionement (col zone 1)\n");
		return E_ERROR;
	}

	/* top zone is in ZX[0] and zy */
	LOG("PRO zone is lines [%d,%d] and columns [%d,%d]\n",
			zx[0].start_line, zx[0].end_line,
			zy[0].start_col,  zy[0].end_col);

	/* calcul des coordonnées du barycentre */
	uint32_t bi = 0, bj = 0, coef_sum = 0;

	for( uint8_t i = zx[0].start_line; i < zx[0].end_line; i++)
	for( uint8_t j = zy[0].start_col; j < zy[0].end_col; j++)
	{
		bi += (*p)[INDEX_FROM_LINE_COL(i,j)] * i;
		bj += (*p)[INDEX_FROM_LINE_COL(i,j)] * j;
		coef_sum += (*p)[INDEX_FROM_LINE_COL(i,j)];
	}

	bi /= coef_sum;
	bj /= coef_sum;

	LOG("Barycentre line=%d, col=%d\n", bi, bj);

	t_point A, B;
	double a, b;
	t_acq_mat *mat = (t_acq_mat *)p;
	if( isleft == TRUE)
	{
		RND_Gvt_Get3_left( *mat, &A,  &B);

		a = ((double)B.col - (double)A.col) / ((double)B.line - (double)A.line);
		b =  (double)A.col - (a * (double)A.line);

		double tcol = (double)bi * a + b;
		tcol -= 2.;

		*dev = bj - tcol;

		LOG("Right: dev = %d\n", *dev);

	}
	else
	{
		RND_Gvt_Get3_right( *mat, &A,  &B);

		a = ((double)B.col - (double)A.col) / ((double)B.line - (double)A.line);
		b =  (double)A.col - (a * (double)A.line);

		double tcol = (double)bi * a + b;
		tcol += 2.;

		*dev = tcol - bj;

		LOG("Right: dev = %d\n", *dev);

		/*
		 * dev > 0 control;
		 * dev < 0 supinal;
		 */
	}

	return E_OK;
}

//*********************************************************
//
//
//*********************************************************
t_return
_cal_pronation_matrix( matrix_t *p, bool isleft, int8_t *dev)
{
uint32_t	line_sum[TOTAL_LINES];
uint32_t	col_sum[TOTAL_COL];
uint32_t	moy;
uint8_t		index;
uint16_t	val;
t_line_zone	zx[10];
t_col_zone	zy[10];

	/* summ up lines */
	memset( (void *)line_sum, 0, sizeof(line_sum));
	for( uint8_t i = 0; i < TOTAL_LINES; i++)
		for( uint8_t j = 0; j < TOTAL_COL; j++)
			line_sum[i] += (*p)[i][j];

	/* Get the average of lines */
	moy = 0;
	for( uint8_t i = 0; i < TOTAL_LINES; i++)
		moy += line_sum[i];
	moy /= TOTAL_LINES;

	/* recherche zones */

	memset( (void *)zx, 0, sizeof(zx));
	val = index = 0;
	for( uint8_t i = 0; i < TOTAL_LINES; i++)
	{
		if( (line_sum[i] > moy) && (val < moy))
		{
			zx[index].index = index;
			zx[index].start_line = i;
		}
		else
		if( ((line_sum[i] < moy) || (i==TOTAL_LINES-1)) && (val > moy))
		{
			zx[index].end_line = i;
			zx[index].n_lines = zx[index].end_line - zx[index].start_line;
	//			LOG("line_zone[%d] -> from line %d to line %d\n",
	//					index, zx[index].start_line, zx[index].end_line);
			index ++;
		}
		val = line_sum[i];
	}

	/* if more than 2 zones, sort so as to keep the two biggest and order from top */
	if( index > 2)
	{
		/* sort and take the two biggest zones */
		qsort( (void *)zx, index, sizeof(t_line_zone), compare_n_lines);
		qsort( (void *)zx, 2, sizeof(t_line_zone), compare_index);
		index = 2;
	}
	else if( index <= 1)
	{
		LOG("Pb positionnement (line)\n");
		return E_ERROR;
	}

	/* handle highest zone */
	/* sum up columns */
	memset( (void *)col_sum, 0, sizeof(col_sum));
	for( uint8_t j = 0; j < TOTAL_COL; j++)
		for( uint8_t i = zx[1].start_line; i < zx[1].end_line; i ++)
			col_sum[j] += (*p)[i][j];

	/* get the average */
	moy = 0;
	for( uint8_t i = 0; i < TOTAL_COL; i++)
		moy += col_sum[i];
	moy /= (TOTAL_COL);

	/* recherche zone */
	memset( (void *)zy, 0, sizeof(zy));
	val = index = 0;
	for( uint8_t i = 0; i < TOTAL_COL; i++)
	{
		if( (col_sum[i] > moy) && (val < moy))
		{
			zy[index].index = index;
			zy[index].start_col = i;
		}
		else
		if( ((col_sum[i] < moy)||(i == (TOTAL_COL)-1)) && (val > moy))
		{
			zy[index].end_col = i;
			zy[index].n_cols = zy[index].end_col - zy[index].start_col;
//			LOG("col_zone[%d] -> from col %d to col %d\n",
//					index, zy[index].start_col, zy[index].end_col);
			index ++;
		}
		val = col_sum[i];
	}

	/* if more than one, sort and order so as to keep the biggest one */
	if( index > 1)
	{
		/* sort and take biggest one */
		qsort( (void *)zy, index, sizeof(t_col_zone), compare_n_cols);
		qsort( (void *)zy, index, sizeof(t_col_zone), compare_index);
		index = 1;
	}
	else if( !index)
	{
		LOG("Pb positionement (col zone 1)\n");
		return E_ERROR;
	}

	/* top zone is in ZX[0] and zy */
	LOG("PRONATION zone is lines [%d,%d] and columns [%d,%d]\n",
			zx[1].start_line, zx[1].end_line,
			zy[0].start_col,  zy[0].end_col);

	/* calcul des coordonnées du barycentre */
	uint32_t bi = 0, bj = 0, coef_sum = 0;

	for( uint8_t i = zx[1].start_line; i < zx[1].end_line; i++)
	for( uint8_t j = zy[0].start_col; j < zy[0].end_col; j++)
	{
		bi += (*p)[i][j] * i;
		bj += (*p)[i][j] * j;
		coef_sum += (*p)[i][j];
	}

	bi /= coef_sum;
	bj /= coef_sum;

	LOG("Barycentre line=%d, col=%d\n", bi, bj);

	t_point A, B;
	double a, b;
	matrix_t *mat = (matrix_t *)p;
	if( isleft == TRUE)
	{
		RND_Gvt_Get_Axis_Left( *mat, &A,  &B);

		a = ((double)B.col - (double)A.col) / ((double)B.line - (double)A.line);
		b =  (double)A.col - (a * (double)A.line);

		double tcol = (double)bi * a + b;
		//tcol -= 2.;

		*dev = bj - tcol;

		LOG("Left: dev = %d\n", *dev);

	}
	else
	{
		RND_Gvt_Get_Axis_Right( *mat, &A,  &B);

		a = ((double)B.col - (double)A.col) / ((double)B.line - (double)A.line);
		b =  (double)A.col - (a * (double)A.line);

		double tcol = (double)bi * a + b;
		//tcol += 2.;

		*dev = tcol - bj;

		LOG("Right: dev = %d\n", *dev);

		/*
		 * dev > 0 control;
		 * dev < 0 supinal;
		 */
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
RND_Gvt_Get( t_gvt_data *p)
{
t_return ret = E_ERROR;
t_point left_A, left_B;
t_point right_A, right_B;

	LOG("GVT calculation\n");

	/* Acquire data */
//	LOG("GVT: Acquiring 4 frames\n");
	RND_Acq_Multiple( &p->data, 5);

	/**********************************************/
	RND_Fill_Dead_Pix(&p->data, &p->matrix);
	RND_Reorder(&p->matrix);
	RND_Fill_Neighboor(&p->matrix);
	RND_send_UART_full_matrix( &p->matrix );
	/**********************************************/

	/* make binary images */
#if (SIMULATION==1)
	_binarize( left_sensor_tab,  left_bin);
	_binarize( right_sensor_tab, right_bin);
#else
	//_binarize( p->data.left,  left_bin);
	//_binarize( p->data.right, right_bin);
	_binarize_matrix(p->matrix.left, matrix_left_bin);
	_binarize_matrix(p->matrix.right, matrix_right_bin);
#endif

	/* filter matrixes */
#if (SIMULATION==1)
	_filter_mat( left_sensor_tab, left_bin, left_filtered);
	_filter_mat( right_sensor_tab, right_bin, right_filtered);
#else
	//_filter_mat( p->data.left, left_bin, left_filtered);
	//_filter_mat( p->data.right, right_bin, right_filtered);
	_filter_matrix( p->matrix.left, matrix_left_bin, matrix_left_filtered);
	_filter_matrix( p->matrix.right, matrix_right_bin, matrix_right_filtered);

#endif

	//RND_Gvt_Get2( &(p->data.left) , &left_A,  &left_B);
	//RND_Gvt_Get2( &(p->data.right), &right_A, &right_B);
	RND_Gvt_Get_Zones( &matrix_left_filtered , &left_A,  &left_B);
	RND_Gvt_Get_Zones( &matrix_right_filtered , &right_A,  &right_B);

//	RND_Gvt_Get3_left( left_filtered, &left_A, &left_B);
//	RND_Gvt_Get3_right( right_filtered, &right_A, &right_B);

	p->left_lo = left_A;
	p->left_hi = left_B;
	p->right_lo = right_A;
	p->right_hi = right_B;

	/* get median lines for each */
//	p->left_median  = _get_median_line( left_bin);
//	p->right_median = _get_median_line( right_bin);

	p->left_median  = (left_A.line + left_B.line) / 2;
	p->right_median = (right_A.line + right_B.line) / 2;

	LOG("GVT: Left median line : %d\n", p->left_median);
	LOG("GVT: Right median line: %d\n", p->right_median);

	//p->left_upper_sum  = _sum_mat( left_filtered, 0, p->left_median);
	//p->left_lower_sum  = _sum_mat( left_filtered, p->left_median, TOTAL_LINES);
	//p->right_upper_sum = _sum_mat( right_filtered, 0, p->right_median);
	//p->right_lower_sum = _sum_mat( right_filtered, p->right_median, TOTAL_LINES);
	p->left_lower_sum  = _sum_matrix( matrix_left_filtered, 0, p->left_median);
	p->left_upper_sum  = _sum_matrix( matrix_left_filtered, p->left_median, TOTAL_LINES);
	p->right_lower_sum = _sum_matrix( matrix_right_filtered, 0, p->right_median);
	p->right_upper_sum = _sum_matrix( matrix_right_filtered, p->right_median, TOTAL_LINES);

	LOG("GVT: left lower sum : %d\n", p->left_lower_sum);
	LOG("GVT: left upper sum : %d\n", p->left_upper_sum);
	LOG("GVT: right lower sum: %d\n", p->right_lower_sum);
	LOG("GVT: right upper sum: %d\n", p->right_upper_sum);

	p->total_sum = p->right_lower_sum + p->right_upper_sum +
				p->left_lower_sum  + p->left_upper_sum;

	LOG("GVT: total: %d\n", p->total_sum);

	p->gvt = (double)(p->right_lower_sum + p->left_lower_sum) / (double)p->total_sum;
	LOG("gvt = %5.2f\n", p->gvt);

	/* new calculation method */
	double alpha = 2.0 / 3.0;
	if( p->gvt >= alpha)
		p->igvt = 0;
	else
		p->igvt = (uint8_t)(((alpha - p->gvt) / alpha) * 17);

	LOG("igvt = %d\n", p->igvt);

	osDelay(100);

	/* cal pronation */
//	p->left_extern_p = p->left_intern_p = p->right_extern_p = p->right_intern_p = 0;
//
//	_cal_pron( left_filtered, 0, TOTAL_LINES, &p->left_extern_p, &p->left_intern_p);
//	LOG("left_sensor (method 1): extern_p = %d, intern_p = %d\n", p->left_extern_p, p->left_intern_p);
//
//	_cal_pron( right_filtered, 0, TOTAL_LINES, &p->right_intern_p, &p->right_extern_p);
//	LOG("right_sensor (method 1): extern_p = %d, intern_p = %d\n", p->right_extern_p, p->right_intern_p);
//
//	_cal_pron2( left_filtered, &left_A, &left_B, &p->left_extern_p, &p->left_intern_p);
//	LOG("left_sensor (method 2): extern_p = %d, intern_p = %d\n", p->left_extern_p, p->left_intern_p);
//
//	_cal_pron2( right_filtered, &right_A, &right_B, &p->right_intern_p, &p->right_extern_p);
//	LOG("right_sensor (method 2): extern_p = %d, intern_p = %d\n", p->right_extern_p, p->right_intern_p);
//
//	_cal_pron3( &(p->data.left), &p->left_extern_p, &p->left_intern_p);
//	LOG("left_sensor  (method 3): extern_p = %d, intern_p = %d\n", p->left_extern_p, p->left_intern_p);
//
//	_cal_pron3( &(p->data.right), &p->right_intern_p, &p->right_extern_p);
//	LOG("right_sensor (method 3): extern_p = %d, intern_p = %d\n", p->right_extern_p, p->right_intern_p);
//
//	p->extern_p = p->left_extern_p + p->right_extern_p;
//	p->intern_p = p->left_intern_p + p->right_intern_p;
//
//	LOG("PRO: extern_pressure = %d\n", p->extern_p);
//	LOG("PRO: intern_pressure = %d\n", p->intern_p);

	//_cal_pron4( t_acq_tab *p, bool isleft, t_pronation *pronation)

	int8_t devg = 0, devd = 0, dev = 0;

	//_cal_pron4( &(p->data.left), TRUE, &devg);
	//_cal_pron4( &(p->data.right), FALSE, &devd);
	_cal_pronation_matrix(&matrix_left_filtered,  TRUE, &devg);
	_cal_pronation_matrix(&matrix_right_filtered,  FALSE, &devd);

	dev = (devg + devd) / 2;

	LOG("dev_total = %d\n", dev);

	/*if( dev == 0)
		p->pronation = NEUTRE_t;
	else if( dev > 0)
		p->pronation = CONTROL_t;
	else
		p->pronation = SUPINAL_t;*/

	if( dev < -2 )
			p->pronation = SUPINAL_t;
		else if( dev > 0)
			p->pronation = CONTROL_t;
		else
			p->pronation = NEUTRE_t;

	osDelay(100);

//	if( abs( p->extern_p - p->intern_p) < (((p->extern_p + p->intern_p) * 20)/100) )
//	{
//		p->pronation = NEUTRE_t;
//		LOG("Pronation: NEUTRE\n");
//	}
//	else if ( p->extern_p < p->intern_p)
//	{
//		p->pronation = CONTROL_t;
//		LOG("Pronation: CONTROL\n");
//	}
//	else if ( p->extern_p > p->intern_p)
//	{
//		p->pronation = SUPINAL_t;
//		LOG("Pronation: SUPINAL\n");
//	}

	return ret;
}

/* not used */
void RegLineaire(double x[], double y[], int n, double *a, double *b)
{
	int i;
	double xsomme, ysomme, xysomme, xxsomme;
	double ai, bi;
	xsomme = 0.0;
	ysomme = 0.0;
	xysomme = 0.0;
	xxsomme = 0.0;
	for (i = 0; i < n; i++)
	{
		xsomme = xsomme + x[i];
		ysomme = ysomme + y[i];
		xysomme = xysomme + x[i] * y[i];
		xxsomme = xxsomme + x[i] * x[i];
	}
	ai = (n * xysomme - xsomme * ysomme) / (n * xxsomme - xsomme * xsomme);
	bi = (ysomme - ai * xsomme) / n;
	*a = ai;
	*b = bi;
	return;
}


#if (OPTIMIZE==1)
__OPTIMIZE_END
#endif

/*** End Of File ***/
