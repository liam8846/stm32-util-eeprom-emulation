/**
  ******************************************************************************
  * @file    eeprom_emulation/interface_flash/eeprom_itfflash_flitf_edata.c
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

/** @defgroup EEPROM_EMULATION_INTERFACE_FLASH_FLITF_EDATA Interface FLASH FLITF EDATA
  * @{
  */

/* Private struct ---------------------------------------------------------*/
/** @defgroup EEPROM_EMULATION_INTERFACE_FLASH_FLITF_STRUCT_VAR  Interface FLASH FLITF Private Struct
  * @{
  */

/**
  * @brief struct definition to group local variable.
  */
typedef struct
{
  hal_flash_handle_t *FlashObject;      /*!< Pointer to flash handle                      */
  ee_itf_flash_callback_t ope_callback; /*!< operation callback function                  */
  __IO uint32_t eccd_flag;              /*!< flag to indicate an ECC error detection      */
  uint32_t eccd_address_start;          /*!< Address read start                           */
  uint32_t eccd_address_end;            /*!< Address read end                             */
  __IO uint8_t  operation;              /*!< Operation status 1 ongoing 0 no operation    */
} ee_itf_context_t;

/**
  * @}
  */

/* Private variables ---------------------------------------------------------*/
/** @defgroup EEPROM_EMULATION_INTERFACE_FLASH_FLITF_PRIVATE_VARIABLES  Interface FLASH FLITF Private Variables
  * @{
  */

/**
  * @brief local variable used to store all local data.
  */
static ee_itf_context_t ee_itf_context =
{
  .FlashObject = NULL,
  .ope_callback = NULL,
  .eccd_flag =   0,
};
/**
  * @}
  */

/* Private functions ---------------------------------------------------------*/
/** @defgroup EEPROM_EMULATION_INTERFACE_FLASH_FLITF_PRIVATE_FUNCTIONS  Interface FLASH FLITF Private Functions
  * @{
  */
static hal_status_t EE_ITF_ECCCallback(hal_flash_handle_t *hflash, hal_flash_bank_t bank);
static void EE_ITF_ErrorCallback(hal_flash_handle_t *hflash, hal_flash_bank_t bank);
static void EE_ITF_EraseCallback(hal_flash_handle_t *hflash, uint32_t flash_addr, uint32_t size_byte);
/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/**
  * @addtogroup EEPROM_EMULATION_INTERFACE_FLASH_FUNCTIONS
  * @{
  */

/** @defgroup EEPROM_EMULATION_INTERFACE_FLASH_FLITF_EDATA_APIS Interface FLASH FLITF Edata Functions
  * @{
  */
/**
  * @brief  Initialize the EDATA FLITF flash interface.
  * @param  f_object Flash handle used by the interface.
  * @param  ee_callback Callback invoked on interrupted operations.
  * @retval Possible return values: see @ref ee_itf_flash_status.
  */
ee_itf_flash_status EE_ITF_FLASH_Init(void *f_object, ee_itf_flash_callback_t ee_callback)
{
  ee_itf_flash_status status = EE_ITF_FLASH_ERROR;

  /* Store the flash handle and callback  */
  ee_itf_context.FlashObject = (hal_flash_handle_t *)f_object;
  ee_itf_context.ope_callback = ee_callback;

  /* Set Flash Programming mode to Double WORD  */
  if (HAL_FLASH_SetProgrammingMode(ee_itf_context.FlashObject, HAL_FLASH_PROGRAM_QUADWORD) != HAL_OK)
  {
    goto error;
  }

  /* Set the ECC error callback to report ECC error to eeprom emul */
  if (HAL_FLASH_RegisterECCErrorCallback(ee_itf_context.FlashObject, EE_ITF_ECCCallback) != HAL_OK)
  {
    goto error;
  }

  /* Set the Erase callback*/
  if (HAL_FLASH_RegisterEraseByAddrCpltCallback(ee_itf_context.FlashObject, EE_ITF_EraseCallback) != HAL_OK)
  {
    goto error;
  }

  /* Set the error  callback */
  if (HAL_FLASH_RegisterErrorCallback(ee_itf_context.FlashObject, EE_ITF_ErrorCallback) != HAL_OK)
  {
    goto error;
  }

  status = EE_ITF_FLASH_OK;
error:
  return status;
}

/**
  * @brief  Write data to the EDATA FLITF flash interface.
  * @param  address Flash address.
  * @param  p_data Pointer to data to write.
  * @param  size Data size in bytes.
  * @retval Possible return values: see @ref ee_itf_flash_status.
  */
