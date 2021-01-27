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
#include "usb_host.h"
//#include "usbh_diskio.h"
#include "fatfs.h"

/*******************************************************************************
 * Locals
 *******************************************************************************/
char buff[80];

/*******************************************************************************
 * Globals
 *******************************************************************************/
bool					FS_Connected = FALSE;

/*******************************************************************************
 * Externals
 *******************************************************************************/
extern ApplicationTypeDef 	Appli_state;
extern uint8_t retUSBH;    /* Return value for USBH */
extern char USBHPath[4];   /* USBH logical drive path */
extern FATFS USBHFatFS;    /* File system object for USBH logical drive */
extern FIL USBHFile;       /* File object for USBH */
extern uint16_t right_sensor[ TOTAL_SENSORS_PER_SIDE];
extern uint16_t left_sensor [ TOTAL_SENSORS_PER_SIDE];

/*******************************************************************************
 * Function     : RND_Usb_IsPresent
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
bool
RND_Usb_IsPresent( void)
{
	return (FS_Connected == TRUE);
}

/*******************************************************************************
 * Function     : RND_Usb_Process
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
t_return
RND_Usb_Process( void)
{
	//osDelay(SECOND);

	while(TRUE)
	{
		osDelay( SECOND / 4);

		MX_USB_HOST_Process();

		switch( Appli_state)
		{
		case APPLICATION_START:
			break;

		case APPLICATION_DISCONNECT:
			/* device disconected from USB */
			if (f_mount(NULL, "", 0) != FR_OK)
			{
				LOG("Error: cannot deinitialize fatfs\n");
			}

			if( FATFS_UnLinkDriver( USBHPath) != 0)
			{
				LOG("Error: cannot unlink USB fatfs driver\n");
			}

			LOG("Umounted USB Driver\n");
			FS_Connected = FALSE;
			Appli_state = APPLICATION_IDLE;
			break;

		case APPLICATION_READY:
			/* device connect to usb */
			if( FATFS_LinkDriver( &USBH_Driver, USBHPath) != 0)
			{
				LOG("Failed to attach driver\n");
				FS_Connected = FALSE;
				Appli_state = APPLICATION_IDLE;
				break;
			}

			if( f_mount( &USBHFatFS, USBHPath, 1) != FR_OK)
			{
				LOG("Failed to mount file system\n");
				FS_Connected = FALSE;
				Appli_state = APPLICATION_IDLE;
				break;
			}

			LOG("Attached file system\n");
			FS_Connected = TRUE;
			Appli_state = APPLICATION_IDLE;
			break;

		case APPLICATION_IDLE:
			break;
		}
	}
}

/*******************************************************************************
 * Function     : RND_Usb_Write_Matrix
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
t_return
RND_Usb_Write_Matrix( char *name, uint16_t *ptr)
{
t_return ret = E_ERROR;
FRESULT	fr;
FIL		fp;

	if( FS_Connected != TRUE)
	{
		//LOG("No file system attached\n");
		goto end;
	}

	LOG("Writting data to \"%s\" file\n", name);

	/* open file */
	fr = f_open( &fp, name, FA_CREATE_ALWAYS | FA_WRITE);
	if( fr != FR_OK)
	{
		LOG("Failed to open/create \"%s\"\n", name);
		goto end;
	}

//	f_printf( &fp, "\n#include \"RND_Main.h\"\n");
//	f_printf( &fp, "\n\nuint16_t left_sensor_tab[] = \n{\n");

	for( uint16_t i = 0; i < TOTAL_SENSORS_PER_SIDE; i++)
	{
//		f_printf( &fp, "%d", ptr[i]);
//		if( i && !((i+1)%8) )
//			f_printf( &fp, "\n");
//		else
//			f_printf( &fp, ",");

		f_printf( &fp, "\"%d\"", ptr[i]);
		if( i && !((i+1)%8) )
			f_printf( &fp, "\n");
		else
			f_printf( &fp, ";");
	}

//	f_printf( &fp, "};\n");

	f_sync( &fp);
	f_close( &fp);
	ret = E_OK;

end:
	return ret;
}

