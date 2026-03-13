/**
  ******************************************************************************
  * @file    eeprom_emulation/core/eeprom_emul_core.c
  * @brief   This file provides all the EEPROM emulation firmware functions.
  @verbatim
  ==============================================================================
                        ##### How to use this driver #####
  ==============================================================================
    [..]
      This driver provides functions to initialize EEPROM emulation, to read and
      write EEPROM variables, and to cleanup FLASH pages used by EEPROM emulation.

      (#) EEPROM emulation initialization functions:
           (++) Format the FLASH pages used by EEPROM emulation using EE_Format().
                This function is optionally used, it can be called the very first
                time EEPROM emulation is used, to prepare FLASH pages for EEPROM
                emulation with empty EEPROM variables. It can also be called at
                any time, to flush all EEPROM variables.
           (++) Initialize EEPROM emulation, and restore the FLASH pages used by
                EEPROM emulation to a known good state in case of power loss
                using EE_Init(). It must be performed at system start up.

      (#) EEPROM variables access functions:
           (++) Write EEPROM variable using EE_WriteVariableXbits() functions
                A Clean Up request can be raised as return parameter in case
                FLASH pages used by EEPROM emulation, are full.
           (++) Read EEPROM variable using EE_ReadVariableXbits() functions

      (#) Clean up functions of FLASH pages, used by EEPROM emulation:
           (++) There Two modes of erasing:
            (+++) Polling mode using EE_CleanUp() function
            (+++) Interrupt mode using EE_CleanUp_IT() function
           (++) The completion of the operation can be checked by a callback
                function called pass parameter when the interrupt cleanup operation
                is completed or by calling the EE_GetSataus_Cleanup_IT() function
                which gives the status of the cleanup operation.

  @endverbatim
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
#include "eeprom_emul_core.h"
#if defined(EE_ALGO_FLITF)
#include "eeprom_algo_flitf.h"
#elif defined(EE_ALGO_NVM)
#include "eeprom_algo_nvm.h"
#else
#error "EEPROM_EMUL : no algo selected"
#endif /*EE_ALGO_FLITF*/

/** @addtogroup EEPROM_EMULATION
  * @{
  */

/** @addtogroup EEPROM_EMULATION_CORE
  * @{
  */

/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions -------------------------------------------------------*/

/** @addtogroup EEPROM_EMULATION_EXPORTED_FUNCTIONS
  * @{
  */

/**
  * @brief  Restore the pages to a known good state in case of power loss.
  *         If a page is in RECEIVE state, resume transfer.
  *         Then if some pages are ERASING state, erase these pages.
  * @param  ee_object pointer on eeprom_emulation object
  * @param  erase_type @ref ee_erase_type Type of erase to apply on page
  *         requiring to be erased.
  * @retval ee_status refer to @ref ee_status for possible return values
  *           - EE_OK: operation finished successfully
  *           - EE_ERROR_CORRUPTION(FLITF Algo): the algo was not able to recover data
  *           - EE_ERROR_FLASH_ITF : an error is returned by flash interface
  */
ee_status EE_Init(ee_object_t *ee_object, ee_erase_type erase_type)
{
#if defined(EE_ALGO_FLITF)
  return EE_FLITF_Init(ee_object, erase_type);
#elif defined(EE_ALGO_NVM)
  (void)(erase_type);
  return EE_NVM_Init(ee_object);
#endif /* EE_ALGO_FLITF */
}

/**
  * @brief  Returns the last stored variable data, if found, which correspond to the passed virtual address.
  * @param  virt_address Variable virtual address on 16 bits (can't be 0x0000 or 0xFFFF)
  * @param  p_data 96bits data to be written pointed by a uin64_t pointer
  * @retval ee_status refer to @ref ee_status
  *           - EE_OK: operation finished successfully
  *           - EE_ERROR_ALGO(FLITF Algo): if no active page found
  *           - EE_INVALID_VIRTUALADDRESS: if try to read from an invalid address
  *           - EE_INFO_NODATA: if no valid data is available
  *           - EE_INFO_NOTSUPPORTED: if called when ALGO NVM selected
  */
ee_status EE_ReadVariable96bits(uint16_t virt_address, uint64_t *p_data)
{
#if defined(EE_ALGO_FLITF)
  if ((virt_address != 0x0000UL) && (virt_address <= EE_NB_OF_VARIABLES))
  {
    return EE_FLITF_ReadVariable96bits(virt_address, p_data);
  }
  else
  {
    return EE_INVALID_VIRTUALADDRESS;
  }
#elif defined(EE_ALGO_NVM)
  (void)(virt_address);
  (void)(p_data);
  return EE_INFO_NOTSUPPORTED;
#endif /* EE_ALGO_FLITF */
}

