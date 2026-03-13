/**
  ******************************************************************************
  * @file    eeprom_emulation/core/algo/eeprom_algo_nvm.c
  * @brief   This file provides all the nvm algo firmware functions.
@verbatim
==============================================================================
##### How to use this driver #####
==============================================================================
[..]
This driver provides functions to initialize EEPROM emulation, to read and
write EEPROM variables, and to cleanup NVM pages used by EEPROM emulation.

(#) EEPROM emulation initialization functions:
(++) Format the NVM pages used by EEPROM emulation using EE_Format().
This function is optionally used, it can be called the very first
time EEPROM emulation is used, to prepare NVM pages for EEPROM
emulation with empty EEPROM variables. It can also be called at
any time, to flush all EEPROM variables.
(++) Initialize EEPROM emulation. It must be performed at system start up.

(#) EEPROM variables access functions:
(++) Write EEPROM variable using EE_WriteVariableXbits() functions
(++) Read EEPROM variable using EE_ReadVariableXbits() functions
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
#include "eeprom_algo_nvm.h"
#include "eeprom_itf_flash.h"
#include "eeprom_itf_crc.h"
#include "eeprom_itf_ecc.h"
#include <string.h>

/** @addtogroup EEPROM_EMULATION_ALGO_NVM
  * @{
  */

/* Private define -----------------------------------------------------------*/
/** @defgroup EEPROM_EMULATION_ALGO_NVM_PRIVATE_DEFINES Algo NVM Private Defines
  * @{
  */

/**
  * @brief  this macro is used to disable the debug information.
  * @note   to get debug information, the implementation of the macro must be done inside the configuration
  */
#if !defined(EE_DEBUG_PRINT)
#define EE_DEBUG_PRINT(...)
#endif /* !defined(EE_DEBUG_PRINT) */


/**
  * @brief  this macro is used to get 8bits data frame.
  */
#define EE_8BITS_FRAME_SIZE  (sizeof(ee_data_frame_8bits))

/**
  * @brief  this macro is used to get 32bits data frame.
  */
#define EE_32BITS_FRAME_SIZE (sizeof(ee_data_frame_32bits))

/**
  * @brief  a define of an offset used to calculate offset counter.
  */
#define EE_OFFSET_MODULO   0x10U

/**
  * @brief  this macro is used to get the data size requested.
  */
#define EE_DATA_SIZE    ((EE_NB_OF_8BITS_VARIABLES * EE_8BITS_FRAME_SIZE)\
                         + (EE_NB_OF_16_32BITS_VARIABLES * EE_32BITS_FRAME_SIZE))

/**
  * @brief  this macro is used to get the number of page to store the requested data size.
  */
#define EE_NBPAGE_TOSTOREDATA  ((uint32_t)(EE_DATA_SIZE / EE_SECTOR_SIZE ) + 1U)

/**
  * @brief  this macro is used to get the number of page to store the requested data size.
  */
#define EE_NBPAGE_SOLUTION     (EE_NBPAGE_TOSTOREDATA * EE_CYCLES_NUMBER)

/**
  * @}
  */

/* Private typedef -----------------------------------------------------------*/

/** @defgroup EEPROM_EMULATION_ALGO_NVM_PRIVATE_TYPEDEF Algo NVM Private Typedef
  * @{
  */

/**
  * @brief Data format for 8bits data.
  *
  * @note  the frame for 8bits is the following
  *        Data (8bits), OffsetCounter (4bits), CRC(8bits), ECC (10bits + 1bit parity)
  *        CRC is calculated for the frame: Data + OffsetCounter
  *        ECC is calculated for the frame: Data + OffsetCounter + CRC
  */