t_return
RND_USB_Write_Gvt( t_gvt_data *p, char *filename)
{
t_return ret = E_ERROR;
FRESULT	fr;
FIL		fp;
//uint16_t	index;

	if( RND_Usb_IsPresent() != TRUE)
	{
		LOG("No USB filesystem\n");
		return E_ERROR;
	}

	if( FS_Connected != TRUE)
	{
		LOG("Filesystem not connected\n");
		return E_ERROR;
	}

	fr = f_open( &fp, filename, FA_CREATE_ALWAYS | FA_WRITE);
	if( fr != FR_OK)
	{
		LOG("Failed to open/create file\n");
		goto end;
	}

	f_printf( &fp, "\n");
	f_printf( &fp, "Runpad, version %s\n", RND_GetVersionString());
	f_printf( &fp, "\n");
	f_printf( &fp, "Donnees de pronation et de gravity drop\n");
	f_printf( &fp, "GRAVITY: \n");
	f_printf( &fp, "left_hi line=%d, col=%d\n", p->left_hi.line, p->left_hi.col);
	f_printf( &fp, "left_lo line=%d, col=%d\n", p->left_lo.line, p->left_lo.col);
	f_printf( &fp, "right_hi line=%d, col=%d\n", p->right_hi.line, p->right_hi.col);
	f_printf( &fp, "right_lo line=%d, col=%d\n", p->right_lo.line, p->right_lo.col);
	f_printf( &fp, "left_median = %d\n", p->left_median);
	f_printf( &fp, "left_upper_sum = %d\n", p->left_upper_sum);
	f_printf( &fp, "left_lower_sum = %d\n", p->left_lower_sum);
	f_printf( &fp, "right_median = %d\n", p->right_median);
	f_printf( &fp, "right_upper_sum = %d\n", p->right_upper_sum);
	f_printf( &fp, "right_lower_sum = %d\n", p->right_lower_sum);
	f_printf( &fp, "total_sum = %d\n", p->total_sum);
	sprintf( buff, "%5.2f", p->gvt); f_printf( &fp, "gvt = %s\n", buff);
	f_printf( &fp, "igvt = %d\n", p->igvt);
	f_printf( &fp, "PRONATION: \n");
	f_printf( &fp, "barycentre_left: line=%d, col=%d\n", p->barycentre_left.line, p->barycentre_left.col);
	f_printf( &fp, "neutral_line_left_toe: line=%d, col=%d\n", p->neutral_left.toe.line, p->neutral_left.toe.col);
	f_printf( &fp, "neutral_line_left_heel: line=%d, col=%d\n", p->neutral_left.heel.line, p->neutral_left.heel.col);
	sprintf( buff, "%5.2f", p->dev_left); f_printf( &fp, "dev_left = %s\n", buff);
	f_printf( &fp, "barycentre_right: line=%d, col=%d\n", p->barycentre_right.line, p->barycentre_right.col);
	f_printf( &fp, "neutral_line_right_toe: line=%d, col=%d\n", p->neutral_right.toe.line, p->neutral_right.toe.col);
	f_printf( &fp, "neutral_line_right_heel: line=%d, col=%d\n", p->neutral_right.heel.line, p->neutral_right.heel.col);
	sprintf( buff, "%5.2f", p->dev_right); f_printf( &fp, "dev_right = %s\n", buff);
	sprintf( buff, "%5.2f", p->dev_total); f_printf( &fp, "dev_total = %s\n", buff);

	if( p->pronation == NEUTRE_t)
	{
		f_printf( &fp, "pronation = NEUTRE\n");
	}
	else if( p->pronation == NEUTRE_TENDANCE_CONTROL_t)
	{
		f_printf( &fp, "pronation = NEUTRE/CONT\n");
	}
	else if( p->pronation == NEUTRE_TENDANCE_SUPINAL_t)
	{
		f_printf( &fp, "pronation = NEUTRE/SUP\n");
	}
	else if( p->pronation == CONTROL_t)
	{
		f_printf( &fp, "pronation = CONTROL\n");
	}
	else if( p->pronation == SUPINAL_t)
	{
		f_printf( &fp, "pronation = SUPINAL\n");
	}

	f_sync( &fp);
	f_printf( &fp, "\n\n");

	for( int lines = TOTAL_LINES-1; lines >= 0; lines --)
	{
		f_printf( &fp, ";;;");

		for( uint8_t cols = 0; cols < TOTAL_COL*2; cols ++)
		{
			if( cols < TOTAL_COL)
			{
				f_printf( &fp, "\"%d\"", p->matrix.left[lines][cols]);
				if(cols == (TOTAL_COL-1) )
					f_printf( &fp, ";;");
				else
					f_printf( &fp, ";");
			}
			else
			{
				f_printf( &fp, "\"%d\"", p->matrix.right[lines][cols-TOTAL_COL]);
				if( cols ==  (TOTAL_COL*2)-1 )
					f_printf( &fp, "\n");
				else
					f_printf( &fp, ";");
			}
		}
	}

	f_sync( &fp);
	f_close( &fp);
	ret = E_OK;

end:
	return ret;
}

