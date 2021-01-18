/*******************************************************************************
 * CLC                                                                   *
 *******************************************************************************
 * Project:
 * Component:
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
#if	(TARGET==F429ZI) || (TARGET==F469I)
//static uint32_t 	GetSector			(uint32_t Address);
static uint32_t 	GetSectorSize		(uint32_t Sector);
#elif (TARGET==L476RG)
static uint32_t 	GetPage				(uint32_t Address);
static uint32_t 	GetBank				(uint32_t Address);
#endif

/*******************************************************************************
 * Globals
 *******************************************************************************/

/*******************************************************************************
 * Externals
 *******************************************************************************/

/*******************************************************************************
 * Function     : BOOTLD_Flash_Init
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
t_return
BOOTLD_Flash_Init( void)
{
#if	(TARGET==F429ZI) || (TARGET==F469I)
	HAL_FLASH_Unlock();
#elif (TARGET==L476RG)
	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG( FLASH_FLAG_OPTVERR);
#endif
	return E_OK;
}

/*******************************************************************************
 * Function     : BOOTLD_Flash_DeInit
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
t_return
BOOTLD_Flash_DeInit( void)
{
	HAL_FLASH_Lock();
	return E_OK;
}

/*******************************************************************************
 * Function     : BOOTLD_Flash_Erase_Sector
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  : erase one sector (holding the given address)
 *******************************************************************************/
t_return
BOOTLD_Flash_Erase_Sector( uint32_t address)
{
#if	(TARGET==F429ZI) || (TARGET==F469I)
	HAL_StatusTypeDef status;
	FLASH_EraseInitTypeDef eraseinitstruct;
	uint32_t	StartSector = 0, SectorError = 0;

	if( (address < FLASH_BASE) ||
		(address > FLASH_END))
		return E_ERROR;

	StartSector = GetSector( address);
	eraseinitstruct.TypeErase = FLASH_TYPEERASE_SECTORS;
	eraseinitstruct.Sector = StartSector;
	eraseinitstruct.NbSectors = 1;
	eraseinitstruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
	status = HAL_FLASHEx_Erase(&eraseinitstruct, &SectorError);

	if( status != HAL_OK)
		return E_ERROR;

	return E_OK;
#elif (TARGET==L476RG)
	HAL_StatusTypeDef status;
	FLASH_EraseInitTypeDef eraseinitstruct;
	uint32_t	Page, PageError = 0;

	if( (address < FLASH_BASE) ||
		(address > FLASH_END))
		return E_ERROR;

	eraseinitstruct.TypeErase = FLASH_TYPEERASE_PAGES;
	eraseinitstruct.Banks     = 1;
	eraseinitstruct.Page      = GetPage( address);
	eraseinitstruct.Banks     = GetBank( address);
	status = HAL_FLASHEx_Erase( &eraseinitstruct, &PageError);

	if( status != HAL_OK)
		return E_ERROR;
#endif
}

/*******************************************************************************
 * Function     : BOOTLD_Flash_Write
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
t_return
BOOTLD_Flash_Write( uint8_t *src, uint8_t *dest, uint32_t Len)
{
	uint32_t i = 0;

	if( ((uint32_t) dest < FLASH_BASE) ||
		((uint32_t) dest > FLASH_END))
		return ERROR;

	for (i = 0; i < Len; i += 4)
	{
		if( HAL_FLASH_Program( FLASH_TYPEPROGRAM_WORD, (uint32_t)(dest + i), *(uint32_t*) (src + i)) == HAL_OK)
		{
			/* Check the written value */
			if( *(uint32_t*) (src + i) != *(uint32_t*) (dest + i))
			{
				/* Flash content doesn't match SRAM content */
				return E_ERROR;
			}
		}
		else
		{
			/* Error occurred while writing data in Flash memory */
			return E_ERROR;
		}

//		if( *(uint32_t*) (src + i) != *(uint32_t*) (dest + i))
//		{
//			/* Flash content doesn't match SRAM content */
//			return E_ERROR;
//		}
	}

	return E_OK;
}

