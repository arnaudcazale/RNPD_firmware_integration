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
#define LOGGING

/*******************************************************************************
 * Included Files
 *******************************************************************************/
#include "RND_Main.h"
#include "app_touchgfx.h"
#include "dsihost.h"
#include "dma2d.h"
#include "ltdc.h"

//#include "fatfs.h"

/*******************************************************************************
 * Locals
 *******************************************************************************/
static t_return DWT_Delay_Init				(void);
static t_return	RND_MakeVersionString		(void);
static char VersionString[64];

/*******************************************************************************
 * Globals
 *******************************************************************************/
//_IN_DATA_RAM static uint32_t toto;

/*******************************************************************************
 * Externals
 *******************************************************************************/
extern osThreadId TouchGfxHandle;
extern DSI_HandleTypeDef hdsi;
extern DMA2D_HandleTypeDef hdma2d;
extern LTDC_HandleTypeDef hltdc;
extern void BSP_Init_OTM(void);

/*******************************************************************************
 * Function     : RND_Init
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
t_return
RND_Init(void)
{
	/* initialisations */
	if( RND_LogInit() != E_OK)
		return E_ERROR;

	if( RND_MakeVersionString() != E_OK)
	{
		LOG("Failed to make version string\n");
		return E_ERROR;
	}
	LOG("Runpad, version %s\n", RND_GetVersionString());

	LOG("Initialisation: \n");

	/* gestion tempos µs */
	if( DWT_Delay_Init() != E_OK)
		return E_ERROR;
	LOG("DWT:OK\n");

	/* gestion de l'i²c */
	if( RND_I2C_Init() != E_OK)
		return E_ERROR;
	LOG("I2C:OK\n");

	/* gestion expander i2c */
	if( RND_EXP_Init() != E_OK)
		LOG("Failed to init i2c expander\n");
//		return E_ERROR;
	LOG("EXP:OK\n");

	/* gestion des LEDs */
	if( RND_LedInit() != E_OK)
		return E_ERROR;
	LOG("LED:OK\n");

	/* gestion sélection lignes */
	if( RND_MAT_Init() != E_OK)
		return E_ERROR;
	LOG("MAT:OK\n");

	/* gestion affichage text */
	if( RND_Print_Init() != E_OK)
		return E_ERROR;
	LOG("Affichage texte:OK\n");

	if( RND_Calc_Init() != E_OK)
		return E_ERROR;
	LOG("Module calculs:OK\n");

	LOG("Initialisation partie graphique:\n");

	return E_OK;
}

/*******************************************************************************
 * Function     : _goto_low_power
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
void
_goto_low_power( void)
{
	LOG("Display --> off \n");

//	/* switch off gfx task */
//	vTaskSuspend( TouchGfxHandle);

	/* turn LCD backlight off, and put it in sleep mode */
	RND_SetDisplay_Off();
	RND_SetDisplay_Backlight( FALSE);
	RND_SetDisplay_Sleep( TRUE);

	__HAL_RCC_DSI_CLK_DISABLE();
	__HAL_RCC_LTDC_CLK_DISABLE();

	/* disable SDRAM and QSPI */
	//	BSP_SDRAM_DeInit();
	//BSP_QSPI_DeInit();
}

/*******************************************************************************
 * Function     : _resume_from_low__power
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
void
_resume_from_low__power( void)
{
	LOG("Display --> On \n");

	__HAL_RCC_DSI_CLK_ENABLE();
	__HAL_RCC_LTDC_CLK_ENABLE();

	/* enable SDRAM and QSPI */
	//	BSP_SDRAM_Init();
	//BSP_QSPI_Init();
	//BSP_QSPI_EnableMemoryMappedMode();

	/* re-enable LCD display */
	RND_SetDisplay_Sleep( FALSE);
	//RND_SetDisplay_Backlight( TRUE);
	RND_SetDisplay_On();

//	/* resume GFX task */
//	vTaskResume( TouchGfxHandle);