/**
  * @brief  Returns the last stored variable data, if found, which correspond to the passed virtual address.
  * @param  virt_address Variable virtual address on 16 bits (can't be 0x0000 or 0xFFFF)
  * @param  p_data Variable containing the 32bits read variable value
  * @retval ee_status refer to @ref ee_status
  *           - EE_OK: operation finished successfully
  *           - EE_ERROR_ALGO(FLITF Algo): if no active page found
  *           - EE_INVALID_VIRTUALADDRESS: if try to read from an invalid address
  *           - EE_INFO_NODATA: if no valid data is available
  */
ee_status EE_ReadVariable32bits(uint16_t virt_address, uint32_t *p_data)
{
  ee_status status = EE_INVALID_VIRTUALADDRESS;
  if ((virt_address != 0x0000UL) && (virt_address <= EE_NB_OF_VARIABLES))
  {
#if defined(EE_ALGO_FLITF)
    status = EE_FLITF_ReadVariable32bits(virt_address, p_data);
#elif defined(EE_ALGO_NVM)
    if (virt_address > EE_NB_OF_8BITS_VARIABLES)
    {
      status = EE_NVM_ReadVariable32bits((uint16_t)(virt_address - 1UL), p_data);
    }
#endif /* EE_ALGO_FLITF */
  }
  return status;
}

/**
  * @brief  Returns the last stored variable data, if found, which correspond to the passed virtual address.
  * @param virt_address Variable virtual address on 16 bits (can't be 0x0000 or 0xFFFF)
  * @param  p_data Variable containing the 16bits read variable value
  * @retval ee_status refer to @ref ee_status
  *           - EE_OK: operation finished successfully
  *           - EE_ERROR_ALGO: if algo return an error
  *           - EE_INVALID_VIRTUALADDRESS: if try to read from an invalid address
  *           - EE_INFO_NODATA: if no valid data is available
  */
ee_status EE_ReadVariable16bits(uint16_t virt_address, uint16_t *p_data)
{
  ee_status status = EE_INVALID_VIRTUALADDRESS;
  if ((virt_address != 0x0000UL) && (virt_address <= EE_NB_OF_VARIABLES))
  {
#if defined(EE_ALGO_FLITF)
    status = EE_FLITF_ReadVariable16bits(virt_address, p_data);
#elif defined(EE_ALGO_NVM)
    if (virt_address > EE_NB_OF_8BITS_VARIABLES)
    {
      status = EE_NVM_ReadVariable16bits((uint16_t)(virt_address - 1UL), p_data);
    }
#endif /* EE_ALGO_FLITF */
  }
  return status;
}

/**
  * @brief  Returns the last stored variable data, if found, which correspond to the passed virtual address.
  * @param virt_address Variable virtual address on 16 bits (can't be 0x0000 or 0xFFFF)
  * @param  p_data Variable containing the 8bits read variable value
  * @retval ee_status refer to @ref ee_status
  *           - EE_OK: operation finished successfully
  *           - EE_ERROR_ALGO: if algo return an error
  *           - EE_INVALID_VIRTUALADDRESS: if try to read from an invalid address
  *           - EE_INFO_NODATA: if no valid data is available
  */
ee_status EE_ReadVariable8bits(uint16_t virt_address, uint8_t *p_data)
{
  ee_status status = EE_INVALID_VIRTUALADDRESS;

  if ((virt_address != 0x0000UL) && (virt_address <= EE_NB_OF_VARIABLES))
  {
#if defined(EE_ALGO_FLITF)
    status = EE_FLITF_ReadVariable8bits(virt_address, p_data);
#elif defined(EE_ALGO_NVM)
    if (virt_address <= EE_NB_OF_8BITS_VARIABLES)
    {
      status = EE_NVM_ReadVariable8bits((uint16_t)(virt_address - 1UL), p_data);
    }
#endif /* EE_ALGO_FLITF */
  }
  return status;
}

