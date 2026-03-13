/**
  ******************************************************************************
  * @file    eeprom_emulation/interface_ecc/eeprom_itfecc_bch.c
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

/** @defgroup EEPROM_EMULATION_INTERFACE_ECC_BCH Interface ECC BCH
  * @{
  */

/* Private variables ---------------------------------------------------------*/
/** @defgroup EEPROM_EMULATION_INTERFACE_ECC_BCH_PRIVATE_VARIABLES  Interface ECC BCH Private Variables
  * @{
  */

#if defined(CODE_DEBUG)
/**
  * @brief Variable to store the number of errors detected in the ECC BCH algorithm.
  * This variable is used to store the number of errors detected during the control process in the BCH ECC algorithm.
  */
static uint8_t Detection3errors;

#endif /* CODE_DEBUG */
/**
  * @}
  */

/* Exported functions ---------------------------------------------------------*/
/**
  * @addtogroup EEPROM_EMULATION_INTERFACE_ECC_FUNCTIONS
  * @{
  */

/** @defgroup EEPROM_EMULATION_INTERFACE_ECC_BCH_APIS Interface ECC BCH APIs
  * @{
  */

/**
  * @brief  Initialize the ECC interface context.
  * @param  ecc_object ECC instance used by the interface.
  * @retval Possible return values: see @ref ee_itf_ecc_status.
  */
ee_itf_ecc_status EE_ITF_ECC_Init(void *ecc_object)
{
  /*Prevent unused parameter */
  (void)ecc_object;
  return EE_ITF_ECC_OK;
}

/**
  * @brief  Check and correct a 20-bit protected value.
  * @param  data Input value to check.
  * @param  data_output Output pointer that receives the corrected value.
  * @retval Possible return values: see @ref ee_itf_ecc_status.
  */