/*******************************************************************************
 * Function     : BOOTLD_Flash_Read
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
uint8_t*
BOOTLD_Flash_Read( uint8_t *src, uint8_t *dest, uint32_t Len)
{
	uint32_t i = 0;
	uint8_t *psrc = src;

	if( ((uint32_t) src < FLASH_BASE) ||
		((uint32_t) src > FLASH_END))
		return NULL;

	for (i = 0; i < Len; i++)
		dest[i] = *psrc++;

	return (uint8_t*) (dest);
}

#if (TARGET==F429ZI) || (TARGET==F469I)
/*******************************************************************************
 * Function     : GetSector
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
uint32_t GetSector(uint32_t Address)
{
	uint32_t sector = 0;

	if ((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
	{
		sector = FLASH_SECTOR_0;
	}
	else if ((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
	{
		sector = FLASH_SECTOR_1;
	}
	else if ((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
	{
		sector = FLASH_SECTOR_2;
	}
	else if ((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
	{
		sector = FLASH_SECTOR_3;
	}
	else if ((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
	{
		sector = FLASH_SECTOR_4;
	}
	else if ((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
	{
		sector = FLASH_SECTOR_5;
	}
	else if ((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
	{
		sector = FLASH_SECTOR_6;
	}
	else if ((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
	{
		sector = FLASH_SECTOR_7;
	}
	else if ((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
	{
		sector = FLASH_SECTOR_8;
	}
	else if ((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
	{
		sector = FLASH_SECTOR_9;
	}
	else if ((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
	{
		sector = FLASH_SECTOR_10;
	}
	else if ((Address < ADDR_FLASH_SECTOR_12) && (Address >= ADDR_FLASH_SECTOR_11))
	{
		sector = FLASH_SECTOR_11;
	}
	else if ((Address < ADDR_FLASH_SECTOR_13) && (Address >= ADDR_FLASH_SECTOR_12))
	{
		sector = FLASH_SECTOR_12;
	}
	else if ((Address < ADDR_FLASH_SECTOR_14) && (Address >= ADDR_FLASH_SECTOR_13))
	{
		sector = FLASH_SECTOR_13;
	}
	else if ((Address < ADDR_FLASH_SECTOR_15) && (Address >= ADDR_FLASH_SECTOR_14))
	{
		sector = FLASH_SECTOR_14;
	}
	else if ((Address < ADDR_FLASH_SECTOR_16) && (Address >= ADDR_FLASH_SECTOR_15))
	{
		sector = FLASH_SECTOR_15;
	}
	else if ((Address < ADDR_FLASH_SECTOR_17) && (Address >= ADDR_FLASH_SECTOR_16))
	{
		sector = FLASH_SECTOR_16;
	}
	else if ((Address < ADDR_FLASH_SECTOR_18) && (Address >= ADDR_FLASH_SECTOR_17))
	{
		sector = FLASH_SECTOR_17;
	}
	else if ((Address < ADDR_FLASH_SECTOR_19) && (Address >= ADDR_FLASH_SECTOR_18))
	{
		sector = FLASH_SECTOR_18;
	}
	else if ((Address < ADDR_FLASH_SECTOR_20) && (Address >= ADDR_FLASH_SECTOR_19))
	{
		sector = FLASH_SECTOR_19;
	}
	else if ((Address < ADDR_FLASH_SECTOR_21) && (Address >= ADDR_FLASH_SECTOR_20))
	{
		sector = FLASH_SECTOR_20;
	}
	else if ((Address < ADDR_FLASH_SECTOR_22) && (Address >= ADDR_FLASH_SECTOR_21))
	{
		sector = FLASH_SECTOR_21;
	}
	else if ((Address < ADDR_FLASH_SECTOR_23) && (Address >= ADDR_FLASH_SECTOR_22))
	{
		sector = FLASH_SECTOR_22;
	}
	else /* (Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_23) */
	{
		sector = FLASH_SECTOR_23;
	}
	return sector;
}

/*******************************************************************************
 * Function     : GetSectorSize
 * Arguments    :
 * Outputs      :
 * Return code  :
 * Description  :
 *******************************************************************************/
static uint32_t GetSectorSize(uint32_t Sector)
{
	uint32_t sectorsize = 0x00;
	if ((Sector == FLASH_SECTOR_0) || (Sector == FLASH_SECTOR_1)
			|| (Sector == FLASH_SECTOR_2) || (Sector == FLASH_SECTOR_3)
			|| (Sector == FLASH_SECTOR_12) || (Sector == FLASH_SECTOR_13)
			|| (Sector == FLASH_SECTOR_14) || (Sector == FLASH_SECTOR_15))
	{
		sectorsize = 16 * 1024;
	}
	else if ((Sector == FLASH_SECTOR_4) || (Sector == FLASH_SECTOR_16))
	{
		sectorsize = 64 * 1024;
	}
	else
	{
		sectorsize = 128 * 1024;
	}
	return sectorsize;
}

#elif (TARGET==L476RG)
/*******************************************************************************
 * Function     : GetPage
 * Arguments    :
 * Outputs      :
 * Return code  : The page number of a given address
 * Description  :
 *******************************************************************************/
static uint32_t GetPage(uint32_t Addr)
{
	uint32_t page = 0;

	if (Addr < (FLASH_BASE + FLASH_BANK_SIZE))
	{
		/* Bank 1 */
		page = (Addr - FLASH_BASE) / FLASH_PAGE_SIZE;
	}
	else
	{
		/* Bank 2 */
		page = (Addr - (FLASH_BASE + FLASH_BANK_SIZE)) / FLASH_PAGE_SIZE;
	}

	return page;
}

/*******************************************************************************
 * Function     : GetBank
 * Arguments    :
 * Outputs      :
 * Return code  : The bank number of a given address
 * Description  :
 *******************************************************************************/
static uint32_t GetBank(uint32_t Addr)
{
	uint32_t bank = 0;

	if (READ_BIT(SYSCFG->MEMRMP, SYSCFG_MEMRMP_FB_MODE) == 0)
	{
		/* No Bank swap */
		if (Addr < (FLASH_BASE + FLASH_BANK_SIZE))
		{
			bank = FLASH_BANK_1;
		}
		else
		{
			bank = FLASH_BANK_2;
		}
	}
	else
	{
		/* Bank swap */
		if (Addr < (FLASH_BASE + FLASH_BANK_SIZE))
		{
			bank = FLASH_BANK_2;
		}
		else
		{
			bank = FLASH_BANK_1;
		}
	}

	return bank;
}
#endif

/*** End Of File ***/
