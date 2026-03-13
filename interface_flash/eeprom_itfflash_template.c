/**
  ******************************************************************************
  * @file    eeprom_emulation//interface_flash/eeprom_itfflash_template.c
  * @brief   This file provides all the EEPROM emulation flash interface functions.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "eeprom_itf_flash.h"
/** @addtogroup EEPROM_EMULATION
  * @{
  */

/** @addtogroup EEPROM_EMULATION_INTERFACE
  * @{
  */

/** @defgroup EEPROM_EMULATION_INTERFACE_FLASH_TEMPLATE EEPROM Interface FLASH Template
  * @{
  */

/* Exported functions --------------------------------------------------------*/
/**
  * @addtogroup EEPROM_EMULATION_INTERFACE_FLASH_FUNCTIONS
  * @{
  */

/** @defgroup EEPROM_EMULATION_INTERFACE_FLASH_TEMPLATE_APIS Interface FLASH Template Functions
  * @{
  */

/**
  * @brief  Initialize the flash interface (template).
  * @param  f_object Flash instance used by the interface.
  * @param  ee_callback Callback invoked on interrupted operations.
  * @retval Possible return values: see @ref ee_itf_flash_status.
  */
ee_itf_flash_status EE_ITF_FLASH_Init(void *f_object, ee_itf_flash_callback_t ee_callback)
{
  return EE_ITF_FLASH_ERROR;
}

/**
  * @brief  Write data to flash (template).
  * @param  address Flash address.
  * @param  p_data Pointer to data to write.
  * @param  size Data size in bytes.
  * @retval Possible return values: see @ref ee_itf_flash_status.
  */
ee_itf_flash_status EE_ITF_FLASH_WriteData(uint32_t address, uint8_t *p_data, uint16_t size)
{
  return EE_ITF_FLASH_ERROR;
  ;
}

/**
  * @brief  Read data from flash (template).
  * @param  address Flash address.
  * @param  p_data Output data buffer.
  * @param  size Data size in bytes.
  * @retval Possible return values: see @ref ee_itf_flash_status.
  */
ee_itf_flash_status EE_ITF_FLASH_ReadData(uint32_t address, uint8_t *p_data, uint16_t size)
{
  return EE_ITF_FLASH_ERROR;
}

/**
  * @brief  Erase flash pages (template).
  * @param  page Page-aligned address.
  * @param  nb_pages Number of pages to erase.
  * @retval Possible return values: see @ref ee_itf_flash_status.
  */
ee_itf_flash_status EE_ITF_FLASH_PageErase(uint32_t page, uint16_t nb_pages)
{
  return EE_ITF_FLASH_ERROR;
}

/**
  * @brief  Erase flash pages using interrupt process (template).
  * @param  page Page-aligned address.
  * @param  nb_pages Number of pages to erase.
  * @retval Possible return values: see @ref ee_itf_flash_status.
  */
ee_itf_flash_status EE_ITF_FLASH_PageErase_IT(uint32_t page, uint16_t nb_pages)
{
  return EE_ITF_FLASH_ERROR;
}

/**
  * @brief  Get last interrupted operation status (template).
  * @param  address Output pointer for the interrupted operation address.
  * @retval Possible return values: see @ref ee_itf_flash_status.
  */
ee_itf_flash_status EE_ITF_FLASH_GetLastOperationStatus(uint32_t *address)
{
  return EE_ITF_FLASH_ERROR;
}

/**
  * @brief  Clear flash interface errors (template).
  * @retval Possible return values: see @ref ee_itf_flash_status.
  */
ee_itf_flash_status EE_ITF_FLASH_ClearError(void)
{
  return EE_ITF_FLASH_ERROR;
}
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
