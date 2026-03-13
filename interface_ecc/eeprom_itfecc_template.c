/**
  ******************************************************************************
  * @file    eeprom_emulation/interface_ecc/eeprom_itfecc_template.c
  * @brief   This file provides all the EEPROM emulation ecc bch stm32 interface functions.
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
#include "eeprom_itf_ecc.h"
/** @addtogroup EEPROM_EMULATION
  * @{
  */

/** @addtogroup EEPROM_EMULATION_INTERFACE
  * @{
  */

/** @addtogroup EEPROM_EMULATION_INTERFACE_ECC
  * @{
  */

/* Exported functions --------------------------------------------------------*/
/**
  * @addtogroup EEPROM_EMULATION_INTERFACE_ECC_FUNCTIONS
  * @{
  */

/** @defgroup EEPROM_EMULATION_INTERFACE_ECC_TEMPLATE_APIS Interface Template ECC APIs
  * @{
  */

/**
  * @brief  Initialize the ECC interface context (template).
  * @param  ecc_object ECC instance used by the interface.
  * @retval Possible return values: see @ref ee_itf_ecc_status.
  */
ee_itf_ecc_status EE_ITF_ECC_Init(void *ecc_object)
{
  return EE_ITF_ECC_ERROR;
}

/**
  * @brief  Compute ECC for a 20-bit protected value (template).
  * @param  data Data value to encode.
  * @param  ecc Output pointer that receives the ECC value.
  * @retval Possible return values: see @ref ee_itf_ecc_status.
  */
ee_itf_ecc_status EE_ITF_ECC_Calculation20BIT(uint32_t data, uint16_t *ecc)
{
  return EE_ITF_ECC_ERROR;
}

/**
  * @brief  Compute ECC for a 44-bit protected value (template).
  * @param  data Data value to encode.
  * @param  ecc Output pointer that receives the ECC value.
  * @retval Possible return values: see @ref ee_itf_ecc_status.
  */
ee_itf_ecc_status EE_ITF_ECC_Calculation44BIT(uint64_t data, uint16_t *ecc)
{
  return EE_ITF_ECC_ERROR;
}

/**
  * @brief  Check and correct a 20-bit protected value (template).
  * @param  data Input value to check.
  * @param  data_output Output pointer that receives the corrected value.
  * @retval Possible return values: see @ref ee_itf_ecc_status.
  */
ee_itf_ecc_status EE_ITF_ECC_Control20BIT(uint32_t data, uint32_t *data_output)
{
  return EE_ITF_ECC_ERROR;
}

/**
  * @brief  Check and correct a 44-bit protected value (template).
  * @param  data Input value to check.
  * @param  data_output Output pointer that receives the corrected value.
  * @retval Possible return values: see @ref ee_itf_ecc_status.
  */
ee_itf_ecc_status EE_ITF_ECC_Control44BIT(uint64_t data, uint64_t *data_output)
{
  return EE_ITF_ECC_ERROR;
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
