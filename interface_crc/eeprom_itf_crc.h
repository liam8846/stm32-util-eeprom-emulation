/**
  ******************************************************************************
  * @file    eeprom_emulation/interface_crc/eeprom_itf_crc.h
  * @brief   This file contains all the functions prototypes for the crc interface
  *          of the eeprom emulation.
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
#ifndef EEPROM_ITF_CRC_H
#define EEPROM_ITF_CRC_H

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

/** @defgroup EEPROM_EMULATION_INTERFACE_CRC Interface CRC
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup EEPROM_EMULATION_INTERFACE_CRC_TYPEDEF Interface CRC Types Definition
  * @{
  */

/**
  * @brief Define the status values returned by the CRC interface.
  */
typedef enum
{
  EE_ITF_CRC_OK    = 0,    /*!< CRC interface operation successful              */
  EE_ITF_CRC_ERROR = -1    /*!< An error has been detected in the CRC interface */
} ee_itf_crc_status;

/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/* Private types -------------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/** @defgroup EEPROM_EMULATION_INTERFACE_CRC_FUNCTIONS Interface CRC Functions
  * @{
  */

ee_itf_crc_status EE_ITF_CRC_Init(void *crc_object);
ee_itf_crc_status EE_ITF_CRC_Calcul16Bit(uint8_t *crc_data, uint16_t data_size_byte, uint16_t *crc_value);
ee_itf_crc_status EE_ITF_CRC_Calcul8Bit(uint8_t *crc_data, uint16_t data_size_byte, uint8_t *crc_value);

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
#endif /* EEPROM_ITF_CRC_H */
