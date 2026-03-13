/**
  ******************************************************************************
  * @file    eeprom_emulation/interface_ecc/eeprom_itf_ecc.h
  * @brief   This file contains all the functions prototypes for the EEPROM
  *          emulation ecc interface.
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
#ifndef EEPROM_ITF_ECC_H
#define EEPROM_ITF_ECC_H

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

/** @defgroup EEPROM_EMULATION_INTERFACE_ECC  Interface ECC
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup EEPROM_EMULATION_INTERFACE_ECC_TYPEDEF Interface ECC Types Definition
  * @{
  */

/**
  * @brief Define the status values returned by the ECC interface.
  */
typedef enum
{
  EE_ITF_ECC_OK = 0,           /*!< ECC interface operation successful                   */
  EE_ITF_ECC_DATA_UPDATED = 1, /*!< Data has been updated due to error correction        */
  EE_ITF_ECC_ERROR = -1        /*!< An error has occurred in the ECC interface operation */
} ee_itf_ecc_status;

/**
  * @}
  */

/* Exported functions ---------------------------------------------------------*/

/** @defgroup EEPROM_EMULATION_INTERFACE_ECC_FUNCTIONS Interface ECC Functions
  * @{
  */

ee_itf_ecc_status EE_ITF_ECC_Init(void *ecc_object);
ee_itf_ecc_status EE_ITF_ECC_Calculation20BIT(uint32_t data, uint16_t *ecc);
ee_itf_ecc_status EE_ITF_ECC_Calculation44BIT(uint64_t data, uint16_t *ecc);
ee_itf_ecc_status EE_ITF_ECC_Control20BIT(uint32_t data, uint32_t *data_output);
ee_itf_ecc_status EE_ITF_ECC_Control44BIT(uint64_t data, uint64_t *data_output);

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
#endif /* EEPROM_ITF_ECC_H */