typedef union
{
  struct
  {
    uint32_t Ecc           :11u; /*!< ECC value */
    uint32_t Crc           :8u;  /*!< CRC value */
    uint32_t OffsetCounter :4u;  /*!< OffsetCounter value aligned with the value EE_OFFSET_MODULO */
    uint32_t DataValue     :8u;  /*!< Data value */
    uint32_t               :1u;
  } EE_FrameStruct_8BITS;        /*!< Frame struct */

  struct
  {
    uint32_t               :19u;
    uint32_t CRCInput      :12u; /*!< CRC input used for crc calculation */
    uint32_t               :1;
  } EE_CRCCalcul_8BITS;          /*!< CRC calculation struct */

  struct
  {
    uint32_t               :11u;
    uint32_t EccInput      :20u; /*!< ECC input used for ecc calculation */
    uint32_t               :1;
  } EE_ECCCalcul_8BITS;          /*!< ECC calculation struct */
  uint32_t Data32bits;           /*!< Frame value */
} ee_data_frame_8bits;

/**
  * @brief Data format for 32bits data (available to store 16bits data).
  *
  * @note  The frame for 32bits is the following
  *        Data (32bits), OffsetCounter(4bits) CRC(8bits), ECC (12bits + 1bit parity)
  *        CRC is calculated for the frame: Data + OffsetCounter
  *        ECC is calculated for the frame: Data + OffsetCounter + CRC
  */
typedef union
{

  struct
  {
    uint64_t Ecc : 13;          /*!< ECC value */
    uint64_t Crc : 8;           /*!< CRC value */
    uint64_t OffsetCounter : 4; /*!< OffsetCounter value aligned with the value EE_OFFSET_MODULO */
    uint64_t DataValue : 32;    /*!< Data value */
    uint64_t : 7;
  } EE_FrameStruct_32BITS; /*!< Frame struct */
  struct
  {
    uint64_t : 21u;
    uint64_t CRCInput : 36u; /*!< CRC input used for crc calculation */
    uint64_t : 7;
  } EE_CRCCalcul_32BITS; /*!< CRC calculation struct*/
  struct
  {
    uint64_t : 13u;
    uint64_t EccInput : 44u; /*!< ECC input used for ecc calculation */
    uint64_t : 7;
  } EE_ECCCalcul_32BITS; /*!< ECC calculation struct */
  uint64_t Data64bits;   /*!< Frame value */
} ee_data_frame_32bits;

/**
  * @brief type of data.
  */
typedef enum
{
  EE_DATA_8BITS,   /*!< 8bits data  */
  EE_DATA_32BITS,  /*!< 16bits data */
} ee_data_type_t;

/**
  * @}
  */

/* Private function prototypes -----------------------------------------------*/
/** @defgroup EEPROM_EMULATION_ALGO_NVM_PRIVATE_FUNCTIONS Algo NVM Private Functions
  * @{
  */

static ee_status ee_GetNextAddressAndOffset(uint32_t var_offset, ee_data_type_t data_type,
                                            uint32_t *next_address, uint8_t *next_offset_counter);

static ee_status ee_GetCurrentAddress(uint32_t var_offset, ee_data_type_t data_type, uint32_t *current_address);

static ee_status ee_GetData(uint32_t var_address, void *pdata, uint8_t *p_offset_counter, ee_data_type_t data_type);

static inline uint32_t ee_Get_VarOffset8bit(uint16_t var_idx);
static inline uint32_t ee_Get_VarOffset32bit(uint16_t var_idx);

static uint32_t ee_EncodeData8bits(uint8_t data, uint8_t offset_counter);
static uint64_t ee_EncodeData32bits(uint32_t data, uint8_t offset_counter);
/**
  * @}
  */

/** @addtogroup EEPROM_EMULATION_ALGO_NVM_EXPORTED_FUNCTIONS
  * @{
  */

/**
  * @brief  Initialize the eeprom emulation and all used interfaces.
  * @param  object pointer on eeprom_emulation object
  * @retval ee_status refer to @ref ee_status
  *           - EE_OK: operation finished successfully
  *           - EE_ERROR_ALGO: if an error occurred during initialization
  */