ee_itf_ecc_status EE_ITF_ECC_Control20BIT(uint32_t data, uint32_t *data_output)
{
  uint8_t i;
  uint8_t j;
  uint8_t error_det;
  uint8_t elp[3];
  uint8_t s3;
  uint8_t count;
  uint8_t loc[3] = {0};
  uint8_t reg[3] = {0};
  uint8_t aux;
  uint8_t NewParityBit;
  uint8_t syn[4];
  uint32_t synd = 0U;
  uint32_t mask = 1U;

  /************                 BCH code variables                ****************

  for Data = 8 bits /  BCH (30,20)
  20 inputs bits ( 8bits data + CRC 8 bits + Counter size 4 bits )    ****/

  /**
    * Table mapping integers to corresponding encoded values for Data20b encoding.
    * This table maps integers to their corresponding encoded values for Data20b encoding.
    */
  const uint8_t alpha_to_Data20b[32] =
  {
    1, 2, 4, 8, 16, 5, 10, 20, 13, 26, 17, 7, 14, 28, 29, 31, 27, 19, 3, 6, 12, 24, 21, 15, 30, 25, 23, 11, 22, 9, 18, 0
  };

  /**
    * Array storing the index of Data20b elements
    * This array contains the index values corresponding to elements in Data20b.
    */
  const uint8_t index_of_Data20b[32] =
  {
    0xFF, 0, 1, 18, 2, 5, 19, 11, 3, 29, 6, 27, 20, 8, 12, 23, 4, 10, 30, 17, 7, 22, 28, 26, 21, 25, 9, 16, 13,
    14, 24, 15
  };

  /**
    * Array storing mask values for decoding bits 31 to 20
    * This array contains mask values used for decoding bits 31 to 20, including parity bits and specific masks
    * for different sections.
    */
  const uint32_t maskDecode31_20[] =
  {
    0x5D8F9A42, 0x3B1F3484, 0x2BB1F348, 0x5763E690, 0x2EC7CD20, 0x00000000, /*!< syn[1] */
    0x6A3BE4C2, 0x185A8EF8, 0x351DF260, 0x4C2D477C, 0x6F930B50, 0x00000000, /*!< syn[3] */
    0x7FFFFFFF,                                                             /*!< paritybit */
    0x73485762, 0x5763E690, 0x15D8F9A4, 0x79A42BB0, 0x2BB1F348, 0x00000000  /*!< syn[2] */
  };
#if defined(CODE_DEBUG)
  Detection3errors = 0;
#endif /* CODE_DEBUG */

  *data_output = data;
  for (i = 0; i < 13U; i++)
  {
    uint32_t val = (data & maskDecode31_20[i]);
    val ^= (val >> 1U);
    val ^= (val >> 2U);
    val ^= (val >> 4);
    val ^= (val >> 8U);
    val ^= (val >> 16U);
    synd |= ((val & 1U) == 1U) ? (mask << i) : 0U;
  }
  /* Calcul parity bit on all data bits, parity bit included */
  /* Parity bit = 0: no error or 2 errors found, = 1: 1 or 3 errors found */
  NewParityBit = (uint8_t)((synd >> 12U) & 0x1U);
  syn[1] = index_of_Data20b[synd & 0x1FU];
  syn[3] = index_of_Data20b[(synd >> 6U) & 0x1FU];

  /* No error found, data is valid */
  if (synd == 0U)
  {
    return EE_ITF_ECC_OK;
  }
  /* Error only on parity bit data[0] */
  else if (synd == 0x1000U)
  {
    /* Correct parity bit */
    *data_output ^= mask;
    return EE_ITF_ECC_DATA_UPDATED;
  }
  else if (syn[1] != 0xFFU)
  {
    s3 = (syn[1] * 3U) % 31U;
    /* 1 error found */
    if (syn[3] == s3)
    {
      /* Correction of 1 error */
      *data_output ^= (mask << (syn[1] + 1U));
      if (NewParityBit == 0U)
      {
        /* NewParityBit must be equal to 1 if there is 1 error-->There is 1 error on the parity bit that we correct */
        *data_output ^= mask;
      }
      return EE_ITF_ECC_DATA_UPDATED;
    }
    /* more than one error found */
    else
    {
      for (i = 13U; i < 19U; i++)
      {
        uint32_t val = data & maskDecode31_20[i];
        val ^= (val >> 1U);
        val ^= (val >> 2U);
        val ^= (val >> 4U);
        val ^= (val >> 8U);
        val ^= (val >> 16U);
        synd |= ((val & 1U) == 1U) ? (1UL << i) : 0UL;
      }
      syn[2] = index_of_Data20b[(synd >> 13U) & 0x1FU];

      if (syn[3] != 0xFFU)
      {
        aux = (alpha_to_Data20b[s3] ^ alpha_to_Data20b[syn[3]]);
      }
      else
      {
        aux = alpha_to_Data20b[s3];
      }

      /* Error location */
      elp[0] = 0U;
      elp[1] = (syn[2] - index_of_Data20b[aux] + 31U) % 31U;
      elp[2] = (syn[1] - index_of_Data20b[aux] + 31U) % 31U;

      for (i = 1U; i <= 2U; i++)
      {
        reg[i] = elp[i];
      }
      /* Initialize number of errors */
      count = 0U;
      /* Perform a Chien search */
      for (i = 1U; i <= 31U; i++)
      {
        error_det = 1U;
        for (j = 1U; j <= 2U; j++)
        {
          if (reg[j] != 0xFFU)
          {
            reg[j] = (reg[j] + j) % 31U;
            error_det ^= alpha_to_Data20b[reg[j]];
          }
        }
        /* Store error location number index */
        if (error_det == 0U)
        {
          loc[count] = i % 31U;
          count++;
        }
      }

      if (count == 2U)
      {
        /* 3 Errors found , can't be corrected */
        if (NewParityBit == 1U)
        {
#if defined(CODE_DEBUG)
          Detection3errors = 1;
#endif /* CODE_DEBUG */
          return EE_ITF_ECC_ERROR;
        }
        /* 2 errors found */
        *data_output ^= (mask << (loc[0] + 1U));
        *data_output ^= (mask << (loc[1] + 1U));
        return EE_ITF_ECC_DATA_UPDATED;
      }
    }
    /* 3 Errors found , can't be corrected */
    if (NewParityBit == 1U)
    {
#if defined(CODE_DEBUG)
      Detection3errors = 1U;
#endif /* CODE_DEBUG */
      return EE_ITF_ECC_ERROR;
    }
  }
  return EE_ITF_ECC_ERROR;
}

