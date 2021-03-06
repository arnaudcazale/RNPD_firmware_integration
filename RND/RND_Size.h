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
#ifndef __RND_SIZE_H__
#define __RND_SIZE_H__

/*******************************************************************************
 * pre-include files
 *******************************************************************************/

/*******************************************************************************
 * Configuration
 *******************************************************************************/

/*******************************************************************************
 * Definitions
 *******************************************************************************/

/*******************************************************************************
 * types, enums ...
 *******************************************************************************/

/*******************************************************************************
 * Included Files
 *******************************************************************************/

/*******************************************************************************
 * Exported
 *******************************************************************************/
t_return		RND_Size_Get			(t_measure *p);

uint32_t		_calc_means				(t_acq_tab in);
t_return		_binarize				(t_acq_tab inTab, t_bin_mat outMat);
uint8_t			_get_median_line		(t_bin_mat in);
void			_get_coord_extr_left	(t_bin_mat in, uint8_t *xa, uint8_t *ya, uint8_t *xb, uint8_t *yb);
void			_get_coord_extr_right	(t_bin_mat in, uint8_t *xc, uint8_t *yc, uint8_t *xd, uint8_t *yd);
void			_get_hilo_pos			(t_bin_mat in, uint8_t *hi, uint8_t *low);
void			_get_hilo_tab_pos		(t_acq_tab tab, uint8_t *hi, uint8_t *low);
void			_get_hilo_pos_matrix    (matrix_t in, uint8_t *hi, uint8_t *low);

uint32_t		_calc_mean_matrix		                (matrix_t in);
t_return		_binarize_matrix	                    (matrix_t inTab, matrix_bin_t outMat);
t_return		_binarize_matrix_from_noise_margin	    (matrix_t inTab, matrix_bin_t outMat);
void			_get_coord_extr_axial_left	            (matrix_bin_t in, uint8_t *xa, uint8_t *ya, uint8_t *xb, uint8_t *yb);
void			_get_coord_extr_axial_right	            (matrix_bin_t in, uint8_t *xa, uint8_t *ya, uint8_t *xb, uint8_t *yb);
t_return 	    _clear_data                             (t_acq_tab inTab);
t_return 	    _clear_matrix                           (matrix_t inTab);
t_return 	    _clear_matrix_bin                       (matrix_bin_t inTab);



#endif
/*** End Of File ***/