ee_status EE_NVM_Init(ee_object_t *object)
{
  uint32_t address;
  ee_status status = EE_OK;
  /* Initialize the flash interface */
  (void)EE_ITF_FLASH_Init(object->f_object, NULL);
  /* Initialize the crc interface */
  (void)EE_ITF_CRC_Init(object->crc_object);
  /* Initialize the ecc interface */
  (void)EE_ITF_ECC_Init(NULL);

  /* Check for interrupted operation after reset */
  switch (EE_ITF_FLASH_GetLastOperationStatus(&address))
  {
    case EE_ITF_FLASH_OPEINT_WRITE:
    case EE_ITF_FLASH_OPEINT_BURSTWRITE:
      if ((address >= EE_START_PAGE_ADDRESS) \
          && (address < (EE_START_PAGE_ADDRESS + (EE_NBPAGE_SOLUTION * EE_SECTOR_SIZE))))
      {
        uint64_t data = 0xFFFFFFFFFFFFFFFFU;
        /* check if address is allocated for 8 bit data type */

        uint32_t address_offset = (address - EE_START_PAGE_ADDRESS) % (EE_NBPAGE_TOSTOREDATA * EE_SECTOR_SIZE);
        if (address_offset < (EE_NB_OF_8BITS_VARIABLES * EE_8BITS_FRAME_SIZE))
        {
          /* Delete corrupted address in 4 bytes alignment */
          if (EE_ITF_FLASH_WriteData(address, (uint8_t *)&data, (uint16_t)sizeof(uint32_t)) != EE_ITF_FLASH_OK)
          {
            status = EE_ERROR_ALGO;
          }
        }
        else
        {
          /* Delete corrupted address in 8 bytes alignment */
          if (EE_ITF_FLASH_WriteData(address, (uint8_t *)&data, (uint16_t)sizeof(uint64_t)) != EE_ITF_FLASH_OK)
          {
            status = EE_ERROR_ALGO;
          }
        }
      }
      break;
    default:
      /* nothing to do */
      break;
  }

  return status;
}

/**
  * @brief  Format all NVM pages allocated for eeprom emulation.
  * @param  erase_type /ref ee_erase_type Type of erase to apply on page
  *         requiring to be erased.
  * @note   This function can be called the very first time eeprom emulation is
  *         used, to prepare NVM pages for eeprom emulation with empty eeprom
            variables. It can also be called at any time, to flush all eeprom
  *         variables.
  * @retval ee_status /ref ee_status
  *           - EE_OK: operation finished successfully
  *           - EE_ERROR_ALGO: if an error occurred during memory formatting
  */
ee_status EE_NVM_Format(ee_erase_type erase_type)
{
  (void)(erase_type);
  uint64_t data = 0xFFFFFFFFFFFFFFFFU;
  uint32_t address = EE_START_PAGE_ADDRESS;
  uint32_t end_address = EE_START_PAGE_ADDRESS + (EE_NBPAGE_SOLUTION * EE_SECTOR_SIZE);

  while (address < end_address)
  {
    if (EE_ITF_FLASH_WriteData(address, (uint8_t *)&data, (uint16_t)sizeof(uint64_t)) != EE_ITF_FLASH_OK)
    {
      return EE_ERROR_ALGO;
    }
    else
    {
      address += sizeof(uint64_t);
    }
  }
  return EE_OK;
}

/**
  * @brief  Returns the last stored variable data, if found, which correspond to the passed virtual address.
  * @param virt_address Variable virtual address on 16 bits
  * @param p_data 8bits data to be written
  * @retval ee_status refer to @ref ee_status
  *           - EE_OK: operation finished successfully
  *           - EE_INFO_NODATA: if no valid data found
  */
