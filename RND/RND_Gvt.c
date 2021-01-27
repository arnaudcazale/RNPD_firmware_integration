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
#include "RND_Sequencer.h"

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

//***********************************************************************
//
//
//***********************************************************************
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

//***********************************************************************
//
//
//***********************************************************************
t_return
RND_Gvt_Get_Neutral_Line_Left(matrix_t *mat, t_point *pA, t_point *pB)
{
	t_point A = {0,0};
	t_point B = {0,0};
	int median_line = 0;

	RND_Gvt_Get_Zones( mat , &A,  &B);
	median_line  = (A.line + B.line) / 2;
	LOG("median_line = %d\n",median_line);

	_binarize_matrix_from_noise_margin (*mat, matrix_left_bin);

	//Make sum of each lines
	int sum_col[TOTAL_LINES] = {0};
	int sum = 0;

	for(int i = 0; i<TOTAL_LINES; i++)
	{
		for(int j = 0 ; j < TOTAL_COL; j++){
			sum+=matrix_left_bin[i][j];
		}
		sum_col[i] = sum;
		sum = 0;
	}

	//Find center of largest line below median line, priority to lower line:
	//Find index of larger line
	int max = 0;
	int lineMaxLow = 0;
	for(int i = median_line; i>=0; i--)
	{
		if(sum_col[i] >= max)
		{
			lineMaxLow = i;
			max = sum_col[i];
		}
	}
	//Find left beggining of largest line
	int startCol = 0;
	for(int j = 0; j<TOTAL_COL; j++)
	{
		if(matrix_left_bin[lineMaxLow][j] == 1){
			startCol = j;
		 break;
		}
	}
	//Find right end of largest line
	int stopCol = 0;
	for(int j = TOTAL_COL-1; j>=0; j--)
	{
		if(matrix_left_bin[lineMaxLow][j] == 1){
			stopCol = j;
		 break;
		}
	}
	double centerColLow = ceil( ((double)stopCol-(double)startCol)/2 ) + startCol;

	pB->col = (int)centerColLow;
	pB->line = lineMaxLow;
	LOG("pB_col = %d, pB_line = %d\n", pB->col, pB->line);


	//Find center largest line above median line, priority to lower line:
	//Find index of larger line
	max = 0;
	int lineMaxHi = 0;
	for(int i = median_line; i<TOTAL_LINES; i++)
	{
		if(sum_col[i] >= max)
		{
			lineMaxHi = i;
			max = sum_col[i];
		}
	}
	//Find left beggining of largest line
	startCol = 0;
	for(int j = 0; j<TOTAL_COL; j++)
	{
		if(matrix_left_bin[lineMaxHi][j] == 1){
			startCol = j;
		 break;
		}
	}
	//Find right end of largest line
	stopCol = 0;
	for(int j = TOTAL_COL-1; j>=0; j--)
	{
		if(matrix_left_bin[lineMaxHi][j] == 1){
			stopCol = j;
		 break;
		}
	}

	double centerColHi = ceil( ((double)stopCol-(double)startCol)/2 ) + startCol;

	pA->col = (int)centerColHi;
	pA->line = lineMaxHi;
	LOG("pA_col = %d, pA_line = %d\n", pA->col, pA->line);

	return TRUE;
}

//***********************************************************************
//
//
//***********************************************************************
t_return
RND_Gvt_Get_Neutral_Line_Right(matrix_t *mat, t_point *pA, t_point *pB)
{
	t_point A = {0,0};
	t_point B = {0,0};
	int median_line = 0;

	RND_Gvt_Get_Zones( mat , &A,  &B);
	median_line  = (A.line + B.line) / 2;
	LOG("median_line = %d\n",median_line);

	_binarize_matrix_from_noise_margin (*mat, matrix_right_bin);

	//Make sum of each lines
	int sum_col[TOTAL_LINES] = {0};
	int sum = 0;

	for(int i = 0; i<TOTAL_LINES; i++)
	{
		for(int j = 0 ; j < TOTAL_COL; j++){
			sum+=matrix_right_bin[i][j];
		}
		sum_col[i] = sum;
		sum = 0;
	}

	//Find center of largest line below median line, priority to lower line:
	//Find index of larger line
	int max = 0;
	int lineMaxLow = 0;
	for(int i = median_line; i>=0; i--)
	{
		if(sum_col[i] >= max)
		{
			lineMaxLow = i;
			max = sum_col[i];
		}
	}
	//Find left beggining of largest line
	int startCol = 0;
	for(int j = 0; j<TOTAL_COL; j++)
	{
		if(matrix_right_bin[lineMaxLow][j] == 1){
			startCol = j;
		 break;
		}
	}
	//Find right end of largest line
	int stopCol = 0;
	for(int j = TOTAL_COL-1; j>=0; j--)
	{
		if(matrix_right_bin[lineMaxLow][j] == 1){
			stopCol = j;
		 break;
		}
	}
	double centerColLow = ceil( ((double)stopCol-(double)startCol)/2 ) + startCol;

	pB->col = (int)centerColLow;
	pB->line = lineMaxLow;
	LOG("pB_col = %d, pB_line = %d\n", pB->col, pB->line);


	//Find center largest line above median line, priority to lower line:
	//Find index of larger line
	max = 0;
	int lineMaxHi = 0;
	for(int i = median_line; i<TOTAL_LINES; i++)
	{
		if(sum_col[i] >= max)
		{
			lineMaxHi = i;
			max = sum_col[i];
		}
	}
	//Find left beggining of largest line
	startCol = 0;
	for(int j = 0; j<TOTAL_COL; j++)
	{
		if(matrix_right_bin[lineMaxHi][j] == 1){
			startCol = j;
		 break;
		}
	}
	//Find right end of largest line
	stopCol = 0;
	for(int j = TOTAL_COL-1; j>=0; j--)
	{
		if(matrix_right_bin[lineMaxHi][j] == 1){
			stopCol = j;
		 break;
		}
	}

	double centerColHi = ceil( ((double)stopCol-(double)startCol)/2 ) + startCol;

	pA->col = (int)centerColHi;
	pA->line = lineMaxHi;
	LOG("pA_col = %d, pA_line = %d\n", pA->col, pA->line);

	return TRUE;
}