/**
  * @brief  Writes/updates variable data in EEPROM.
  *         Trig internal Pages transfer if half of the pages are full.
  * @param virt_address Variable virtual address on 16 bits (can't be 0x0000 or 0xFFFF)
  * @param  data 96bits data to be written pointed by a uin64_t pointer
  * @warning This function is not reentrant
  * @retval ee_status refer to @ref ee_status
  *           - EE_OK: operation finished successfully
  *           - EE_INVALID_VIRTUALADDRESS: if write in an invalid address
  *           - EE_ERROR_ALGO(FLITF Algo): if an error during writing operation
  *           - EE_ERROR_ITF_CRC(FLITF Algo): if an crc calculation error occurred
  *           - EE_INFO_CLEANUP_REQUIRED(FLITF Algo): success and user has to trig flash pages cleanup
  *           - EE_INFO_NOTSUPPORTED(NVM Algo): if called when ALGO NVM selected
  */
ee_status EE_WriteVariable96bits(uint16_t virt_address, uint64_t *data)
{
#if defined(EE_ALGO_FLITF)
  if ((virt_address != 0x0000UL) && (virt_address <= EE_NB_OF_VARIABLES))
  {
    return EE_FLITF_WriteVariable96bits(virt_address, data);
  }
  else
  {
    return EE_INVALID_VIRTUALADDRESS;
  }
#elif defined(EE_ALGO_NVM)
  (void)(virt_address);
  (void)(data);
  return EE_INFO_NOTSUPPORTED;
#endif /* EE_ALGO_FLITF */
}

/**
  * @brief  Writes/updates variable data in EEPROM.
  *         Trig internal Pages transfer if half of the pages are full.
  * @param virt_address Variable virtual address on 16 bits (can't be 0x0000 or 0xFFFF)
  * @param  data 32bits data to be written
  * @warning This function is not reentrant
  * @retval ee_status refer to @ref ee_status
  *           - EE_OK: operation finished successfully
  *           - EE_ERROR_ALGO: if an error during writing operation
  *           - EE_INVALID_VIRTUALADDRESS: if write in an invalid address
  *           - EE_ERROR_ITF_CRC(FLITF Algo): if an crc calculation error occurred
  *           - EE_INFO_CLEANUP_REQUIRED(FLITF Algo): success and user has to trig flash pages cleanup
  */
ee_status EE_WriteVariable32bits(uint16_t virt_address, uint32_t data)
{
  ee_status status = EE_INVALID_VIRTUALADDRESS;
  if ((virt_address != 0x0000UL) && (virt_address <= EE_NB_OF_VARIABLES))
  {
#if defined(EE_ALGO_FLITF)
    status = EE_FLITF_WriteVariable32bits(virt_address, data);
#elif defined(EE_ALGO_NVM)
    if (virt_address > EE_NB_OF_8BITS_VARIABLES)
    {
      status = EE_NVM_WriteVariable32bits((uint16_t)(virt_address - 1UL), data);
    }
#endif /* EE_ALGO_FLITF */
  }
  return status;
}

/**
  * @brief  Writes/updates variable data in EEPROM.
  *         Trig internal Pages transfer if half of the pages are full.
  * @param virt_address Variable virtual address on 16 bits (can't be 0x0000 or 0xFFFF)
  * @param  data 16bits data to be written
  * @warning This function is not reentrant
  * @retval ee_status refer to @ref ee_status
  *           - EE_OK: operation finished successfully
  *           - EE_INVALID_VIRTUALADDRESS: if write in an invalid address
  *           - EE_ERROR_ALGO(FLITF Algo): if no active page found
  *           - EE_ERROR_ITF_CRC(FLITF Algo): if an crc calculation error occurred
  *           - EE_INFO_CLEANUP_REQUIRED(FLITF Algo): success and user has to trig flash pages cleanup
  */
ee_status EE_WriteVariable16bits(uint16_t virt_address, uint16_t data)
{
  ee_status status = EE_INVALID_VIRTUALADDRESS;
  if ((virt_address != 0x0000UL) && (virt_address <= EE_NB_OF_VARIABLES))
  {
#if defined(EE_ALGO_FLITF)
    status = EE_FLITF_WriteVariable16bits(virt_address, data);
#elif defined(EE_ALGO_NVM)
    if (virt_address > EE_NB_OF_8BITS_VARIABLES)
    {
      status = EE_NVM_WriteVariable16bits((uint16_t)(virt_address - 1UL), data);
    }
#endif /* EE_ALGO_FLITF */
  }
  return status;
}