t_return
RND_USB_Write_Size( t_sz_data *p, char *filename)
{
t_return ret = E_ERROR;
FRESULT	fr;
FIL		fp;
uint16_t	index;

	if( RND_Usb_IsPresent() != TRUE)
	{
		LOG("No USB filesystem\n");
		return E_ERROR;
	}

	if( FS_Connected != TRUE)
	{
		LOG("Filesystem not connected\n");
		return E_ERROR;
	}

	fr = f_open( &fp, filename, FA_CREATE_ALWAYS | FA_WRITE);
	if( fr != FR_OK)
	{
		LOG("Failed to open/create file\n");
		goto end;
	}

	f_printf( &fp, "\n");
	f_printf( &fp, "Runpad, version %s\n", RND_GetVersionString());
	f_printf( &fp, "\n");
	f_printf( &fp, "Donnees de pointure\n");
	sprintf( buff, "%5.2f", p->left_angle); f_printf( &fp, "left_angle = %s\n", buff);
	sprintf( buff, "%5.2f", p->left_size); f_printf( &fp, "left_size = %s\n", buff);
	f_printf( &fp, "left_hi = %d\n", p->left_hi);
	f_printf( &fp, "left_lo = %d\n", p->left_lo);
	sprintf( buff, "%5.2f", p->right_angle); f_printf( &fp, "right_angle = %s\n", buff);
	sprintf( buff, "%5.2f", p->right_size); f_printf( &fp, "right_size = %s\n", buff);
	f_printf( &fp, "right_hi = %d\n", p->right_hi);
	f_printf( &fp, "right_lo = %d\n", p->right_lo);
	sprintf( buff, "%5.2f", p->size); f_printf( &fp, "size = %s\n", buff);

	f_sync( &fp);
	f_printf( &fp, "\n\n");

	for( int lines = TOTAL_LINES-1; lines >= 0; lines --)
		{
			f_printf( &fp, ";;;");

			for( uint8_t cols = 0; cols < TOTAL_COL*2; cols ++)
			{
				if( cols < TOTAL_COL)
				{
					f_printf( &fp, "\"%d\"", p->matrix.left[lines][cols]);
					if(cols == (TOTAL_COL-1) )
						f_printf( &fp, ";;");
					else
						f_printf( &fp, ";");
				}
				else
				{
					f_printf( &fp, "\"%d\"", p->matrix.right[lines][cols-TOTAL_COL]);
					if( cols ==  (TOTAL_COL*2)-1 )
						f_printf( &fp, "\n");
					else
						f_printf( &fp, ";");
				}
			}
		}

	/*for( uint8_t lines = 0; lines < TOTAL_LINES; lines ++)
	{
		f_printf( &fp, ";;;");

		for( uint8_t cols = 0; cols < TOTAL_COL; cols ++)
		{
			index = INDEX_FROM_LINE_COL( lines, cols);

			if( cols < TOTAL_COL / 2)
			{
				f_printf( &fp, "\"%d\"", p->data.left[index]);
				if( index & !((index+1)%8))
					f_printf( &fp, ";;");
				else
					f_printf( &fp, ";");
			}
			else
			{
				f_printf( &fp, "\"%d\"", p->data.right[index]);
				if( index & !((index+1)%8))
					f_printf( &fp, "\n");
				else
					f_printf( &fp, ";");
			}
		}
	}*/

	f_sync( &fp);
	f_close( &fp);
	ret = E_OK;

end:
	return ret;
}

