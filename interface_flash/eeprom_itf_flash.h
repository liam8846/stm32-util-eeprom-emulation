/**
  ******************************************************************************
  * @file    eeprom_emulation/interface_flash/eeprom_itf_flash.h
  * @brief   This file contains all the functions prototypes for the EEPROM
  *          emulation flash interface.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef EEPROM_ITF_FLASH_H
#define EEPROM_ITF_FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32_hal.h"
/** @addtogroup EEPROM_EMULATION
  * @{
  */

/** @addtogroup EEPROM_EMULATION_INTERFACE
  * @{
  */

/** @defgroup EEPROM_EMULATION_INTERFACE_FLASH Interface FLASH
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup EEPROM_EMULATION_INTERFACE_FLASH_TYPEDEF Interface FLASH Types Definition
  * @{
  */

/**
  * @brief Define the status value returned by the flash interface.
  */
typedef enum
{
  EE_ITF_FLASH_OK = 0,            /*!< Operation successful           */

  EE_ITF_FLASH_OPEINT_WRITE,      /*!< Operation write interrupted by reset */
  EE_ITF_FLASH_OPEINT_BURSTWRITE, /*!< Operation burst write interrupted by reset */
  EE_ITF_FLASH_OPEINT_ERASED,     /*!< Operation erased interrupted by reset */
  EE_ITF_FLASH_NOTSUPPORTED,      /*!< Operation not supported        */

  EE_ITF_FLASH_ERROR,             /*!< error detected                 */
  EE_ITF_FLASH_ERROR_ERASE,       /*!< Error during erase operation   */
  EE_ITF_FLASH_ERROR_WRITE,       /*!< Error during write operation   */
  EE_ITF_FLASH_ERROR_ECCC         /*!< Error related to ECC           */

} ee_itf_flash_status;

/**
  * @brief Define the operation status corresponding return by ee_itf_flash_operation.
  */
typedef enum
{
  EE_ITF_FLASH_OPERATION_OK,      /*!< Operation successful           */
  EE_ITF_FLASH_OPERATION_ERROR,   /*!< error detected                 */
} ee_itf_flash_operation;


/**
  * @brief Define the callback prototype to receive an operation status.
  */
typedef void (*ee_itf_flash_callback_t)(ee_itf_flash_operation);

/**
  * @}
  */

/* Exported functions ------------------------------------------------------- */
/** @defgroup EEPROM_EMULATION_INTERFACE_FLASH_FUNCTIONS Interface FLASH Functions
  * @{
  */

ee_itf_flash_status EE_ITF_FLASH_Init(void *f_object, ee_itf_flash_callback_t ee_callback);
ee_itf_flash_status EE_ITF_FLASH_WriteData(uint32_t address, uint8_t *p_data, uint16_t size);
ee_itf_flash_status EE_ITF_FLASH_ReadData(uint32_t address, uint8_t *p_data, uint16_t size);
ee_itf_flash_status EE_ITF_FLASH_PageErase(uint32_t address, uint16_t nb_pages);
ee_itf_flash_status EE_ITF_FLASH_PageErase_IT(uint32_t address, uint16_t nb_pages);
ee_itf_flash_status EE_ITF_FLASH_GetLastOperationStatus(uint32_t *address);
ee_itf_flash_status EE_ITF_FLASH_ClearError(void);

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
#ifdef __cplusplus
}
#endif
#endif /* EEPROM_ITF_FLASH_H */
