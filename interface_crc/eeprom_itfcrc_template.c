/**
  ******************************************************************************
  * @file    eeprom_emulation/interface_crc/eeprom_itfcrc_template.c
  * @brief   This file provides a template implementation of the CRC interface.
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

/** @defgroup EEPROM_EMULATION_INTERFACE_CRC_TEMPLATE Interface CRC Template
  * @note this file is a template to build a custom CRC interface
  * @{
  */

/* Exported functions --------------------------------------------------------*/

/**
  * @addtogroup EEPROM_EMULATION_INTERFACE_CRC_FUNCTIONS
  * @{
  */


/** @defgroup EEPROM_EMULATION_INTERFACE_CRC_TEMPLATE_APIS Interface CRC Template APIs
  * @{
  */
/**
  * @brief  Initialize the CRC interface context (template).
  * @param  crc_object CRC instance used by the interface.
  * @retval Possible return values: see @ref ee_itf_crc_status.
  */
ee_itf_crc_status EE_ITF_CRC_Init(void *crc_object)
{
  (void)(crc_object);
  return EE_ITF_CRC_ERROR;
}

/**
  * @brief  Compute a 16-bit CRC value (template).
  * @param  crc_data Pointer to the data buffer used for CRC computation.
  * @param  data_size_byte Data size in bytes.
  * @param  crc_value Output pointer that receives the CRC value.
  * @retval Possible return values: see @ref ee_itf_crc_status.
  */
ee_itf_crc_status EE_ITF_CRC_Calcul16Bit(uint8_t *crc_data, uint16_t data_size_byte, uint16_t *crc_value)
{
  ee_itf_crc_status retr_status =  EE_ITF_CRC_ERROR;
  return retr_status;
}

/**
  * @brief  Compute an 8-bit CRC value (template).
  * @param  crc_data Pointer to the data buffer used for CRC computation.
  * @param  data_size_byte Data size in bytes.
  * @param  crc_value Output pointer that receives the CRC value.
  * @retval Possible return values: see @ref ee_itf_crc_status.
  */
ee_itf_crc_status EE_ITF_CRC_Calcul8Bit(uint8_t *crc_data, uint16_t data_size_byte, uint8_t *crc_value)
{
  ee_itf_crc_status retr_status =  EE_ITF_CRC_ERROR;
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
