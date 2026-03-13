/**
  ******************************************************************************
  * @file    eeprom_emulation/interface_flash/eeprom_itfflash_nvm_edata.c
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

/** @addtogroup EEPROM_EMULATION_INTERFACE_FLASH
  * @{
  */

/** @defgroup EEPROM_Emulation_interface_FLASH_NVM_EDATA Interface FLASH NVM EDATA
  * @{
  */

/* Private variables ---------------------------------------------------------*/
/** @defgroup EEPROM_EMULATION_INTERFACE_FLASH_NVM_PRIVATE_VARIABLES Interface FLASH NVM Private Variables
  * @{
  */

/**
  * @brief local variable the store the HAL flash handle.
  */
hal_nvm_handle_t *NVMObject = NULL;

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/**
  * @addtogroup EEPROM_EMULATION_INTERFACE_FLASH_FUNCTIONS
  * @{
  */

/** @defgroup EEPROM_EMULATION_INTERFACE_FLASH_NVM_EDATA_API Interface FLASH NVM Edata Functions
  * @{
  */

/**
  * @brief  Initialize the EDATA NVM flash interface.
  * @param  f_object Flash instance used by the interface.
  * @param  ee_callback Callback invoked on interrupted operations.
  * @retval Possible return values: see @ref ee_itf_flash_status.
  */
ee_itf_flash_status EE_ITF_FLASH_Init(void *f_object, ee_itf_flash_callback_t ee_callback)
{
  ee_itf_flash_status retr = EE_ITF_FLASH_OK;
  (void)ee_callback;
  /* Save the nvm handle */
  NVMObject = (hal_nvm_handle_t *)f_object;

  return retr;
}

/**
  * @brief  Write data to the EDATA NVM flash interface.
  * @param  address Flash address.
  * @param  p_data Pointer to data to write.
  * @param  size Data size in bytes.
  * @retval Possible return values: see @ref ee_itf_flash_status.
  */
ee_itf_flash_status EE_ITF_FLASH_WriteData(uint32_t address, uint8_t *p_data, uint16_t size)
{
  ee_itf_flash_status status = EE_ITF_FLASH_ERROR_WRITE;

  (void)HAL_NVM_ITF_Unlock(NVMObject->instance);
  if (HAL_NVM_EDATA_ProgramByAddrAdapt(NVMObject, address, (const uint32_t *)p_data, size, 100) == HAL_OK)
  {
    status = EE_ITF_FLASH_OK;
  }
  (void)HAL_NVM_ITF_Lock(NVMObject->instance);

  return status;
}

/**
  * @brief  Read data from the EDATA NVM flash interface.
  * @param  address Flash address.
  * @param  p_data Output data buffer.
  * @param  size Data size in bytes.
  * @retval Possible return values: see @ref ee_itf_flash_status.
  */
ee_itf_flash_status EE_ITF_FLASH_ReadData(uint32_t address, uint8_t *p_data, uint16_t size)
{
  for (uint32_t i = 0; i < size; i++)
  {
    p_data[i] = *((uint8_t *)(address + i));
  }

  return EE_ITF_FLASH_OK;
}

/**
  * @brief  Erase pages in the EDATA NVM flash interface.
  * @param  address Page-aligned address.
  * @param  nb_pages Number of pages to erase.
  * @retval Possible return values: see @ref ee_itf_flash_status.
  */
ee_itf_flash_status EE_ITF_FLASH_PageErase(uint32_t address, uint16_t nb_pages)
{
  (void)(address);
  (void)(nb_pages);
  return EE_ITF_FLASH_NOTSUPPORTED;
}

/**
  * @brief  Erase pages using interrupt process in the EDATA NVM flash interface.
  * @param  address Page-aligned address.
  * @param  nb_pages Number of pages to erase.
  * @retval Possible return values: see @ref ee_itf_flash_status.
  */
ee_itf_flash_status EE_ITF_FLASH_PageErase_IT(uint32_t address, uint16_t nb_pages)
{
  (void)(address);
  (void)(nb_pages);
  return EE_ITF_FLASH_NOTSUPPORTED;
}

/**
  * @brief  Get last interrupted operation status for the EDATA NVM flash interface.
  * @param  address Output pointer for the interrupted operation address.
  * @retval Possible return values: see @ref ee_itf_flash_status.
  */
ee_itf_flash_status EE_ITF_FLASH_GetLastOperationStatus(uint32_t *address)
{
  ee_itf_flash_status status = EE_ITF_FLASH_OK;
  hal_nvm_interrupted_by_reset_operation_info_t p_info_by_reset;

  /* Get interrupted Operation information after a Reset */
  HAL_NVM_GetInterruptedByResetOperationInfo(NVMObject, &p_info_by_reset);
  *address = p_info_by_reset.interrupted_op_addr;
  switch (p_info_by_reset.interrupted_op_code)
  {
    case HAL_NVM_INTERRUPTED_SINGLE_WRITE:
      status = EE_ITF_FLASH_OPEINT_WRITE;
      break;
    case HAL_NVM_INTERRUPTED_BURST_WRITE:
      status = EE_ITF_FLASH_OPEINT_BURSTWRITE;
      break;

    default:
      /* not managed outside eeprom emulation competencies */
      break;
  }
  return status;
}

/**
  * @brief  Clear errors for the EDATA NVM flash interface.
  * @retval Possible return values: see @ref ee_itf_flash_status.
  */
ee_itf_flash_status EE_ITF_FLASH_ClearError(void)
{
  return EE_ITF_FLASH_NOTSUPPORTED;
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

/**
  * @}
  */