ee_status EE_NVM_ReadVariable8bits(uint16_t virt_address, uint8_t *p_data)
{
  ee_status status = EE_INFO_NODATA;
  uint32_t flash_address;
  uint32_t var_offset;
  uint8_t OffsetCounter;
  ee_data_frame_8bits DataValue =  {.Data32bits = 0};
  var_offset = ee_Get_VarOffset8bit(virt_address);

  /* Get the physical address location of last valid data */
  if (ee_GetCurrentAddress(var_offset, EE_DATA_8BITS, &flash_address) == EE_OK)
  {
    /* Read the data */
    if (ee_GetData(flash_address, (void *)&DataValue, &OffsetCounter, EE_DATA_8BITS) == EE_OK)
    {
      *p_data = (uint8_t)DataValue.EE_FrameStruct_8BITS.DataValue;
      status = EE_OK;
    }
  }
  return status;
}

/**
  * @brief  Returns the last stored variable data, if found, which correspond to the passed virtual address.
  * @param virt_address Variable virtual address on 16 bits
  * @param p_data 16bits data to be written
  * @retval ee_status refer to @ref ee_status
  *           - EE_OK: operation finished successfully
  *           - EE_INFO_NODATA: if no valid data found
  */
ee_status EE_NVM_ReadVariable16bits(uint16_t virt_address, uint16_t *p_data)
{
  uint32_t Data32bits;
  ee_status status = EE_NVM_ReadVariable32bits(virt_address, &Data32bits);

  *p_data = (uint16_t)Data32bits;

  return status;
}

/**
  * @brief  Returns the last stored variable data, if found, which correspond to the passed virtual address.
  * @param virt_address Variable virtual address on 16 bits (can't be 0x0000 or 0xFFFF)
  * @param p_data 32bits data to be written
  * @retval ee_status refer to @ref ee_status
  *           - EE_OK: operation finished successfully
  *           - EE_INFO_NODATA: if no valid data found
  */
ee_status EE_NVM_ReadVariable32bits(uint16_t virt_address, uint32_t *p_data)
{
  ee_status status = EE_INFO_NODATA;

  uint32_t FlashAddress;
  uint32_t varOffset;
  uint8_t OffsetCounter;
  ee_data_frame_32bits DataVal = {.Data64bits = 0};

  varOffset = ee_Get_VarOffset32bit(virt_address);
  /* Get the physical address location of last valid data */
  if (ee_GetCurrentAddress(varOffset, EE_DATA_32BITS, &FlashAddress) == EE_OK)
  {
    /* Read the data */
    if (ee_GetData(FlashAddress, (void *)&DataVal, &OffsetCounter, EE_DATA_32BITS) == EE_OK)
    {
      *p_data = (uint32_t)DataVal.EE_FrameStruct_32BITS.DataValue;
      status = EE_OK;
    }
  }
  return status;
}

/**
  * @brief  Writes/updates variable data in EEPROM.
  * @param  virt_address Variable virtual address on 16 bits
  * @param  data 8bits data to be written
  * @warning This function is not reentrant
  * @retval ee_status refer to @ref ee_status
  *           - EE_OK: operation finished successfully
  *           - EE_ERROR_ALGO: if an error occurred
  */
ee_status EE_NVM_WriteVariable8bits(uint16_t virt_address, uint8_t data)
{
  ee_status status = EE_ERROR_ALGO;
  uint32_t FlashAddress;
  uint32_t varOffset;
  uint8_t OffsetCounter;
  uint32_t DataValue;

  varOffset = ee_Get_VarOffset8bit(virt_address);

  /* Get the physical address location and the offset counter */
  (void)ee_GetNextAddressAndOffset(varOffset, EE_DATA_8BITS, &FlashAddress, &OffsetCounter);
  /* set data value */
  DataValue = ee_EncodeData8bits(data, OffsetCounter);
  /* Write data in Flash */
  if (EE_ITF_FLASH_WriteData(FlashAddress, (uint8_t *)&DataValue, (uint16_t)sizeof(uint32_t)) == EE_ITF_FLASH_OK)
  {
    status = EE_OK;
  }
  return status;
}

/**
  * @brief  Writes/updates variable data in EEPROM.
  * @param  virt_address Variable virtual address on 16 bits
  * @param  data 16bits data to be written
  * @warning This function is not reentrant
  * @retval ee_status refer to @ref ee_status
  *           - EE_OK: operation finished successfully
  *           - EE_ERROR_ALGO: if an error occurred
  */
