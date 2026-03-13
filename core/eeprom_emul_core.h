/**
  ******************************************************************************
  * @file    eeprom_emulation/core/eeprom_emul_core.h
  * @brief   This file contains all the functions prototypes for the EEPROM
  *          emulation firmware library.
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
#ifndef EEPROM_EMUL_CORE_H
#define EEPROM_EMUL_CORE_H

/* Includes ------------------------------------------------------------------*/
#include "stm32_hal.h"
#include "eeprom_emul_conf.h"

#ifdef __cplusplus
extern "C" {
#endif


/** @defgroup EEPROM_EMULATION EEPROM Emulation
  * @{
  */

/** @defgroup EEPROM_EMULATION_CORE EEPROM Emulation Core
  * @{
  */

/* Exported typedef ---------------------------------------------------------*/

/** @defgroup EEPROM_EMULATION_EXPORTED_TYPEDEF Exported Typedef
  * @{
  */

/**
  * @brief EE object containing the peripheral information.
  *
  */
typedef struct
{
  void *f_object;     /*!< pointer to flash handle  */
  void *crc_object;   /*!< pointer to crc handle    */
} ee_object_t;

/**
  * @brief EE Status enum definition.
  *
  * Enumeration of different status codes for EEPROM operations.
  */
typedef enum
{
  /* return information code */
  EE_INFO_NOTSUPPORTED                     = 0x4U,  /*!< Operation is not supported and API call has not effect*/
  EE_INFO_CLEANUP_REQUIRED                 = 0x3U,  /*!< Cleanup operation is recommended */
  EE_INFO_NOERASING_PAGE                   = 0x1U,  /*!< no page to erase */
  EE_INFO_NODATA                           = 0x1U,  /*!< No data found */

  /* return code: action ok */
  EE_OK                                    = 0U,    /*!< The execution status is OK */

  /* return code: corresponding to an Invalid parameters */
  EE_INVALID_PARAM                         = -1,    /*!< Invalid parameter */
  EE_INVALID_VIRTUALADDRESS                = -2,    /*!< Invalid virtual address */

  /* return code: corresponding to an error */
  EE_ERROR_CORRUPTION                      = -3,    /*!< eeprom data are corrupted and not possible to recover */
  EE_ERROR_ALGO                            = -4,    /*!< algo return an error, the operation has net been executed */
  EE_ERROR_ITF_FLASH                       = -5,    /*!< Error return by the flash interface */
  EE_ERROR_ITF_CRC                         = -6,    /*!< Error return by the crc interface */
  EE_ERROR_ITF_ECC                         = -7,    /*!< Error return by the ecc interface */
} ee_status;


/**
  * @brief EE operation status value.
  */
typedef enum
{
  EE_OPERATION_NONE,      /*!< no operation */
  EE_OPERATION_ONGOING,   /*!< operation ongoing */
  EE_OPERATION_COMPLETE,  /*!< operation successfully complete */
  EE_OPERATION_ERROR,     /*!< operation error */
} ee_operation_status;

/**
  * @brief Type of page erasing.
  */
typedef enum
{
  EE_FORCED_ERASE,      /*!< pages to erase are erased unconditionally          */
  EE_CONDITIONAL_ERASE  /*!< pages to erase are erased only if not fully erased */
} ee_erase_type;


/**
  * @brief Define the callback prototype to receive an operation status.
  */
typedef void (*ee_operation_callback_t)(ee_operation_status);

/**
  * @}
  */

/* Private constants ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/** @defgroup EEPROM_EMULATION_EXPORTED_FUNCTIONS Exported Functions
  * @{
  */
ee_status EE_Init(ee_object_t *ee_object, ee_erase_type erase_type);
ee_status EE_Format(ee_erase_type erase_type);

ee_status EE_ReadVariable32bits(uint16_t virt_address, uint32_t *p_data);
ee_status EE_WriteVariable32bits(uint16_t virt_address, uint32_t data);
ee_status EE_ReadVariable96bits(uint16_t virt_address, uint64_t *p_data);
ee_status EE_WriteVariable96bits(uint16_t virt_address, uint64_t *data);

ee_status EE_ReadVariable16bits(uint16_t virt_address, uint16_t *p_data);
ee_status EE_WriteVariable16bits(uint16_t virt_address, uint16_t data);
ee_status EE_ReadVariable8bits(uint16_t virt_address, uint8_t *p_data);
ee_status EE_WriteVariable8bits(uint16_t virt_address, uint8_t data);
ee_status EE_CleanUp(void);
ee_status EE_CleanUp_IT(ee_operation_callback_t operation_complete);
ee_operation_status EE_GetStatusCleanUp_IT(void);

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

#endif /* EEPROM_EMUL_CORE_H */