ee_itf_flash_status EE_ITF_FLASH_WriteData(uint32_t address, uint8_t *p_data, uint16_t size)
{
  ee_itf_flash_status status = EE_ITF_FLASH_ERROR_WRITE;

  if (HAL_FLASH_ITF_Unlock(ee_itf_context.FlashObject->instance) == HAL_OK)
  {
    if (HAL_FLASH_EDATA_ProgramByAddrAdapt(ee_itf_context.FlashObject, address, (const uint32_t *)p_data, size,
                                           100) == HAL_OK)
    {
      status = EE_ITF_FLASH_OK;
    }
    (void)HAL_FLASH_ITF_Lock(ee_itf_context.FlashObject->instance);
  }
  return status;
}

/**
  * @brief  Read data from the EDATA FLITF flash interface.
  * @param  address Flash address.
  * @param  p_data Output data buffer.
  * @param  size Data size in bytes.
  * @retval Possible return values: see @ref ee_itf_flash_status.
  */
ee_itf_flash_status EE_ITF_FLASH_ReadData(uint32_t address, uint8_t *p_data, uint16_t size)
{
  ee_itf_flash_status status = EE_ITF_FLASH_OK;
  uint8_t *copy_pdata = p_data;

  /* reset the ECC flag, to catch ecc error corresponding to below read */
  ee_itf_context.eccd_flag = 0UL;
  ee_itf_context.eccd_address_start = address;
  ee_itf_context.eccd_address_end = address + size;
  __ISB();

  for (uint32_t index = 0; index < (size / sizeof(uint64_t)); index++)
  {
    *((uint64_t *)p_data) = (*(__IO uint64_t *)(address + (index * sizeof(uint64_t))));
    p_data = p_data + sizeof(uint64_t);
  }

  /* check ECC flag */
  if (ee_itf_context.eccd_flag == 1UL)
  {
    /* return invalid value ie 0xFF */
    for (uint32_t index = 0; index < (size / sizeof(uint64_t)); index++)
    {
      if (*((uint64_t *)copy_pdata) != 0xFFFFFFFFFFFFFFFFULL)
      {
        status = EE_ITF_FLASH_ERROR_ECCC;
        *((uint64_t *)copy_pdata) = 0xFFFFFFFFFFFFFFFFULL;
      }
      copy_pdata = copy_pdata + sizeof(uint64_t);
    }
  }
  return status;
}

/**
  * @brief  Erase pages in the EDATA FLITF flash interface.
  * @param  address Page-aligned address.
  * @param  nb_pages Number of pages to erase.
  * @retval Possible return values: see @ref ee_itf_flash_status.
  */
ee_itf_flash_status EE_ITF_FLASH_PageErase(uint32_t address, uint16_t nb_pages)
{
  ee_itf_flash_status status = EE_ITF_FLASH_ERROR_ERASE;
  uint32_t pages_size = (uint32_t)nb_pages * FLASH_EDATA_PAGE_SIZE;
  if (HAL_FLASH_ITF_Unlock(ee_itf_context.FlashObject->instance) == HAL_OK)
  {
    if (HAL_FLASH_EDATA_EraseByAddr(ee_itf_context.FlashObject, address, pages_size,
                                    100UL * (uint32_t)nb_pages) == HAL_OK)
    {
      status = EE_ITF_FLASH_OK;
    }
    (void)HAL_FLASH_ITF_Lock(ee_itf_context.FlashObject->instance);
  }
  return status;
}

/**
  * @brief  Erase pages using interrupt process in the EDATA FLITF flash interface.
  * @param  address Page-aligned address.
  * @param  nb_pages Number of pages to erase.
  * @retval Possible return values: see @ref ee_itf_flash_status.
  */
ee_itf_flash_status EE_ITF_FLASH_PageErase_IT(uint32_t address, uint16_t nb_pages)
{
  ee_itf_flash_status status = EE_ITF_FLASH_ERROR_ERASE;
  uint32_t pages_size = (uint32_t)nb_pages * FLASH_EDATA_PAGE_SIZE;
  if (HAL_FLASH_ITF_Unlock(ee_itf_context.FlashObject->instance) == HAL_OK)
  {
    ee_itf_context.operation = 1;
    if (HAL_FLASH_EraseByAddr_IT(ee_itf_context.FlashObject, address, pages_size) == HAL_OK)
    {
      status = EE_ITF_FLASH_OK;
    }
    else
    {
      ee_itf_context.operation = 0;
    }
  }
  return status;
}

/**
  * @brief  Get last interrupted operation status for the EDATA FLITF flash interface.
  * @param  address Output pointer for the interrupted operation address.
  * @retval Possible return values: see @ref ee_itf_flash_status.
  */