ee_status EE_NVM_WriteVariable16bits(uint16_t virt_address, uint16_t data)
{
  return EE_NVM_WriteVariable32bits(virt_address, (uint32_t)data);
}

/**
  * @brief  Writes/updates variable data in EEPROM.
  * @param  virt_address Variable virtual address on 16 bits
  * @param  data 32bits data to be written
  * @warning This function is not reentrant
  * @retval ee_status refer to @ref ee_status
  *           - EE_OK: operation finished successfully
  *           - EE_ERROR_ALGO: if an error occurred
  */
ee_status EE_NVM_WriteVariable32bits(uint16_t virt_address, uint32_t data)
{
  ee_status status = EE_ERROR_ALGO;

  /* the var_idx is a valid id */
  uint32_t varOffset;
  uint64_t DataValue;
  uint32_t FlashAddress;
  uint8_t OffsetCounter;

  /* get offset position of the variables */
  varOffset = ee_Get_VarOffset32bit(virt_address);

  /* Get the physical address location and the offset counter */
  (void)ee_GetNextAddressAndOffset(varOffset, EE_DATA_32BITS, &FlashAddress, &OffsetCounter);
  /* set data value */
  DataValue = ee_EncodeData32bits(data, OffsetCounter);
  /* Write data in Flash */
  if (EE_ITF_FLASH_WriteData(FlashAddress, (uint8_t *)&DataValue, (uint16_t)sizeof(uint64_t)) == EE_ITF_FLASH_OK)
  {
    status = EE_OK;
  }
  return status;
}

/**
  * @}
  */

/** @addtogroup EEPROM_EMULATION_ALGO_NVM_PRIVATE_FUNCTIONS
  * @{
  */

/**
  * @brief Encode 8Bit data.
  * @param data data value
  * @param offset_counter offset counter value
  * @return the encoded value
  */
static uint32_t ee_EncodeData8bits(uint8_t data, uint8_t offset_counter)
{
  ee_data_frame_8bits lData;
  uint8_t CRCValue;
  uint32_t CRCInput;
  uint32_t ECCInput;
  uint16_t ecc_value;

  lData.Data32bits = 0;
  lData.EE_FrameStruct_8BITS.DataValue = data;
  lData.EE_FrameStruct_8BITS.OffsetCounter = offset_counter;

  CRCInput = lData.EE_CRCCalcul_8BITS.CRCInput;

  /* Calculate the CRC */
  (void)EE_ITF_CRC_Calcul8Bit((uint8_t *)&CRCInput, 2, &CRCValue);

  lData.EE_FrameStruct_8BITS.Crc = CRCValue;
  ECCInput = lData.EE_ECCCalcul_8BITS.EccInput;
  /* Calculate the ECC */
  (void)EE_ITF_ECC_Calculation20BIT(ECCInput, &ecc_value);
  lData.EE_FrameStruct_8BITS.Ecc = ecc_value;

  return lData.Data32bits;
}

/**
  * @brief Encode 32Bit data.
  * @param data data value
  * @param offset_counter offset counter value
  * @return the encoded value
  */
static uint64_t ee_EncodeData32bits(uint32_t data, uint8_t offset_counter)
{
  ee_data_frame_32bits lData;
  uint8_t CRCValue;
  uint64_t CRCInput;
  uint64_t ECCInput;
  uint16_t ecc_value;
  lData.Data64bits = 0;
  lData.EE_FrameStruct_32BITS.DataValue = data;
  lData.EE_FrameStruct_32BITS.OffsetCounter = offset_counter;

  CRCInput = lData.EE_CRCCalcul_32BITS.CRCInput;

  /* Calculate the CRC */
  (void)EE_ITF_CRC_Calcul8Bit((uint8_t *)&CRCInput, 5, &CRCValue);

  lData.EE_FrameStruct_32BITS.Crc = CRCValue;
  ECCInput = lData.EE_ECCCalcul_32BITS.EccInput;

  /* Calculate the ECC */
  (void)EE_ITF_ECC_Calculation44BIT(ECCInput, &ecc_value);
  lData.EE_FrameStruct_32BITS.Ecc = ecc_value;

  return lData.Data64bits;
}

