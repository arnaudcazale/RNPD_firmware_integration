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
#ifndef __RND_MAIN_H__
#define __RND_MAIN_H__

/*******************************************************************************
 * pre-include files
 *******************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

/*******************************************************************************
 * Configuration
 *******************************************************************************/

/*******************************************************************************
 * Definitions
 *******************************************************************************/
#define	HW_MAJOR_VERSION		1
#define HW_MINOR_VERSION		9
#define SW_MAJOR_VERSION		0
#define SW_MINOR_VERSION		993

#define	REAL_HARDWARE			1
#define OPTIMIZE				1
#define SIMULATION				0
#define REMOVE_OFFSET			1

/*******************************************************************************
 * types, enums ...
 *******************************************************************************/
#define		TOTAL_LINES						48
#define 	TOTAL_COL						16
#define		TOTAL_SENSORS					(TOTAL_LINES * TOTAL_COL)
#define		TOTAL_SENSORS_PER_SIDE			((TOTAL_LINES * TOTAL_COL)/2)

#define 	TOTAL_LINES_RED					(TOTAL_LINES / 4)
#define 	TOTAL_COL_RED					TOTAL_COL
#define 	TOTAL_SENSORS_RED				(TOTAL_LINES_RED * TOTAL_COL_RED)
#define 	TOTAL_SENSORS_RED_PER_SIDE		(TOTAL_SENSORS_RED / 2)

#define 	LINE_FROM_INDEX(a)				((uint8_t)(a/8) % TOTAL_LINES)
#define 	COL_FROM_INDEX(a)				((a<TOTAL_SENSORS_PER_SIDE) ? (a%(TOTAL_COL/2)) : (a%(TOTAL_COL/2))+8)
#define 	COL_GROUP_FROM_INDEX(a)			(uint8_t)(COL_FROM_INDEX(a)/4)
#define 	INDEX_FROM_LINE_COL(line,col) 	(col < TOTAL_COL/2) ? ((line*8)+col) : ((line*8)+col-8)

typedef enum
{
	E_ERROR,
	//E_SUCCESS,
	E_OK,
	E_TIMEOUT
}	t_return;

typedef enum
{
	NEUTRE_t,
	NEUTRE_TENDANCE_SUPINAL_t,
	NEUTRE_TENDANCE_CONTROL_t,
	CONTROL_t,
	SUPINAL_t,
}	t_pronation;

typedef	enum
{
	t_ACQ_FULL,
	t_ACQ_REDUCED
}	t_ACQ_Type;

typedef uint16_t (t_acq_tab)[TOTAL_SENSORS_PER_SIDE];
typedef uint16_t (t_acq_mat)[TOTAL_LINES][TOTAL_COL/2];
typedef uint8_t  (t_bin_mat)[TOTAL_LINES][TOTAL_COL/2];
typedef uint16_t (matrix_t) [TOTAL_LINES][TOTAL_COL];
typedef uint8_t  (matrix_bin_t)[TOTAL_LINES][TOTAL_COL];


typedef	struct
{
	uint8_t	line;
	uint8_t	col;
}	t_point;

typedef	struct
{
	t_point	heel;
	t_point	toe;
}	t_line;

typedef struct
{
	t_acq_tab 	left;
	t_acq_tab	right;
}	t_acq;

typedef struct
{
	matrix_t 	left;
	matrix_t	right;
}	matrix_full_t;

typedef	struct
{
	t_acq		data;
	matrix_full_t    matrix;

	t_point		left_hi;
	t_point		left_lo;
	t_point		right_hi;
	t_point		right_lo;
	uint8_t 	left_median;
	uint8_t		right_median;
	uint32_t 	left_upper_sum;
	uint32_t	left_lower_sum;
	uint32_t	right_upper_sum;
	uint32_t	right_lower_sum;
	uint32_t 	total_sum;

	double		gvt;
	uint8_t		igvt;
	t_point		barycentre_left;
	t_point		barycentre_right;
	t_line		neutral_left;
	t_line      neutral_right;
	double      dev_left;
	double      dev_right;
	double		dev_total;
	/*uint32_t 	left_extern_p;
	uint32_t	left_intern_p;
	uint32_t 	right_extern_p;
	uint32_t	right_intern_p;
	uint32_t 	extern_p;
	uint32_t	intern_p;*/

	t_pronation	pronation;
}	t_gvt_data;

typedef struct
{
	t_acq		     data;
	matrix_full_t    matrix;

	double 		left_angle;
	double 		left_size;
	uint8_t		left_hi;
	uint8_t		left_lo;

	double 		right_angle;
	double 		right_size;
	uint8_t		right_hi;
	uint8_t		right_lo;

	double		size;
}	t_sz_data;

typedef	struct
{
	t_sz_data	d1;
	t_sz_data	d2;
	double		pointure;
}	t_measure;

#ifndef __cplusplus
#if !defined(TRUE) && !defined(FALSE)
typedef	enum
{
	TRUE = 1,
	FALSE = 0,
}	bool;
#else
typedef	_Bool 	bool;
#endif
#else
#define TRUE	true
#define FALSE	false
#endif

#define	SECOND				(1000)
#define Byte_0(a)			((a>>24)&0xff)
#define Byte_1(a)			((a>>16)&0xff)
#define Byte_2(a)			((a>>8)&0xff)
#define Byte_3(a)			(a&0xff)
#define Word_0(a)			((a>>16)&0xffff)
#define Word_1(a)			(a&0xffff)
#define LOW_WORD(a)			(Word_0(a))
#define HIGH_WORD(a)		(Word_1(a))

#define _priority_fix(a) 	(a + abs( tskIDLE_PRIORITY-osPriorityIdle))
#define _IN_DATA_RAM		__attribute__((section(".dtaram")))

#define __OPTIMIZE_START	_Pragma("GCC push_options") \
							_Pragma("GCC optimize (\"O3\")")
#define __OPTIMIZE_END		_Pragma("GCC pop_options")

#define _MAX(x,y) ( \
    { __auto_type __x = (x); __auto_type __y = (y); \
      __x > __y ? __x : __y; })

/*******************************************************************************
 * Included Files
 *******************************************************************************/
#include "main.h"
#include "stm32469i_discovery.h"
#include "stm32469i_discovery_sdram.h"
//#include "stm32469i_discovery_qspi.h"
#include "cmsis_os.h"
#include "time.h"
#include "RND_Log.h"
#include "RND_Led.h"
#include "RND_I2c.h"
#include "RND_Disp.h"
#include "RND_Print.h"
#include "RND_Sequencer.h"
#include "RND_Calc.h"
#include "RND_Exp.h"
#include "RND_MAT.h"
#include "RND_Stabl.h"
#include "RND_Usb.h"
#include "RND_Size.h"
#include "RND_Gvt.h"
#include "BOOTLD_Flash.h"

/*******************************************************************************
 * Exported
 *******************************************************************************/
t_return	RND_Init				(void);
void		RND_Main_App			(void);
void		_goto_low_power			(void);
void		_resume_from_low__power	(void);
void 		DWT_Delay_us			(volatile uint32_t microseconds);
uint32_t	DWT_Get					(void);
char *		RND_GetVersionString	(void);

#endif
/*** End Of File ***/