/**
  * @brief  Check and correct a 44-bit protected value.
  * @param  data Input value to check.
  * @param  data_output Output pointer that receives the corrected value.
  * @retval Possible return values: see @ref ee_itf_ecc_status.
  */
ee_itf_ecc_status EE_ITF_ECC_Control44BIT(uint64_t data, uint64_t *data_output)
{
  uint8_t i;
  uint8_t j;
  uint8_t error_det;
  uint8_t elp[3];
  uint8_t s3;
  uint8_t count;
  uint8_t loc[3] = {0};
  uint8_t reg[3] = {0};
  uint8_t aux;
  uint8_t NewParityBit;
  uint64_t mask = 1ULL;
  uint8_t syn[4];
  uint32_t synd = 0U;

  /**       for Data = 16 bits : BCH (40,28)  : 28 inputs bits
  ( 16bits data + CRC 8 bits + Counter size 4 bits )

  or Data = 32 bits : BCH (56,44) :  44 inputs bits
  ( 32bits data + CRC 8 bits + Counter size 4 bits )    */

  /**
    * Array representing alpha to Data28b mapping
    * This array maps alpha values to Data28b elements.
    */
  const uint8_t alpha_to_Data28b[64] =
  {
    1, 2, 4, 8, 16, 32, 3, 6, 12, 24, 48, 35, 5, 10, 20, 40, 19, 38, 15, 30, 60, 59, 53, 41, 17, 34, 7, 14, 28, 56, 51,
    37, 9, 18, 36, 11, 22, 44, 27, 54, 47, 29, 58, 55, 45, 25, 50, 39, 13, 26, 52, 43, 21, 42, 23, 46, 31, 62, 63, 61,
    57, 49, 33, 0
  };

  /**
    * Array storing the index of Data28b elements
    * This array contains the index values corresponding to elements in Data28b.
    */
  const uint8_t index_of_Data28b[64] =
  {
    0xFFU, 0, 1, 6, 2, 12, 7, 26, 3, 32, 13, 35, 8, 48, 27, 18, 4, 24, 33, 16, 14, 52, 36, 54, 9, 45, 49, 38, 28,
    41, 19, 56, 5, 62, 25, 11, 34, 31, 17, 47, 15, 23, 53, 51, 37, 44, 55, 40, 10, 61, 46, 30, 50, 22, 39, 43, 29, 60,
    42, 21, 20, 59, 57, 58
  };

  /**
    * Array storing mask values for decoding bits 57 to 44
    * This array contains mask values used for decoding bits 57 to 44, including 12 bits for ECC and 1 parity bit.
    * ECC 12 bits + 1 parity bit
    */
  const uint64_t maskDecode57_44[] =
  {
    0x00B376938BCA3082, 0x01D59BB49C5E5184, 0x01AB376938BCA308, 0x01566ED271794610, 0x00ACDDA4E2F28C20,
    0x0159BB49C5E51840, /*!< syn[1] */
    0x002EADE1756F0BAA, 0x01F6239FB11CFD88, 0x0192834C941A64A0, 0x001756F0BAB785D4, 0x01EC473F6239FB10,
    0x012506992834C940, /*!< syn[3] */
    0x01FFFFFFFFFFFFFF, /*!< parity bit */
    0x01E51841FAB37692, 0x01566ED271794610, 0x017946107EACDDA4, 0x00ACDDA4E2F28C20, 0x00F28C20FD59BB48,
    0x0159BB49C5E51840, /*!< syn[2] */
  };
#if defined(CODE_DEBUG)
  Detection3errors = 0;
#endif /* CODE_DEBUG */

  *data_output = data;
  for (i = 0U; i < 13U; i++)
  {
    uint64_t val = data & maskDecode57_44[i];
    val ^= (val >> 1U);
    val ^= (val >> 2U);
    val ^= (val >> 4U);
    val ^= (val >> 8U);
    val ^= (val >> 16U);
    val ^= (val >> 32U);
    synd |= ((val & 1U) == 1U) ? (uint32_t)(mask << i) : 0U;
  }
  /* Calcul parity by on all data bits, parity bit included */
  /* Parity bit = 0: no error or 2 errors found, = 1: 1 or 3 errors found */
  NewParityBit = (uint8_t)((synd >> 12U) & 0x1U);
  syn[1] = index_of_Data28b[synd & 0x3FU];
  syn[3] = index_of_Data28b[(synd >> 6U) & 0x3FU];

  /* No error found, data is valid */
  if (synd == 0U)
  {
    return EE_ITF_ECC_OK;
  }
  /* Error only on parity bit data[0]*/
  else if (synd == 0x1000U)
  {
    /* Correct parity bit */
    *data_output ^= mask;
    return EE_ITF_ECC_DATA_UPDATED;
  }
  else if (syn[1] != 0xFFU)
  {
    s3 = (syn[1] * 3U) % 63U;
    /* 1 error found */
    if (syn[3] == s3)
    {
      /* Correction of 1 error */
      *data_output ^= (mask << (syn[1] + 1U));
      if (NewParityBit == 0U)
      {
        /* NewParityBit must be equal to 1 if there is 1 error-->There is 1 error on the parity bit that we correct */
        *data_output ^= mask;
      }
      return EE_ITF_ECC_DATA_UPDATED;
    }
    /* more than one error found */
    else
    {
      for (i = 13U; i < 19U; i++)
      {
        uint64_t val = data & maskDecode57_44[i];
        val ^= (val >> 1U);
        val ^= (val >> 2U);
        val ^= (val >> 4U);
        val ^= (val >> 8U);
        val ^= (val >> 16U);
        val ^= (val >> 32U);
        synd |= ((val & 1UL) == 1UL) ? (uint32_t)(mask << i) : 0UL;
      }
      syn[2] = index_of_Data28b[(synd >> 13U) & 0x3FU];

      if (syn[3] != 0xFFU)
      {
        aux = alpha_to_Data28b[s3] ^ alpha_to_Data28b[syn[3]];
      }
      else
      {
        aux = alpha_to_Data28b[s3];
      }
      /* Error location */
      elp[0] = 0U;
      elp[1] = ((syn[2] - index_of_Data28b[aux] + 63U) % 63U);
      elp[2] = ((syn[1] - index_of_Data28b[aux] + 63U) % 63U);

      for (i = 1U; i <= 2U; i++)
      {
        reg[i] = elp[i];
      }
      /* Initialize number of errors */
      count = 0U;
      /* Perform a Chien search */
      for (i = 1U; i <= 63U; i++)
      {
        error_det = 1U;
        for (j = 1U; j <= 2U; j++)
        {
          if (reg[j] != 0xFFU)
          {
            reg[j] = (reg[j] + j) % 63U;
            error_det ^= alpha_to_Data28b[reg[j]];
          }
        }
        /* Store error location number index */
        if (error_det == 0U)
        {

          loc[count] = i % 63U;
          count++;
        }
      }
      if (count == 2U)
      {
        /* 3 Errors found , can't be corrected */
        if (NewParityBit == 1U)
        {
#if defined(CODE_DEBUG)
          Detection3errors = 1U;
#endif /* CODE_DEBUG */
          return EE_ITF_ECC_ERROR;
        }
        /* 2 errors found */
        *data_output ^= (mask << (loc[0] + 1U));
        *data_output ^= (mask << (loc[1] + 1U));
        return EE_ITF_ECC_DATA_UPDATED;
      }
    }
    /* 3 Errors found , can't be corrected */
    if (NewParityBit == 1U)
    {
#if defined(CODE_DEBUG)
      Detection3errors = 1U;
#endif /* CODE_DEBUG */
      return EE_ITF_ECC_ERROR;
    }
  }
  return EE_ITF_ECC_ERROR;
}