///*******************************************************************************
// * Function     : RND_Usb_Write_Matrix_2
// * Arguments    :
// * Outputs      :
// * Return code  :
// * Description  :
// *******************************************************************************/
//t_return
//RND_Usb_Write_Matrix_2( t_acq data, t_measure *p, char *name)
//{
//t_return ret = E_ERROR;
//FRESULT	fr;
//FIL		fp;
//uint16_t	index;
//
//	if( FS_Connected != TRUE)
//		goto end;
//
//	LOG("Writting data to \"%s\" file\n", name);
//
//	/* open file */
//	fr = f_open( &fp, name, FA_CREATE_ALWAYS | FA_WRITE);
//	if( fr != FR_OK)
//	{
//		LOG("Failed to open/create \"%s\"\n", name);
//		goto end;
//	}
//
//	f_printf( &fp, "\n");
//	f_printf( &fp, "Runpad, version %s\n", RND_GetVersionString());
//	f_printf( &fp, "\n");
//	sprintf( buff, "%5.2f", p->left_angle); f_printf( &fp, "left_angle = %s\n", buff);
//	f_printf( &fp, "left_hi = %d\n", p->left_hi);
//	f_printf( &fp, "left_lo = %d\n", p->left_lo);
//	sprintf( buff, "%5.2f", p->left_size); f_printf( &fp, "left_size = %s\n", buff);
//	f_printf( &fp, "left_extern_pressure = %d\n", p->left_extern_pressure);
//	f_printf( &fp, "left_intern_pressure = %d\n", p->left_intern_pressure);
//	f_printf( &fp, "\n");
//	sprintf( buff, "%5.2f", p->right_angle); f_printf( &fp, "right_angle = %s\n", buff);
//	f_printf( &fp, "right_hi = %d\n", p->right_hi);
//	f_printf( &fp, "right_lo = %d\n", p->right_lo);
//	sprintf( buff, "%5.2f", p->right_size); f_printf( &fp, "right_size = %s\n", buff);
//	f_printf( &fp, "right_extern_pressure = %d\n", p->right_extern_pressure);
//	f_printf( &fp, "right_intern_pressure = %d\n", p->right_intern_pressure);
//	f_printf( &fp, "\n");
//	sprintf( buff, "%5.2f", p->pointure); f_printf( &fp, "pointure = %s\n", buff);
//	sprintf( buff, "%5.2f", p->gvt); f_printf( &fp, "gvt = %s\n", buff);
//	f_printf( &fp, "igvt = %d\n", p->igvt);
//	if( p->pronation == NEUTRE_t)
//	{
//		f_printf( &fp, "pronation = NEUTRE\n");
//	}
//	else if( p->pronation == CONTROL_t)
//	{
//		f_printf( &fp, "pronation = CONTROL\n");
//	}
//	else if( p->pronation == SUPINAL_t)
//	{
//		f_printf( &fp, "pronation = SUPINAL\n");
//	}
//	f_sync( &fp);
//	f_printf( &fp, "\n\n");
//
//	for( uint8_t lines = 0; lines < TOTAL_LINES; lines ++)
//	{
//		f_printf( &fp, ";;;");
//
//		for( uint8_t cols = 0; cols < TOTAL_COL; cols ++)
//		{
//			index = INDEX_FROM_LINE_COL( lines, cols);
//
//			if( cols < TOTAL_COL / 2)
//			{
//				f_printf( &fp, "\"%d\"", data.left[index]);
//				if( index & !((index+1)%8))
//					f_printf( &fp, ";;");
//				else
//					f_printf( &fp, ";");
//			}
//			else
//			{
//				f_printf( &fp, "\"%d\"", data.right[index]);
//				if( index & !((index+1)%8))
//					f_printf( &fp, "\n");
//				else
//					f_printf( &fp, ";");
//			}
//		}
//	}
//
//	f_sync( &fp);
//
//	f_close( &fp);
//	ret = E_OK;
//
//end:
//	return ret;
//}

/*** End Of File ***/
