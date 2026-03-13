/**
  ******************************************************************************
  * @file    eeprom_emulation/interface_crc/eeprom_itfcrc_crc.c
  * @brief   This file provides an implementation of the CRC interface based on HAL CRC.
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
#include "eeprom_itf_crc.h"

/** @addtogroup EEPROM_EMULATION
  * @{
  */

/** @addtogroup EEPROM_EMULATION_INTERFACE
  * @{
  */

/** @addtogroup EEPROM_EMULATION_INTERFACE_CRC
  * @{
  */

/** @defgroup EEPROM_EMULATION_INTERFACE_CRC_STM32CRC Interface CRC STM32
  * @note this file contains the implementation for STM32 CRC peripheral
  * @{
  */

/* Private variables ---------------------------------------------------------*/

/** @defgroup EEPROM_EMULATION_INTERFACE_CRC_STM32CRC_PRIVATE_VARIABLES Interface CRC Private Variables
  * @{
  */

/**
  * @brief local variable to store the HAL CRC handle.
 */
hal_crc_handle_t *hCRC = NULL;

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/

/**
  * @addtogroup EEPROM_EMULATION_INTERFACE_CRC_FUNCTIONS
  * @{
  */

/** @defgroup EEPROM_EMULATION_INTERFACE_CRC_STM32_APIS Interface CRC STM32 APIS
  * @{
  */
/**
  * @brief  Initialize the CRC interface context.
  * @param  crc_object CRC instance used by the interface.
  * @retval Possible return values: see @ref ee_itf_crc_status.
  */
ee_itf_crc_status EE_ITF_CRC_Init(void *crc_object)
{
  /* Store the instance of the CRC into a local variable */
  hCRC = (hal_crc_handle_t *)crc_object;
  return EE_ITF_CRC_OK;
}

/**
  * @brief  Compute a 16-bit CRC value.
  * @param  crc_data Pointer to the data buffer used for CRC computation.
  * @param  data_size_byte Data size in bytes.
  * @param  crc_value Output pointer that receives the CRC value.
  * @retval Possible return values: see @ref ee_itf_crc_status.
  */
ee_itf_crc_status EE_ITF_CRC_Calcul16Bit(uint8_t *crc_data, uint16_t data_size_byte, uint16_t *crc_value)
{
  ee_itf_crc_status retr_status =  EE_ITF_CRC_OK;
  uint32_t crc = 0;

  /* finalize CRC calculation with the data value */
  if (HAL_CRC_Calculate(hCRC, crc_data, data_size_byte, &crc) != HAL_OK)
  {
    retr_status = EE_ITF_CRC_ERROR;
  }

  /* report the CRC calculation result */
  *crc_value = (uint16_t)crc;
  return retr_status;
}

/**
  * @brief  Compute an 8-bit CRC value.
  * @param  crc_data Pointer to the data buffer used for CRC computation.
  * @param  data_size_byte Data size in bytes.
  * @param  crc_value Output pointer that receives the CRC value.
  * @retval Possible return values: see @ref ee_itf_crc_status.
  */
ee_itf_crc_status EE_ITF_CRC_Calcul8Bit(uint8_t *crc_data, uint16_t data_size_byte, uint8_t *crc_value)
{
  ee_itf_crc_status retr_status =  EE_ITF_CRC_OK;
  uint32_t crc = 0;

  /* start CRC calculation with the data pointer */
  if (HAL_CRC_Calculate(hCRC, crc_data, data_size_byte * sizeof(uint8_t), &crc) != HAL_OK)
  {
    retr_status = EE_ITF_CRC_ERROR;
  }
  /* report the CRC calculation result */
  *crc_value = (uint8_t)crc;
  return retr_status;
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