/**
  * @brief  Compute ECC for a 20-bit protected value.
  * @param  data Data value to encode.
  * @param  ecc Output pointer that receives the ECC value.
  * @retval Possible return values: see @ref ee_itf_ecc_status.
  */
ee_itf_ecc_status EE_ITF_ECC_Calculation20BIT(uint32_t data, uint16_t *ecc)
{
  uint32_t mask = 1UL;
  *ecc = 0U;

  /**
    * Array storing mask values for encoding bits 31 to 20
    * This array contains mask values used for encoding bits 31 to 20, including 10 bits for ECC and 1 parity bit.
    * 10 bits ECC+ 1 parity bit
    */
  const uint32_t maskEncode31_20[] =
  {
    0x527AB, 0xA4F56, 0x49EAC, 0xC1AF3, 0x835E6, 0x54C67, 0xFBF65, 0xF7ECA, 0xBDA3F, 0x293D5, 0xCE299
  };

  for (uint8_t i = 0U; i < (sizeof(maskEncode31_20) / sizeof(maskEncode31_20[0])); i++)
  {
    uint32_t val = data & maskEncode31_20[i];
    val ^= (val >> 1U);
    val ^= (val >> 2U);
    val ^= (val >> 4U);
    val ^= (val >> 8U);
    val ^= (val >> 16U);
    *ecc |= ((val & 1U) == 1U) ? (uint16_t)(mask << i) : 0U;
  }

  *ecc = ((*ecc & 0x3FFU) << 1U) | ((*ecc & 0x400U) >> 10U);
  return EE_ITF_ECC_OK;
}