//***********************************************************************
//
//
//***********************************************************************
t_return
RND_Gvt_Get_Zones( matrix_t *p, t_point *A, t_point *B)
{

uint32_t	line_sum[TOTAL_LINES];
uint32_t	col_sum[TOTAL_COL];
uint32_t	moy = 0;
uint8_t		index = 0;
uint16_t	val = 0;

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
		if( (line_sum[i] >= moy) && (val <= moy))
		{
			zx[index].index = index;
			zx[index].start_line = i;
		}
		else if( ((line_sum[i] <= moy) || (i==TOTAL_LINES-1)) && (val >= moy))
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
		if( (col_sum[i] >= moy) && (val <= moy))
		{
			zy[index].index = index;
			zy[index].start_col = i;
		}
		else if( ((col_sum[i] <= moy)||(i == (TOTAL_COL)-1)) && (val >= moy))
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
		//qsort( (void *)zy, index, sizeof(t_col_zone), compare_index);
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
		if( (col_sum[i] >= moy) && (val <= moy))
		{
			zy[index].index = index;
			zy[index].start_col = i;
		}
		else if( ((col_sum[i] <= moy)||(i == (TOTAL_COL)-1)) && (val >= moy))
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
		//qsort( (void *)zy, index, sizeof(t_col_zone), compare_index);
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

	return E_OK;
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
		else if( ((line_sum[i] < moy) || (i==TOTAL_LINES-1)) && (val > moy))
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
		//qsort( (void *)zx, index, sizeof(t_line_zone), compare_n_lines);
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
		else if( ((col_sum[i] < moy)||(i == (TOTAL_COL/2)-1)) && (val > moy))
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
		//qsort( (void *)zy, index, sizeof(t_col_zone), compare_index);
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
		//qsort( (void *)zy, index, sizeof(t_col_zone), compare_index);
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
_cal_pronation_matrix(t_gvt_data *gvt, matrix_t *p, bool isleft, double *dev)
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
		if( (line_sum[i] >= moy) && (val <= moy))
		{
			zx[index].index = index;
			zx[index].start_line = i;
		}
		else if( ((line_sum[i] <= moy) || (i==TOTAL_LINES-1)) && (val >= moy))
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
		if( (col_sum[i] >= moy) && (val <= moy))
		{
			zy[index].index = index;
			zy[index].start_col = i;
		}
		else if( ((col_sum[i] <= moy)||(i == (TOTAL_COL)-1)) && (val >= moy))
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
		//qsort( (void *)zy, index, sizeof(t_col_zone), compare_index);
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

	t_point pA = {0,0};
	t_point pB = {0,0};
	matrix_t *mat = (matrix_t *)p;
	double a = 0;
	double b = 0;
	double tcol = 0;

	if( isleft == TRUE)
	{
		RND_Gvt_Get_Neutral_Line_Left(mat, &pA, &pB);
		LOG("pB_col = %d, pB_line = %d\n", pB.col, pB.line);
		LOG("pA_col = %d, pA_line = %d\n", pA.col, pA.line);

		if(pA.col == pB.col)
		{
			LOG("vertical line");
			tcol = pA.col;
		}else
		{
			LOG("fonction affine");
			a = ( (double)pA.line - (double)pB.line ) / ( (double)pA.col - (double)pB.col )  ;
			b = (double)pA.line - (a * (double)pA.col);
			LOG("y = %5.2f x + %5.2f\n",a ,b);
			tcol = ((double)bi - b) / a;
		}

		*dev = bj - tcol;

		LOG("Left: dev = %5.2f\n", *dev);

		gvt->barycentre_left.line   = bi;
		gvt->barycentre_left.col    = bj;
		gvt->neutral_left.toe.line  = pA.line;
		gvt->neutral_left.toe.col   = pA.col;
		gvt->neutral_left.heel.line = pB.line;
		gvt->neutral_left.heel.col  = pB.col;
		gvt->dev_left  = *dev;

		osDelay(100);

	}
	else
	{
		RND_Gvt_Get_Neutral_Line_Right(mat, &pA, &pB);
		LOG("pB_col = %d, pB_line = %d\n", pB.col, pB.line);
		LOG("pA_col = %d, pA_line = %d\n", pA.col, pA.line);

		if(pA.col == pB.col)
		{
			LOG("vertical line\n");
			tcol = pA.col;
		}else
		{
			LOG("fonction affine\n");
			a = ( (double)pA.line - (double)pB.line ) / ( (double)pA.col - (double)pB.col )  ;
			b = (double)pA.line - (a * (double)pA.col);
			LOG("y = %5.2f x + %5.2f\n",a ,b);
			tcol = ((double)bi - b) / a;
		}

		*dev = tcol - bj;

		LOG("Right: dev = %5.2f\n", *dev);

		gvt->barycentre_right.line   = bi;
		gvt->barycentre_right.col    = bj;
		gvt->neutral_right.toe.line  = pA.line;
		gvt->neutral_right.toe.col   = pA.col;
		gvt->neutral_right.heel.line = pB.line;
		gvt->neutral_right.heel.col  = pB.col;
		gvt->dev_right  = *dev;

		osDelay(100);

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
t_point left_A = {0,0};
t_point left_B = {0,0};
t_point right_A = {0,0};
t_point right_B = {0,0};

	LOG("GVT calculation\n");

	/* Acquire data */
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
	_binarize_matrix(p->matrix.left, matrix_left_bin);
	_binarize_matrix(p->matrix.right, matrix_right_bin);
#endif

	/* filter matrixes */
#if (SIMULATION==1)
	_filter_mat( left_sensor_tab, left_bin, left_filtered);
	_filter_mat( right_sensor_tab, right_bin, right_filtered);
#else
	_filter_matrix( p->matrix.left, matrix_left_bin, matrix_left_filtered);
	_filter_matrix( p->matrix.right, matrix_right_bin, matrix_right_filtered);

#endif

	ret = RND_Gvt_Get_Zones( &matrix_left_filtered , &left_A,  &left_B);
	if(ret == E_ERROR){
		RND_Print("GRAVITY\nPOSITION\nERROR");
		osDelay(2*SECOND);
		return ret;
	}
	ret = RND_Gvt_Get_Zones( &matrix_right_filtered , &right_A,  &right_B);
	if(ret == E_ERROR){
		RND_Print("GRAVITY\nPOSITION\nERROR");
		osDelay(2*SECOND);
		return ret;
	}

	p->left_lo = left_A;
	p->left_hi = left_B;
	p->right_lo = right_A;
	p->right_hi = right_B;

	p->left_median  = (left_A.line + left_B.line) / 2;
	p->right_median = (right_A.line + right_B.line) / 2;

	LOG("GVT: Left median line : %d\n", p->left_median);
	LOG("GVT: Right median line: %d\n", p->right_median);

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

	double devg = 0, devd = 0, dev = 0;

	ret = _cal_pronation_matrix(p, &matrix_left_filtered,  TRUE, &devg);
	if(ret == E_ERROR){
		RND_Print("PRONATION\nDETECTION\nERROR");
		osDelay(2*SECOND);
		return ret;
	}

	ret = _cal_pronation_matrix(p, &matrix_right_filtered,  FALSE, &devd);
	if(ret == E_ERROR){
		RND_Print("PRONATION\nDETECTION\nERROR");
		osDelay(2*SECOND);
		return ret;
	}

	dev = (devg + devd) / 2;
	p->dev_total = dev;

	LOG("dev_total = %5.2f\n", dev);

	if( dev < -3)
		p->pronation = SUPINAL_t;
	    else if( dev > 3)
	    	p->pronation = CONTROL_t;
	    else if( (dev >=-3) && (dev <=-1.5) )
	    	p->pronation = NEUTRE_TENDANCE_SUPINAL_t;
	    else if( (dev >=1.5) && (dev <=3) )
	    	p->pronation = NEUTRE_TENDANCE_CONTROL_t;
	    else
	    	p->pronation = NEUTRE_t;


	osDelay(100);

	//Clear all matrix
	/*_clear_data(p->data.left);
	_clear_data(p->data.right);
	_clear_matrix(p->matrix.left);
	_clear_matrix(p->matrix.right);
	_clear_matrix_bin(matrix_left_bin);
	_clear_matrix_bin(matrix_right_bin);
	_clear_matrix(matrix_left_filtered);
	_clear_matrix(matrix_right_filtered);*/

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