/**
  * @brief get the variable offset for 8bit data.
  * @param var_idx variable ID
  * @return the variable offset
  */
static inline uint32_t ee_Get_VarOffset8bit(uint16_t var_idx)
{
  return var_idx * EE_8BITS_FRAME_SIZE;
}

/**
  * @brief get the variable offset for 32bit data.
  * @param var_idx variable ID
  * @return the variable offset
  */
static inline uint32_t ee_Get_VarOffset32bit(uint16_t var_idx)
{
  return ((EE_NB_OF_8BITS_VARIABLES * EE_8BITS_FRAME_SIZE) + (((uint32_t)var_idx - EE_NB_OF_8BITS_VARIABLES) *
                                                              EE_32BITS_FRAME_SIZE));
}

/**
  * @brief This function calculate the next physical address of data and next_offset_counter value to write.
  *
  * @param var_offset     variable offset
  * @param data_type      type of variable @ref ee_data_type_t
  * @param next_address   next physical address of the data
  * @param next_offset_counter next offsetcounter value to write
  *
  * @return ee_status    EE_OK     return valid Address/OffsetCounter
  *                      EE_ERROR_ALGO  in case of non valid information on the variable
  */
static ee_status ee_GetNextAddressAndOffset(uint32_t var_offset, ee_data_type_t data_type,
                                            uint32_t *next_address, uint8_t *next_offset_counter)
{
  ee_status status = EE_OK;
  uint8_t counter_offset;
  uint8_t counter_invaliddata = 0U;
  uint8_t min = 0xFFU;
  uint8_t max = 0xFFU;
  uint8_t position_min = 0;
  uint8_t position_max = 0;
  uint64_t Data;

  /* read the data to get the offset value */
  for (uint8_t readindex = 0; readindex < EE_CYCLES_NUMBER; readindex++)
  {
    if (ee_GetData(var_offset + ((uint32_t)readindex * EE_NBPAGE_TOSTOREDATA * EE_SECTOR_SIZE) + EE_START_PAGE_ADDRESS,
                   &Data, &counter_offset, data_type) != EE_OK)

    {
      EE_DEBUG_PRINT("\n read_index : %d, counter_offset: %d\n", readindex, counter_offset);
      /* count the invalid data */
      counter_invaliddata++;
    }
    else
    {
      /* Calculation of the maximum offset value */
      if ((counter_offset > (EE_CYCLES_NUMBER)) && ((max == 0xFFU) || (max < counter_offset)))
      {
        max = counter_offset;
        position_max = readindex;
      }

      /* Calculation of the maximum offset value */
      if ((counter_offset < (EE_CYCLES_NUMBER + 1U)) && ((min == 0xFFU) || (min < counter_offset)))
      {
        min = counter_offset;
        position_min = readindex;
      }
    }
  }

  switch (counter_invaliddata)
  {
    /* All values are invalid, the first valid offset is the first */
    case EE_CYCLES_NUMBER:
    {
      *next_address = 0U;
      *next_offset_counter = 0U;
      status = EE_INFO_NODATA;
      break;
    }

    /* the system doesn't care there is corrupted data
       only the latest valid data is taken into account
       thanks to min/max and the corrupted data are rewritten
       by the algorithm */
    default:
    {
      if ((max == 0xFFU)
          || ((min != 0xFFU) && (max == (EE_OFFSET_MODULO - 1U))))
      {
        /* only min is considered */
        *next_address = ((uint32_t)position_min + 1U) % EE_CYCLES_NUMBER;
        *next_offset_counter = (min + 1U) % (EE_OFFSET_MODULO);
      }
      else
      {
        /* only max is considered */
        *next_address = ((uint32_t)position_max + 1U) % EE_CYCLES_NUMBER;
        *next_offset_counter = (max + 1U) % (EE_OFFSET_MODULO);
      }
      break;
    }
  }

  /* Calculate the data Address */
  *next_address = var_offset + ((*next_address) * EE_NBPAGE_TOSTOREDATA * EE_SECTOR_SIZE) + EE_START_PAGE_ADDRESS;

  EE_DEBUG_PRINT("ee_GetNextAddressAndOffset::Address::0x%x Offset[%d]\n", *next_address, *next_offset_counter);

  return status;
}