/**
  * @brief  Compute ECC for a 44-bit protected value.
  * @param  data Data value to encode.
  * @param  ecc Output pointer that receives the ECC value.
  * @retval Possible return values: see @ref ee_itf_ecc_status.
  */
ee_itf_ecc_status EE_ITF_ECC_Calculation44BIT(uint64_t data, uint16_t *ecc)
{
  uint32_t mask = 1UL;
  *ecc = 0U;

  /**
    * Array storing mask values for encoding bits 57 to 44
    * This array contains mask values used for encoding bits 57 to 44, including 12 bits for ECC and 1 parity bit.
    * ECC 12 bits + 1 parity bit
    */
  const uint64_t maskEncode57_44[] =
  {
    0x68C6FC93AC5, 0xD18DF92758A, 0xA31BF24EB14, 0x2EF1180ECED, 0x3524CC8E31F, 0x028F658FCFB,
    0x051ECB1F9F6, 0x0A3D963F3EC, 0x7CBDD0EDD1D, 0xF97BA1DBA3A, 0x9A31BF24EB1, 0x34637E49D62, 0xD8425471643
  };
  for (uint8_t i = 0U; i < (sizeof(maskEncode57_44) / sizeof(maskEncode57_44[0])); i++)
  {
    uint64_t val = data & maskEncode57_44[i];
    val ^= (val >> 1U);
    val ^= (val >> 2U);
    val ^= (val >> 4U);
    val ^= (val >> 8U);
    val ^= (val >> 16U);
    val ^= (val >> 32U);
    *ecc |= ((val & 1U) == 1U) ? (uint16_t)(mask << i) : 0U;
  }

  *ecc = (((*ecc & 0xFFFU) << 1U) | ((*ecc & 0x1000U) >> 12U));
  return EE_ITF_ECC_OK;
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