/**
  * @brief  Writes/updates variable data in EEPROM.
  *         Trig internal Pages transfer if half of the pages are full.
  * @param virt_address Variable virtual address on 16 bits (can't be 0x0000 or 0xFFFF)
  * @param  data 8bits data to be written
  * @warning This function is not reentrant
  * @retval ee_status refer to @ref ee_status
  *           - EE_OK: operation finished successfully
  *           - EE_INVALID_VIRTUALADDRESS: if write in an invalid address
  *           - EE_ERROR_ITF_CRC(FLITF Algo): if an crc calculation error occurred
  *           - EE_INFO_CLEANUP_REQUIRED(FLITF Algo): success and user has to trig flash pages cleanup
  */
ee_status EE_WriteVariable8bits(uint16_t virt_address, uint8_t data)
{
  ee_status status = EE_INVALID_VIRTUALADDRESS;
  if ((virt_address != 0x0000UL) && (virt_address <= EE_NB_OF_VARIABLES))
  {
#if defined(EE_ALGO_FLITF)
    status = EE_FLITF_WriteVariable8bits(virt_address, data);
#elif defined(EE_ALGO_NVM)
    if (virt_address <= EE_NB_OF_8BITS_VARIABLES)
    {
      status = EE_NVM_WriteVariable8bits((uint16_t)(virt_address - 1UL), data);
    }
#endif /* EE_ALGO_FLITF */
  }
  return status;
}

/**
  * @brief  Erase group of pages which are erasing state, in polling mode.
  *         Could be either first half or second half of total pages number.
  * @note   This function must be called when EE_WriteVariableXXbits has
  *         returned EE_INFO_CLEANUP_REQUIRED status (and only in that case)
  * @retval ee_status
  *           - EE_OK: in case of success
  *           - EE error code: if an error occurs
  *           - EE_INFO_NOTSUPPORTED(NVM Algo): if called when ALGO NVM selected
  */
ee_status EE_CleanUp(void)
{
#if defined(EE_ALGO_FLITF)
  return EE_FLITF_CleanUp();
#elif defined(EE_ALGO_NVM)
  return EE_INFO_NOTSUPPORTED;
#endif /* EE_ALGO_FLITF */
}

/**
  * @brief  Erase group of pages which are erasing state, in IT mode.
  *         Could be either first half or second half of total pages number.
  * @param  operation_complete this variable is used to return the operation status
  * @note   This function must be called when EE_WriteVariableXXbits has
  *         returned EE_INFO_CLEANUP_REQUIRED status (and only in that case)
  * @retval ee_status
  *           - EE_OK: in case of success
  *           - EE_INFO_NOERASING_PAGE(Algo FLITF): if no erase page found
  *           - EE_INFO_NOTSUPPORTED(NVM Algo): if called when ALGO NVM selected
  */
ee_status EE_CleanUp_IT(ee_operation_callback_t operation_complete)
{
#if defined(EE_ALGO_FLITF)
  return EE_FLITF_CleanUp_IT(operation_complete);
#elif defined(EE_ALGO_NVM)
  (void)(operation_complete);
  return EE_INFO_NOTSUPPORTED;
#endif /* EE_ALGO_FLITF */
}

/**
  * @brief  return the status of cleanup interrupt operation.
  * @note   This function must be called when calling EE_FLITF_CleanUp_IT to control
  *         the end of the operation and before any new eeprom operation.
  * @retval ee_operation_status: Refer to @ref ee_operation_status for possible return values.
  */
ee_operation_status EE_GetStatusCleanUp_IT(void)
{
#if defined(EE_ALGO_FLITF)
  return EE_FLITF_GetStatusCleanUp_IT();
#elif defined(EE_ALGO_NVM)
  return EE_OPERATION_NONE;
#endif /* EE_ALGO_FLITF */
}

/**
  * @brief  Erases all flash pages of eeprom emulation, and set first page header as ACTIVE.
  * @param  erase_type: @ref ee_erase_type Type of erase to apply on page
  *         requiring to be erased.
  * @note   This function can be called the very first time eeprom emulation is
  *         used, to prepare flash pages for eeprom emulation with empty eeprom
            variables. It can also be called at any time, to flush all eeprom
  *         variables.
  * @retval ee_status
  *           - EE_OK: on success
  *           - EE error code: if an error occurs
  *           - EE_INFO_NOTSUPPORTED(NVM Algo): if called when ALGO NVM selected
  */
ee_status EE_Format(ee_erase_type erase_type)
{
#if defined(EE_ALGO_FLITF)
  return EE_FLITF_Format(erase_type);
#elif defined(EE_ALGO_NVM)
  return EE_NVM_Format(erase_type);
#endif /* EE_ALGO_FLITF */
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