/**
  * @brief This function calculate the current address of the valid data.
  *
  * @param var_offset      offset position of the variable
  * @param data_type       type of variable @ref ee_data_type_t
  * @param current_address physical address of the valid data
  *
  * @return ee_status    EE_OK     return valid Address/OffsetCounter
  *                      EE_ERROR_ALGO  in case of non valid information on the variable
  */
static ee_status ee_GetCurrentAddress(uint32_t var_offset, ee_data_type_t data_type, uint32_t *current_address)
{
  ee_status status = EE_OK;
  uint8_t counter_offset;
  uint8_t counter_invaliddata = 0U;
  uint8_t min = 0xFFU;
  uint8_t max = 0xFFU;
  uint8_t position_min = 0;
  uint8_t position_max = 0;
  uint64_t Data;

  /* read the data to get the offset value */
  for (uint8_t readindex = 0; readindex < EE_CYCLES_NUMBER; readindex++)
  {
    if (ee_GetData(var_offset + ((uint32_t)readindex * EE_NBPAGE_TOSTOREDATA * EE_SECTOR_SIZE) + EE_START_PAGE_ADDRESS,
                   &Data, &counter_offset, data_type) != EE_OK)
    {
      /* count the invalid data */
      counter_invaliddata++;
    }
    else
    {
      /* Calculation of the maximum offset value */
      if ((counter_offset > (EE_CYCLES_NUMBER)) && ((max == 0xFFU) || (max < counter_offset)))
      {
        max = counter_offset;
        position_max = readindex;
      }

      /* Calculation of the maximum offset value */
      if ((counter_offset < (EE_CYCLES_NUMBER + 1U)) && ((min == 0xFFU) || (min < counter_offset)))
      {
        min = counter_offset;
        position_min = readindex;
      }
    }
  }

  switch (counter_invaliddata)
  {
    /* All values are invalid, the first valid offset is the first */
    case EE_CYCLES_NUMBER:
    {
      *current_address = 0U;
      status = EE_INFO_NODATA;
      break;
    }

    /* the system doesn't care there is corrupted data
       only the latest valid data is taken into account
       thanks to min/max and the corrupted data are rewritten
       by the algorithm */
    default:
    {
      if ((max == 0xFFU)
          || ((min != 0xFFU) && (max == (EE_OFFSET_MODULO - 1U))))
      {
        /* only min is considered */
        *current_address = position_min;
      }
      else
      {
        /* only max is considered */
        *current_address = position_max;
      }
      break;
    }
  }

  /* Calculate the data Address */
  *current_address = var_offset + ((*current_address) * EE_NBPAGE_TOSTOREDATA * EE_SECTOR_SIZE) + EE_START_PAGE_ADDRESS;

  EE_DEBUG_PRINT("ee_GetCurrentAddress::Address 0x%x\n", *current_address);
  return status;
}

/**
  * @brief This function read the data and check the validity.
  *
  * @param var_address variable address
  * @param pdata      pointer on the data
  * @param p_offset_counter pointer to get the offset counter
  * @param data_type type of the data to read /ref ee_data_type_t
  * @return ee_status
  */
