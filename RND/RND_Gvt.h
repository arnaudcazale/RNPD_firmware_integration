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
#ifndef __RND_GVT_H__
#define __RND_GVT_H__

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
t_return		RND_Gvt_Get				( t_gvt_data *p);
t_return        RND_Gvt_Get_Zones       (matrix_t *mat, t_point *A, t_point *B);

#endif
/*** End Of File ***/