//	osDelay(500);
//
//	char buffer[1024];
//	vTaskList( buffer);
//	LOG("\n%s\n", buffer);
//	osDelay(200);

}

/*******************************************************************************
 * Function     : RND_App
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
void
RND_Main_App(void)
{
	/* Wait (intro screen takes 3 seconds to bring first message up */
	osDelay(3000);

	RND_SEQ_Init();
	RND_SEQ_Sequencer();
}

/*******************************************************************************
 * Function     :
 * Arguments    :
 * Outputs      :
 * Return code  : Error DWT counter: 1 error, 0 OK
 * Description  : Initializes DWT_Clock_Cycle_Count for DWT_Delay_us function
 *******************************************************************************/
t_return
DWT_Delay_Init(void)
{
 	/* Disable TRC */
 	CoreDebug->DEMCR &= ~CoreDebug_DEMCR_TRCENA_Msk; // ~0x01000000;

 	/* Enable TRC */
 	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; // 0x01000000;

	//DWT->LAR = 0xC5ACCE55;

 	/* Disable clock cycle counter */
 	DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk; //~0x00000001;

	/* Reset the clock cycle counter value */
 	DWT->CYCCNT = 0;

	/* Enable clock cycle counter */
 	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk; //0x00000001;

	/* 3 NO OPERATION instructions */
	__asm volatile ("NOP");
	__asm volatile ("NOP");
 	__asm volatile ("NOP");

	/* Check if clock cycle counter has started */
	if( DWT->CYCCNT)
 		return E_OK; /*clock cycle counter started*/
	else
 		return E_ERROR; /*clock cycle counter not started*/
}

/*******************************************************************************
 * Function     : DWT_Delay_us
 * Arguments    : microseconds: delay in microseconds
 * Outputs      :
 * Return code  :
 * Description  : This function provides a delay (in microseconds)
 *******************************************************************************/
void
DWT_Delay_us( volatile uint32_t microseconds)
{
uint32_t clk_cycle_start = DWT->CYCCNT;

	/* Go to number of cycles for system */
 	microseconds *= (HAL_RCC_GetHCLKFreq() / 1000000);

	/* Delay till end */
 	while ((DWT->CYCCNT - clk_cycle_start) < microseconds);
}

/*******************************************************************************
 * Function     : DWT_Get
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
uint32_t
DWT_Get( void)
{
	return DWT->CYCCNT;
}

/*******************************************************************************
 * Function     : RND_MakeVersionString
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
static t_return
RND_MakeVersionString( void)
{
	char month[8];
	int  mon, date, year, hour, min, sec;

	memset( (void *)VersionString, 0, sizeof(VersionString));

	if( (sscanf( __DATE__, "%s %d %d", month, &date, &year) == 3) &&
		(sscanf( __TIME__, "%d:%d:%d", &hour, &min, &sec) == 3))
	{
		if( !strcmp( month, "Jan"))	mon = 1;
		else if( !strcmp( month, "Fev")) mon = 2;
		else if( !strcmp( month, "Mar")) mon = 3;
		else if( !strcmp( month, "Apr")) mon = 4;
		else if( !strcmp( month, "Mai")) mon = 5;
		else if( !strcmp( month, "Jun")) mon = 6;
		else if( !strcmp( month, "Jul")) mon = 7;
		else if( !strcmp( month, "Aug")) mon = 8;
		else if( !strcmp( month, "Sep")) mon = 9;
		else if( !strcmp( month, "Oct")) mon = 10;
		else if( !strcmp( month, "Nov")) mon = 11;
		else if( !strcmp( month, "Dev")) mon = 12;
		else mon = 0;

		sprintf( VersionString, "V%d.%d_%.2d%.2d%.4d%.2d%.2d%.2d", SW_MAJOR_VERSION, SW_MINOR_VERSION,
				mon, date, year, hour, min, sec);

		return E_OK;
	}

	return E_ERROR;
}

/*******************************************************************************
 * Function     : RND_GetVersionString
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
char *
RND_GetVersionString( void)
{
	return VersionString;
}

/*** End Of File ***/