static ee_status ee_GetData(uint32_t var_address, void *pdata, uint8_t *p_offset_counter, ee_data_type_t data_type)
{
  ee_status status = EE_INFO_NODATA;

  *p_offset_counter = 0xFFU; /* means data is invalid */

  switch (data_type)
  {
    case EE_DATA_8BITS:
    {
      ee_data_frame_8bits data;
      uint32_t ecc_output = 0U;

      /* read data */
      if (EE_ITF_FLASH_ReadData(var_address, (uint8_t *)&data.Data32bits, (uint16_t)sizeof(data)) != EE_ITF_FLASH_OK)
      {
        return EE_ERROR_ALGO;
      }

      ((ee_data_frame_8bits *)pdata)->Data32bits = data.Data32bits;

      if (data.Data32bits == 0xFFFFFFFFU)
      {
        /* This value indicates a read from an erased area that never corresponds to an application value */
        return EE_INFO_NODATA;
      }
      else
      {
        /* decode BCH */
        if (EE_ITF_ECC_Control20BIT(data.Data32bits, &ecc_output) != EE_ITF_ECC_ERROR)
        {
          /* Update the value with corrected value by ecc interface */
          data.Data32bits = ecc_output;

          uint8_t CRCSave = (uint8_t)data.EE_FrameStruct_8BITS.Crc;
          uint8_t CRCValue = 0;

          uint32_t CRCInput = data.EE_CRCCalcul_8BITS.CRCInput;

          /* Calculate the CRC */
          (void)EE_ITF_CRC_Calcul8Bit((uint8_t *)&CRCInput, 2, &CRCValue);

          if (CRCValue == CRCSave)
          {
            *p_offset_counter = (uint8_t)data.EE_FrameStruct_8BITS.OffsetCounter;
            /* get data */
            ((ee_data_frame_8bits *)pdata)->Data32bits = data.Data32bits;
            status = EE_OK;
          }
          else
          {
            status = EE_ERROR_ITF_CRC;
            EE_DEBUG_PRINT("ee_GetData::CRC FAIL\n");
          }
        }
        else
        {
          status = EE_ERROR_ITF_ECC;
          EE_DEBUG_PRINT("ee_GetData::ECC FAIL\n");
        }
      }

      break;
    }
    case EE_DATA_32BITS:
    {
      ee_data_frame_32bits data;
      uint64_t ecc_output = 0;
      /* read data */
      if (EE_ITF_FLASH_ReadData(var_address, (uint8_t *)&data.Data64bits, (uint16_t)sizeof(data)) != EE_ITF_FLASH_OK)
      {
        return EE_ERROR_ALGO;
      }

      if (data.Data64bits == 0xFFFFFFFFFFFFFFFFU)
      {
        /* This value indicates a read from an erased area that never corresponds to an application value */
        return EE_INFO_NODATA;
      }
      else
      {
        /* ECC control data */
        if (EE_ITF_ECC_Control44BIT(data.Data64bits, &ecc_output) != EE_ITF_ECC_ERROR)
        {
          /* Update the value with corrected value by ecc interface */
          data.Data64bits = ecc_output;
          uint8_t CRCSave = (uint8_t)data.EE_FrameStruct_32BITS.Crc;
          uint8_t CRCValue = 0;

          uint64_t CRCInput = data.EE_CRCCalcul_32BITS.CRCInput;

          /* Calculate the CRC */
          (void)EE_ITF_CRC_Calcul8Bit((uint8_t *)&CRCInput, 5, &CRCValue);

          if (CRCValue == CRCSave)
          {
            *p_offset_counter = (uint8_t)data.EE_FrameStruct_32BITS.OffsetCounter;
            /* get data */
            ((ee_data_frame_32bits *)pdata)->Data64bits = data.Data64bits;
            status = EE_OK;
          }
          else
          {
            status = EE_ERROR_ITF_CRC;
            EE_DEBUG_PRINT("ee_GetData::CRC FAIL\n");
          }
        }
        else
        {
          status = EE_ERROR_ITF_ECC;
          EE_DEBUG_PRINT("ee_GetData::ECC FAIL\n");
        }
      }
      break;
    }
    default:
      status = EE_INVALID_PARAM;
      break;
  }
  return status;
}

/**
  * @}
  */

/**
  * @}
  */
