/**
  ******************************************************************************
  * @file    eeprom_emulation/core/algo/eeprom_algo_flitf.h
  * @brief   This file contains all the functions prototypes for the eeprom flitf
  *          algo firmware library.
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
#ifndef EEPROM_ALGO_FLITF_H
#define EEPROM_ALGO_FLITF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/** @addtogroup EEPROM_EMULATION
  * @{
  */

/** @addtogroup EEPROM_EMULATION_CORE
  * @{
  */


/** @defgroup EEPROM_EMULATION_ALGO_FLITF Algo FLITF
  * @{
  */

/* Exported typedef ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/** @defgroup EEPROM_EMULATION_ALGO_FLITF_EXPORTED_FUNCTIONS Algo FLITF Exported Functions
  * @{
  */
ee_status EE_FLITF_Init(ee_object_t *object, ee_erase_type erase_type);
ee_status EE_FLITF_Format(ee_erase_type erase_type);
ee_status EE_FLITF_ReadVariable32bits(uint16_t virt_address, uint32_t *p_data);
ee_status EE_FLITF_WriteVariable32bits(uint16_t virt_address, uint32_t data);
ee_status EE_FLITF_ReadVariable96bits(uint16_t virt_address, uint64_t *p_data);
ee_status EE_FLITF_WriteVariable96bits(uint16_t virt_address, uint64_t *p_data);
ee_status EE_FLITF_ReadVariable16bits(uint16_t virt_address, uint16_t *p_data);
ee_status EE_FLITF_WriteVariable16bits(uint16_t virt_address, uint16_t data);
ee_status EE_FLITF_ReadVariable8bits(uint16_t virt_address, uint8_t *p_data);
ee_status EE_FLITF_WriteVariable8bits(uint16_t virt_address, uint8_t data);
ee_status EE_FLITF_CleanUp(void);
ee_status EE_FLITF_CleanUp_IT(ee_operation_callback_t operation_complete);
ee_operation_status EE_FLITF_GetStatusCleanUp_IT(void);

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

#endif /* EEPROM_ALGO_FLITF_H */