ee_itf_flash_status EE_ITF_FLASH_GetLastOperationStatus(uint32_t *address)
{
  ee_itf_flash_status status = EE_ITF_FLASH_OK;
  hal_flash_interrupted_by_reset_operation_info_t info;

  /* check if an operation has been interrupted since the latest reset */
  HAL_FLASH_GetInterruptedByResetOperationInfo(ee_itf_context.FlashObject, &info);
  *address = info.addr;
  switch (info.operation)
  {
    /* require to perform action to complete last operation */
    case HAL_FLASH_INTERRUPTED_SINGLE_WRITE:
      status = EE_ITF_FLASH_OPEINT_WRITE;
      break;
    case HAL_FLASH_INTERRUPTED_PAGE_ERASE:
      status = EE_ITF_FLASH_OPEINT_ERASED;
      break;
    default :
      /* not managed onside eeprom emulation competencies */
      break;
  }

  return status;
}

/**
  * @brief  Clear errors for the EDATA FLITF flash interface.
  * @retval Possible return values: see @ref ee_itf_flash_status.
  */
ee_itf_flash_status EE_ITF_FLASH_ClearError(void)
{
  return EE_ITF_FLASH_OK;
}

/**
  * @}
  */

/** @addtogroup EEPROM_EMULATION_INTERFACE_FLASH_FLITF_PRIVATE_FUNCTIONS
  * @{
  */

/**
  * @brief  Ecc callback management.
  *
  * This function is called when an ECC  failure occurs.
  * It retrieves the ECC failure information and invokes the appropriate callback function with the
  * address where the ECC failure occurred.
  *
  * @param  hflash: Pointer to the flash handle structure.
  * @param  bank:   Specifies the flash bank where the ECC failure occurred.
  *
  */
static hal_status_t EE_ITF_ECCCallback(hal_flash_handle_t *hflash, hal_flash_bank_t bank)
{
  hal_flash_ecc_info_t ecc_info;
  hal_status_t status = HAL_ERROR;

  HAL_FLASH_ECC_GetInfo(hflash, bank, &ecc_info);

  if (ecc_info.status == HAL_FLASH_ECC_CORRECTED)
  {
    status = HAL_OK;
  }
  else
  {
    if ((ecc_info.addr >= ee_itf_context.eccd_address_start) \
        && (ecc_info.addr <= ee_itf_context.eccd_address_end))
    {
      ee_itf_context.eccd_flag = 1;
      status = HAL_OK;
    }
  }
  return status;
}

/**
  * @brief  Erase callback management.
  *
  * This function is called when an Erase complete call back is done.
  * It retrieves the ECC failure information and invokes the appropriate callback function with the
  * address where the ECC failure occurred.
  *
  * @param  hflash: Pointer to the flash handle structure.
  * @param  flash_addr: flash address.
  * @param  size_byte: size in byte.
  *
  */
static void EE_ITF_EraseCallback(hal_flash_handle_t *hflash, uint32_t flash_addr, uint32_t size_byte)
{
  (void)(hflash);
  (void)(flash_addr);
  (void)(size_byte);
  /* Erase operation is complete */
  if (ee_itf_context.operation == 1U)
  {
    (void)HAL_FLASH_ITF_Lock(ee_itf_context.FlashObject->instance);
    ee_itf_context.operation = 0U;
    /* inform application that operation is successfully complete */
    if (ee_itf_context.ope_callback != NULL)
    {
      ee_itf_context.ope_callback(EE_ITF_FLASH_OPERATION_OK);
    }
  }
}

/**
  * @brief  Error callback management.
  *
  * This function is called when an Erase complete call back is done.
  * It retrieves the ECC failure information and invokes the appropriate callback function with the
  * address where the ECC failure occurred.
  *
  * @param  hflash: Pointer to the flash handle structure.
  * @param  bank:   Specifies the flash bank where the error occurred.
  *
  */
static void EE_ITF_ErrorCallback(hal_flash_handle_t *hflash, hal_flash_bank_t bank)
{
  (void)(hflash);
  (void)(bank);
  /* error operation detected */
  if (ee_itf_context.operation == 1U)
  {
    (void)HAL_FLASH_ITF_Lock(ee_itf_context.FlashObject->instance);
    ee_itf_context.operation = 0U;
    /* inform application that error occurs with an error */
    if (ee_itf_context.ope_callback != NULL)
    {
      ee_itf_context.ope_callback(EE_ITF_FLASH_OPERATION_OK);
    }
  }
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
