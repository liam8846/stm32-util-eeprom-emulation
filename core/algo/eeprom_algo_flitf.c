/**
  ******************************************************************************
  * @file    eeprom_emulation/core/algo/eeprom_algo_flitf.c
  * @brief   This file provides all the FLITF algo firmware functions.
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
           (++) Callback function called when the clean up operation in interrupt
                mode, is finished: EE_EndOfCleanup_UserCallback()

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
#include "eeprom_algo_flitf.h"
#include "eeprom_itf_flash.h"
#include "eeprom_itf_crc.h"

/** @addtogroup EEPROM_EMULATION_ALGO_FLITF
  * @{
  */


/* Private define -----------------------------------------------------------*/
/** @defgroup EEPROM_EMULATION_ALGO_FLITF_PRIVATE_DEFINES Algo FLITF Private Defines
  * @{
  */

/**
  * @brief  this macro use to disable the debug information.
  * @note   to get debug information, the implementation of the macro must be done inside the configuration
  */
#if !defined(EE_DEBUG_PRINT)
#define EE_DEBUG_PRINT(...)
#endif /* !defined(EE_DEBUG_PRINT) */

/**
  * @brief  this macro is used to indicate no page found.
  */
#define EE_NO_PAGE_FOUND        ((uint32_t)0xFFFFFFFFU)

/**
  * @brief Calculate the page address based on the page index.
  * This function calculates the page address using the provided page index.
  * @param page page index for which the page address is calculated.
  * @return uint32_t page address corresponding to the page index.
  */
static inline uint32_t GetPageAddress(uint32_t page)
{
  /*!< Get page address from page index */
  return (uint32_t)(EE_FLASH_BASE_ADDRESS  + (page * EE_FLASH_PAGE_SIZE));
}
/**
  * @brief Calculate the page number for a given memory address.
  * @param address Flash address for which the page number is calculated.
  * @return uint32_t page number corresponding to the flash address.
  */
static inline uint32_t GetPageNumber(uint32_t address)
{
  /*!< Get page index from page address */
  return (uint32_t)(((((address) - EE_FLASH_BASE_ADDRESS)))  / EE_FLASH_PAGE_SIZE);
}


/**
  * @brief  this macro is used to compare frame with a value.
  */
#if defined(EE_FRAME_LINE_SIZE)  &&  (EE_FRAME_LINE_SIZE == 16)
#define COMP_DATA_FRAME(_FRAME_, _FRAME_SYMBOL_)                 \
  (((_FRAME_).data_64_bits_t.data64[0] == (_FRAME_SYMBOL_))      \
   && ((_FRAME_).data_64_bits_t.data64[1] == (_FRAME_SYMBOL_)))
#else
#define COMP_DATA_FRAME(_FRAME_, _FRAME_SYMBOL_)                 \
  ((_FRAME_).data_64_bits_t.data64[0] == (_FRAME_SYMBOL_))
#endif /* EE_FRAME_LINE_SIZE */

/**
  * @brief  this macro is used set frame to a value.
  */
#if defined(EE_FRAME_LINE_SIZE) &&  (EE_FRAME_LINE_SIZE == 16)
#define SET_DATA_FRAME(_FRAME_, _FRAME_SYMBOL_)                 \
  do {                                                          \
    (_FRAME_).data_64_bits_t.data64[0] = (_FRAME_SYMBOL_);      \
    (_FRAME_).data_64_bits_t.data64[1] = (_FRAME_SYMBOL_);      \
  } while(0);
#else
#define SET_DATA_FRAME(_FRAME_, _FRAME_SYMBOL_)                 \
  do {                                                          \
    _FRAME_.data_64_bits_t.data64[0] = _FRAME_SYMBOL_;          \
  } while(0);
#endif /* EE_FRAME_LINE_SIZE */

/**
  * @}
  */

/* Private typedef -----------------------------------------------------------*/
/** @defgroup EEPROM_EMULATION_ALGO_FLITF_PRIVATE_TYPEDEF Algo FLITF Private Typedef
  * @{
  */
#if defined(EE_FRAME_LINE_SIZE) && (EE_FRAME_LINE_SIZE == 16)

/**
  * @brief  Element size : 128 bits = 2 * 64 bits.
  */
#define EE_ELEMENT_SIZE    16U

/**
  * @brief  Frame data size corresponding to data size max of 96 bit with 128 bit flash granularity.
  */
#define EE_FRAME_DATA_SIZE 12U

/**
  * @union ee_frame_t
  * A union representing different data formats for EEPROM frames.
  *
  * This union allows access to EEPROM frames in various data formats, including
  * 16-bit, 64-bit, 32-bit, and 8-bit structures. It is useful for handling
  * EEPROM data in different ways depending on the application requirements.
  */
typedef union
{
  /**
    * @struct frame_s
    * Structure representing a standard EEPROM frame.
    *
    * This structure contains a CRC, a virtual address, and data of size
    * EE_FRAME_DATA_SIZE.
    */
  struct frame_s
  {
    uint16_t crc; /*!< Cyclic Redundancy Check for error detection. */
    uint16_t virtual_address; /*!< Virtual address in EEPROM. */
    uint8_t data[EE_FRAME_DATA_SIZE]; /*!< Data stored in the EEPROM frame. */
  } frame_t; /*!< Instance of the standard EEPROM frame structure. */

  /**
    * @struct data_64_bits_s
    * Structure representing data in 64-bit format.
    *
    * This structure allows access to EEPROM data as two 64-bit values.
    */
  struct data_64_bits_s
  {
    uint64_t data64[2]; /*!< Array of two 64-bit data values. */
  } data_64_bits_t; /*!< Instance of the 64-bit data structure. */

  /**
    * @struct data_32_bits_s
    * Structure representing data in 32-bit format.
    *
    * This structure allows access to EEPROM data as four 32-bit values.
    */
  struct data_32_bits_s
  {
    uint32_t data32[2 * 2]; /*!< Array of four 32-bit data values. */
  } data_32_bits_t; /*!< Instance of the 32-bit data structure. */

  /**
    * @struct data_8_bits_s
    * Structure representing data in 8-bit format.
    *
    * This structure allows access to EEPROM data as sixteen 8-bit values.
    */
  struct data_8_bits_s
  {
    uint8_t data8[2 * 8]; /*!< Array of sixteen 8-bit data values. */
  } data_8_bits_t; /*!< Instance of the 8-bit data structure. */
} ee_frame_t;

#elif defined(EE_FRAME_LINE_SIZE) && (EE_FRAME_LINE_SIZE == 8)

/**
  * @brief  Element size : 64 bits.
  */
#define EE_ELEMENT_SIZE    8U

/**
  * @brief  Frame data size corresponding to data size max of 32 bit with 64 bit flash granularity.
  */
#define EE_FRAME_DATA_SIZE 4U

/**
  * @union ee_frame_t
  * A union representing different data formats for EEPROM frames.
  *
  * This union allows access to EEPROM frames in various data formats, including
  * 16-bit, 64-bit, 32-bit, and 8-bit structures. It is useful for handling
  * EEPROM data in different ways depending on the application requirements.
  */
typedef union
{
  /**
    * Structure representing a standard EEPROM frame.
    *
    * This structure contains a CRC, a virtual address, and data of size
    * EE_FRAME_DATA_SIZE.
    */
  struct
  {
    uint16_t crc; /*!< Cyclic Redundancy Check for error detection. */
    uint16_t virtual_address; /*!< Virtual address in EEPROM. */
    uint8_t data[EE_FRAME_DATA_SIZE]; /*!< Data stored in the EEPROM frame. */
  } frame_t; /*!< Instance of the standard EEPROM frame structure. */

  /**
    * Structure representing data in 64-bit format.
    *
    * This structure allows access to EEPROM data as one 64-bit value.
    */
  struct
  {
    uint64_t data64[1]; /*!< Array of one 64-bit data value. */
  } data_64_bits_t; /*!< Instance of the 64-bit data structure. */

  /**
    * Structure representing data in 32-bit format.
    *
    * This structure allows access to EEPROM data as two 32-bit values.
    */
  struct
  {
    uint32_t data32[1 * 2]; /*!< Array of two 32-bit data values. */
  } data_32_bits_t; /*!< Instance of the 32-bit data structure. */

  /**
    * Structure representing data in 8-bit format.
    *
    * This structure allows access to EEPROM data as eight 8-bit values.
    */
  struct
  {
    uint8_t data[1 * 8]; /*!< Array of eight 8-bit data values. */
  } data_8_bits_t; /*!< Instance of the 8-bit data structure. */
} ee_frame_t;
#else
#error "error wrong definition of the frame line size"
#endif /* EE_FRAME_LINE_SIZE */

/**
  * @brief  page header size is 4 elements to save page state.
  */
#define EE_PAGE_HEADER_SIZE     (EE_ELEMENT_SIZE * 4U)

/**
  * @brief  pages number.
  */
#define EE_PAGES_NUMBER   (((((EE_NB_OF_VARIABLES / EE_NB_MAX_ELEMENTS_BY_PAGE \
                              )+1U) * 2U) * EE_CYCLES_NUMBER) + EE_GUARD_PAGES_NUMBER)

/**
  * @brief  Max number of elements by page.
  */
#define EE_NB_MAX_ELEMENTS_BY_PAGE ((EE_FLASH_PAGE_SIZE - EE_PAGE_HEADER_SIZE) / EE_ELEMENT_SIZE)

/**
  * @brief  Max number of elements written before triggering pages transfer.
  */
#define EE_NB_MAX_WRITTEN_ELEMENTS (EE_NB_MAX_ELEMENTS_BY_PAGE * (1U + (EE_PAGES_NUMBER / 2U)))

/**
  * @brief  Page index of the 1st page used for EEPROM emul, in the bank.
  */
#define EE_START_PAGE            GetPageNumber(EE_START_PAGE_ADDRESS)

/**
  * @brief  End address used for EEPROM Emul.
  */
#define EE_END_ADDRESS           (EE_START_PAGE_ADDRESS + (EE_PAGES_NUMBER * EE_FLASH_PAGE_SIZE))


/**
  * @brief  Page state header : ERASED.
  */
#define EE_PAGESTAT_ERASED      (uint64_t)0xFFFFFFFFFFFFFFFFU

/**
  * @brief  Page state header tag.
 */
#define EE_PAGESTAT_TAG         (uint64_t)0xAAAAAAAAAAAAAAAAU

/**
  * @brief  Page state header : ACTIVE.
 */
#define EE_PAGESTAT_ACTIVE                    EE_PAGESTAT_TAG  /*!< State saved in 1st data type of page header */

/**
  * @brief  Page state header : VALID.
 */
#define EE_PAGESTAT_VALID                     EE_PAGESTAT_TAG  /*!< State saved in 2nd data type of page header */

/**
  * @brief  Page state header : REMOVED.
 */
#define EE_PAGESTAT_REMOVED                   EE_PAGESTAT_TAG  /*!< State saved in 3rd data type of page header */

/**
  * @brief  Page state header : ERASING.
 */
#define EE_PAGESTAT_ERASING                   EE_PAGESTAT_TAG  /*!< State saved in 4th data type of page header */

/**
  * @brief  EE internal page state type enumeration definition.
  */
typedef enum
{
  PAGE_STATE_INVALID,   /*!< page in invalid state                                                     */
  PAGE_STATE_ERASED,    /*!< page is erased                                                            */
  PAGE_STATE_REMOVED,   /*!< page used during the remove of the page the system                        */
  PAGE_STATE_ACTIVE,    /*!< page contains valid data and is not full                                  */
  PAGE_STATE_VALID,     /*!< page contains valid data and is full                                      */
  PAGE_STATE_ERASING    /*!< page used during transfer, must be erased after transfer                  */
} ee_page_state_t;

#if defined(EE_DEBUG)
/**
  * @brief  state string used for debug purpose.
  */
const uint8_t ee_state_type_string[][19] =
{
  "PAGE_STATE_INVALID",
  "PAGE_STATE_ERASED",
  "PAGE_STATE_REMOVED",
  "PAGE_STATE_ACTIVE",
  "PAGE_STATE_VALID",
  "PAGE_STATE_ERASING"
};
#endif /* EE_DEBUG */

/**
  * @brief  EE State Reliability enumeration definition.
  *
  */
typedef enum
{
  STATE_RELIABLE,   /*!< Header of the page is not corrupted, state is reliable. */
  STATE_CORRUPTED   /*!< Header of the page is corrupted, state is corrupted.    */
} ee_state_reliability_t;
/**
  * @}
  */

/** @defgroup EEPROM_EMULATION_ALGO_FLITF_PAGE_MACROS Algo FLITF Page Macros
  * @{
  */

/**
  * @brief Get page index of previous page, among circular page list
  */
#define PREVIOUS_PAGE(__PAGE__)  ((uint32_t)((((__PAGE__) - EE_START_PAGE - 1U + EE_PAGES_NUMBER) \
                                              % EE_PAGES_NUMBER) + EE_START_PAGE))

/**
  * @brief Get page index of following page, among circular page list.
  */
#define FOLLOWING_PAGE(__PAGE__) ((uint32_t)((((__PAGE__) - EE_START_PAGE + 1U) % EE_PAGES_NUMBER) + EE_START_PAGE))

/**
  * @}
  */

/* Private variables ---------------------------------------------------------*/
/** @defgroup EEPROM_EMULATION_PRIVATE_ALGO_FLITF_VARIABLES Algo FLITF Private Variables
  * @{
  */

/* Global variables used to store eeprom status */

/**
  * @brief internal state structure of the FLITF algo.
  *
  */
typedef struct
{

  __IO ee_operation_status ope_status;   /*!< operation status of cleanup interrupt operation */
  uint16_t       erase_it_ope;           /*!< flag to indicate if an operation is ongoing */
  uint16_t erase_it_page_counter;        /*!< counter value used when operation is OK to update page status */
  uint32_t erase_it_first_page;          /*!< first page value used when operation is OK to update page status */
  ee_operation_callback_t operation_callback;/*!< flitf callback to report operation status */
  uint16_t uhNbWrittenElements;     /*!< Nb of elements written in valid and active pages */
  uint16_t ubNbErasingPage;         /*!< Number of erasing pages */
  uint32_t ubCurrentActivePage;     /*!< Current active page (can be active or receive state) */
  uint32_t ublatestValidPage;       /*!< latest valid page */
  uint8_t  ubInitDone;              /*!< variable to update init status */
  uint32_t uwAddressNextWrite;      /*!< Initialize write position just after page header */
#if defined(EE_DEBUG)
  uint32_t _EE_NB_OF_VARIABLES; /*!< Number of variables stored in EEPROM. */
  uint32_t _EE_ELEMENT_SIZE; /*!< Size of each element stored in EEPROM. */
  uint32_t _EE_FLASH_PAGE_SIZE; /*!< Size of each flash page in EEPROM. */
  uint32_t _EE_PAGE_HEADER_SIZE; /*!< Size of the header for each page in EEPROM. */
  uint32_t _EE_NB_MAX_WRITTEN_ELEMENTS; /*!< Maximum number of elements that can be written to EEPROM. */
  uint32_t _EE_NB_MAX_ELEMENTS_BY_PAGE; /*!< Maximum number of elements that can be stored in a single page of EEPROM. */
  uint32_t _EE_NB_OF_VARIABLES_DIV_EE_NB_MAX_ELEMENTS_BY_PAGE; /*!< Number of variables divided by the maximum number of elements per page. */
  uint32_t _EE_CYCLES_NUMBER; /*!< Number of cycles for EEPROM operations. */
  uint32_t _EE_GUARD_PAGES_NUMBER; /*!< Number of guard pages in EEPROM. */
  uint32_t _EE_PAGES_NUMBER; /*!< Total number of pages in EEPROM. */
#endif /* EE_DEBUG */
  ee_page_state_t Page_state[EE_PAGES_NUMBER];  /*!< Array to store page state, used for debug purpose      */
} ee_internal_state_t;

/**
  * @brief internal state data.
  *
  */
static ee_internal_state_t ee_internal_state =
{
  .ope_status = EE_OPERATION_NONE,
  .erase_it_ope = 0U,
  .operation_callback = NULL,
  .uwAddressNextWrite = EE_PAGE_HEADER_SIZE,
#if defined(EE_DEBUG)
  ._EE_NB_OF_VARIABLES = EE_NB_OF_VARIABLES,
  ._EE_ELEMENT_SIZE = EE_ELEMENT_SIZE,
  ._EE_FLASH_PAGE_SIZE = EE_FLASH_PAGE_SIZE,
  ._EE_PAGE_HEADER_SIZE = EE_PAGE_HEADER_SIZE,
  ._EE_NB_MAX_WRITTEN_ELEMENTS = EE_NB_MAX_WRITTEN_ELEMENTS,
  ._EE_NB_MAX_ELEMENTS_BY_PAGE = EE_NB_MAX_ELEMENTS_BY_PAGE,
  ._EE_NB_OF_VARIABLES_DIV_EE_NB_MAX_ELEMENTS_BY_PAGE = EE_NB_OF_VARIABLES / EE_NB_MAX_ELEMENTS_BY_PAGE,
  ._EE_CYCLES_NUMBER = EE_CYCLES_NUMBER,
  ._EE_GUARD_PAGES_NUMBER = EE_GUARD_PAGES_NUMBER,
  ._EE_PAGES_NUMBER = EE_PAGES_NUMBER,
#endif /* EE_DEBUG */
};

/**
  * @}
  */

/* Private function prototypes -----------------------------------------------*/
/** @defgroup EEPROM_EMULATION_ALGO_FLITF_PRIVATE_FUNCTIONS Algo FLITF Private Functions
  * @{
  */
static ee_status WriteVariable(ee_frame_t *data);
static ee_status ReadVariable(uint16_t virt_address,  ee_frame_t *p_data);
static ee_status VerifyPageFullyErased(uint32_t address);

static ee_status Frame_Get(uint32_t address, ee_frame_t *dataframe);
static ee_status Frame_Add(ee_frame_t *p_data);

static ee_status       Page_SetState(uint32_t page, ee_page_state_t state);
static uint32_t        Page_GetWrite(void);
static ee_page_state_t Page_GetState(uint32_t page);
static ee_status       Page_Removed(uint32_t page);

static void Operation(ee_itf_flash_operation operation_status);
/**
  * @}
  */

/* Exported functions -------------------------------------------------------*/
/** @addtogroup EEPROM_EMULATION_ALGO_FLITF_EXPORTED_FUNCTIONS
  * @{
  */

/**
  * @brief  Restore the pages to a known good state in case of power loss.
  *         If a page is in REMOVED state, resume the page remove operation.
  *         Then if some pages are in ERASING state, erase these pages.
  * @param  object pointer on eeprom_emulation object
  * @param  erase_type: @ref ee_erase_type Type of erase to apply on pages requiring an erase operation.
  * @retval ee_status: Refer to @ref ee_status for possible return values
  */
ee_status EE_FLITF_Init(ee_object_t *object, ee_erase_type erase_type)
{
  ee_frame_t FrameData;
  ee_page_state_t pagestatus = PAGE_STATE_INVALID;
  uint32_t page = 0U,
           nb_activepage = 0U, nb_removedpage = 0U, nb_validpage = 0U,
           nb_erasedpage = 0U;
  uint32_t address = 0U;
  uint32_t removed_page = 0U;

  ee_state_reliability_t page_reliability = STATE_CORRUPTED;
  ee_status status = EE_ERROR_CORRUPTION;

  /* Initialize the flash interface :  EE_ITF_FLASH_Init */
  (void)EE_ITF_FLASH_Init(object->f_object, Operation);

  /* Initialize the crc interface : EE_ITF_CRC_Init*/
  (void)EE_ITF_CRC_Init(object->crc_object);

  EE_DEBUG_PRINT("EE_NB_OF_VARIABLES=%d\n", EE_NB_OF_VARIABLES);
  EE_DEBUG_PRINT("EE_ELEMENT_SIZE=%d\n", EE_ELEMENT_SIZE);
  EE_DEBUG_PRINT("EE_FLASH_PAGE_SIZE=%d\n", EE_FLASH_PAGE_SIZE);
  EE_DEBUG_PRINT("EE_PAGE_HEADER_SIZE=%d\n", EE_PAGE_HEADER_SIZE);
  EE_DEBUG_PRINT("EE_NB_MAX_WRITTEN_ELEMENTS=%d\n", EE_NB_MAX_WRITTEN_ELEMENTS);
  EE_DEBUG_PRINT("-----------------------------------------------------\n");

  EE_DEBUG_PRINT("EE_NB_MAX_ELEMENTS_BY_PAGE=%d\n", EE_NB_MAX_ELEMENTS_BY_PAGE);

  EE_DEBUG_PRINT("(EE_NB_OF_VARIABLES / EE_NB_MAX_ELEMENTS_BY_PAGE)=%d\n",
                 (EE_NB_OF_VARIABLES / EE_NB_MAX_ELEMENTS_BY_PAGE));

  EE_DEBUG_PRINT("EE_CYCLES_NUMBER=%d\n", EE_CYCLES_NUMBER);
  EE_DEBUG_PRINT("EE_GUARD_PAGES_NUMBER=%d\n", EE_GUARD_PAGES_NUMBER);
  EE_DEBUG_PRINT("EE_PAGES_NUMBER=%d\n", EE_PAGES_NUMBER);

  /* reset the init process */
  ee_internal_state.ubInitDone = 0;
  ee_internal_state.ubNbErasingPage = 0;
  ee_internal_state.ubCurrentActivePage = EE_NO_PAGE_FOUND;
  ee_internal_state.ublatestValidPage   = EE_NO_PAGE_FOUND;

  EE_DEBUG_PRINT("Step0:\n");
  /***************************************************************************/
  /* Step 0: Check if an operation has been interrupted by a reset           */
  /***************************************************************************/
  switch (EE_ITF_FLASH_GetLastOperationStatus(&address))
  {
    case EE_ITF_FLASH_OPEINT_ERASED:
      (void)EE_ITF_FLASH_PageErase(address, 1U);
      break;
    case EE_ITF_FLASH_OPEINT_WRITE:
    /* any interrutped write operation requires no action,
       thanks to CRC control on the data and Flash hardware ECC feature */
    default:
      /* nothing to do */
      break;
  }

  EE_DEBUG_PRINT("Step1:\n");
  /***************************************************************************/
  /* Step 1: Get the page status for all the pages                           */
  /***************************************************************************/
  /* read all pages header to get their status */
  for (page = EE_START_PAGE; page < (EE_START_PAGE + EE_PAGES_NUMBER); page++)
  {
    pagestatus = Page_GetState(page);

    /*  store the page status */
    ee_internal_state.Page_state[page - EE_START_PAGE] = pagestatus;

    switch (pagestatus)
    {
      case PAGE_STATE_ACTIVE:
      {
        ee_internal_state.ubCurrentActivePage = page;
        nb_activepage++;
        break;
      }
      case PAGE_STATE_REMOVED:
      {
        nb_removedpage++;
        removed_page = page;
        break;
      }
      case PAGE_STATE_VALID:
      {
        nb_validpage++;
        break;
      }
      case PAGE_STATE_ERASING :
      {
        break;
      }
      case PAGE_STATE_ERASED :
      {
        nb_erasedpage++;
        break;
      }
      default :
      {
        /* the page is in unknown state; the system is corrupted */
        goto error;
        break;
      }
    }
  }

#if defined(EE_DEBUG)
  /* Print the state of the pages */
  for (uint32_t idx = 0; idx < EE_PAGES_NUMBER; idx++)
  {
    EE_DEBUG_PRINT("Page[%d]= %s\n", idx + EE_START_PAGE, ee_state_type_string[ee_internal_state.Page_state[idx]]);
  }
#endif /* EE_DEBUG */

  /* page evaluation complete, enable the page state read from internal state */
  ee_internal_state.ubInitDone = 1;

  /* check the valid page */
  if (nb_validpage != 0U)
  {
    EE_DEBUG_PRINT("Step2:\n");
    /*********************************************************************/
    /* Step 2: recover the last valid page                               */
    /*********************************************************************/
    page = FOLLOWING_PAGE(ee_internal_state.ubCurrentActivePage);
    while ((Page_GetState(page) != PAGE_STATE_VALID)
           && (page != ee_internal_state.ubCurrentActivePage))
    {
      page = FOLLOWING_PAGE(page);
    }

    if (Page_GetState(page) == PAGE_STATE_VALID)
    {
      ee_internal_state.ublatestValidPage = page;
    }

    EE_DEBUG_PRINT("Step3:\n");
    /***************************************************************************/
    /* Step 3: check the continuity of valid page                              */
    /***************************************************************************/
    /* start from the latest valid page */
    page = ee_internal_state.ublatestValidPage;
    while (Page_GetState(page) == PAGE_STATE_VALID)
    {
      nb_validpage--;
      page = FOLLOWING_PAGE(page);
    }
  }
  /* check that number of valid page is coherent */
  if (nb_validpage != 0U)
  {
    goto error;
  }

  EE_DEBUG_PRINT("Step4:\n");
  /***************************************************************************/
  /* Step 4: check position of remove page                                   */
  /*         must be before VALID or ACTIVE page                             */
  /***************************************************************************/
  switch (nb_removedpage)
  {
    case 1U:
    {
      switch (Page_GetState(FOLLOWING_PAGE(removed_page)))
      {
        case PAGE_STATE_VALID:
        case PAGE_STATE_ACTIVE:
          /* the position is OK */
          break;
        default:
          goto error;
          break;
      }
      break;
    }
    case 0U:
      /* nothing to do */
      break;
    default :
      /* this case is not possible and must be considered as an error */
      goto error;
      break;
  }

  EE_DEBUG_PRINT("Step5:\n");
  /***************************************************************************/
  /* Step 5: repair the page status, if needed                               */
  /*         by setting the missing valid page state                         */
  /***************************************************************************/
  /* consider the number of active pages present in the system */
  switch (nb_activepage)
  {
    case 2U:
    {
      /* reset occurs during activation of a new page */
      if (Page_GetState(PREVIOUS_PAGE(ee_internal_state.ubCurrentActivePage)) == PAGE_STATE_ACTIVE)
      {
        /* set hold active page to valid */
        if (EE_OK != Page_SetState(PREVIOUS_PAGE(ee_internal_state.ubCurrentActivePage), PAGE_STATE_VALID))
        {
          goto error;
        }
        page_reliability = STATE_RELIABLE;
      }
      else if (Page_GetState(FOLLOWING_PAGE(ee_internal_state.ubCurrentActivePage)) == PAGE_STATE_ACTIVE)
      {
        /* set hold active page to valid */
        if (EE_OK != Page_SetState(ee_internal_state.ubCurrentActivePage, PAGE_STATE_VALID))
        {
          goto error;
        }

        /* set the next page as the current active page */
        ee_internal_state.ubCurrentActivePage = FOLLOWING_PAGE(ee_internal_state.ubCurrentActivePage);

        page_reliability = STATE_RELIABLE;
      }
      else
      {
        /* must never occur because 2 active pages are always consecutive */
        /* return that the system is corrupted */
      }
      break;
    }
    case 1U:
    {
      /* standard boot case data are present */
      page_reliability = STATE_RELIABLE;
      break;
    }
    case 0U:  /* only possible if flash is virgin */
    {
      if (nb_erasedpage == EE_PAGES_NUMBER)
      {
        page_reliability = STATE_RELIABLE;
        EE_DEBUG_PRINT("goto Step9:\n");
        goto check_erase;
      }
      break;
    }
    default :  /* this inconsistent and we will force an erase of the code */
    {
      break;
    }
  }

  if (page_reliability != STATE_RELIABLE)
  {
    goto error;
  }

  EE_DEBUG_PRINT("Step6:\n");
  /*********************************************************************/
  /* Step 6: Initialize eeprom emulation global variables relative     */
  /*         to active page                                            */
  /*********************************************************************/
  /* Initialize des compteurs : uhNbWrittenElements, uwAddressNextWrite */
  ee_internal_state.uhNbWrittenElements = 0U;
  ee_internal_state.uwAddressNextWrite = EE_PAGE_HEADER_SIZE;

  for (uint32_t varidx = EE_PAGE_HEADER_SIZE; varidx < EE_FLASH_PAGE_SIZE; varidx += EE_ELEMENT_SIZE)
  {
    if (EE_ITF_FLASH_ReadData(GetPageAddress(ee_internal_state.ubCurrentActivePage) + varidx,
                              (uint8_t *)FrameData.data_64_bits_t.data64,
                              (uint16_t)(sizeof(ee_frame_t) / sizeof(uint8_t))) == EE_ITF_FLASH_ERROR_ECCC)
    {
      /* the data seems to be invalid but it must be counted*/
      FrameData.data_64_bits_t.data64[0] = 0U;
    }

    /* Check elements present in active page */
#if defined(EE_FRAME_LINE_SIZE) && (EE_FRAME_LINE_SIZE == 16)
    if ((FrameData.data_64_bits_t.data64[0] != EE_PAGESTAT_ERASED)
        && (FrameData.data_64_bits_t.data64[1] != EE_PAGESTAT_ERASED))
#else
    if (FrameData.data_64_bits_t.data64[0] != EE_PAGESTAT_ERASED)
#endif /* defined(EE_FRAME_LINE_SIZE) && (EE_FRAME_LINE_SIZE == 16) */
    {
      /* Then increment uhNbWrittenElements and uwAddressNextWrite */
      ee_internal_state.uhNbWrittenElements++;
      ee_internal_state.uwAddressNextWrite += EE_ELEMENT_SIZE;
    }
    else /* no more element in the page */
    {
      break;
    }
  }

  EE_DEBUG_PRINT("Step7:\n");
  /*********************************************************************/
  /* Step 7: Finalize eeprom emulation global variables relative       */
  /*         to valid pages, increase the uhNbWrittenElements with     */
  /*         valid page                                                */
  /*********************************************************************/
  /* Update global variable uhNbWrittenElements if valid pages are found */
  page = PREVIOUS_PAGE(ee_internal_state.ubCurrentActivePage);
  while (Page_GetState(page) == PAGE_STATE_VALID)
  {
    /* Update uhNbWrittenElements with number of elements in full page */
    ee_internal_state.uhNbWrittenElements += EE_NB_MAX_ELEMENTS_BY_PAGE;
    /* Decrement page index among circular pages list */
    page = PREVIOUS_PAGE(page);
  }

  EE_DEBUG_PRINT("Step8:\n");
  /************************************************************************/
  /* Step 8: manage the remove page                                       */
  /************************************************************************/
  if (nb_removedpage == 1U)
  {
    /* add the element corresponding to remove page */
    ee_internal_state.uhNbWrittenElements += EE_NB_MAX_ELEMENTS_BY_PAGE;
    /* an removed operation has been aborted, restart the remove operation */
    if (EE_OK != Page_Removed(removed_page))
    {
      goto error;
    }
  }

check_erase :
  EE_DEBUG_PRINT("Step9:\n");
  /*********************************************************************/
  /* Step 9: Ensure empty pages are erased (ERASING or ERASED page)    */
  /*********************************************************************/
  for (page = EE_START_PAGE; page < (EE_START_PAGE + EE_PAGES_NUMBER); page++)
  {
    pagestatus = Page_GetState(page);

    switch (pagestatus)
    {
      case PAGE_STATE_ERASING:
        /* Force page erase independently of its content */
        if (Page_SetState(page, PAGE_STATE_ERASED) !=  EE_OK)
        {
          status = EE_ERROR_ITF_FLASH;
          goto error;
        }
        break;
      case PAGE_STATE_ERASED:
        if (erase_type == EE_FORCED_ERASE)
        {
          /* Force page erase independently of its content */
          if (Page_SetState(page, PAGE_STATE_ERASED) !=  EE_OK)
          {
            status = EE_ERROR_ITF_FLASH;
            goto error;
          }
        }
        else
        {
          if (VerifyPageFullyErased(GetPageAddress(page)) != EE_OK)
          {
            /* Force page erase independently of its content */
            if (Page_SetState(page, PAGE_STATE_ERASED) !=  EE_OK)
            {
              status = EE_ERROR_ITF_FLASH;
              goto error;
            }
          }
        }
        break;
      default :
        /* no action required */
        break;
    }
  }

  EE_DEBUG_PRINT("Step10:\n");
  /************************************************************************/
  /* Step 10: if system has only ERASED page, set the first page as ACTIVE */
  /************************************************************************/
  if (nb_erasedpage == EE_PAGES_NUMBER)
  {
    /* mark the first page VALID */
    if (Page_SetState(EE_START_PAGE, PAGE_STATE_ACTIVE) !=  EE_OK)
    {
      status = EE_ERROR_ITF_FLASH;
      goto error;
    }
  }

  /* set status to EE_OK */
  status = EE_OK;

error :
  /* goto error must be considered has crash */
  return status;
}

/**
  * @brief  Erases all flash pages of eeprom emulation and sets the first page header as ACTIVE.
  * @param  erase_type  @ref ee_erase_type Type of erase to apply on pages requiring an erase operation.
  * @note   This function can be called the very first time eeprom emulation is used to prepare flash pages for
  * eeprom emulation with empty eeprom variables. It can also be called at any time to flush all eeprom variables.
  * @retval ee_status  Refer to @ref ee_status for possible return values
  */
ee_status EE_FLITF_Format(ee_erase_type erase_type)
{
  uint32_t page;

  /* Erase All Pages */
  for (page = EE_START_PAGE; page < (EE_START_PAGE + EE_PAGES_NUMBER); page++)
  {
    /* Check if page erase has to be forced unconditionally (default case) */
    if (erase_type == EE_FORCED_ERASE)
    {
      /* Force page erase independently of its content */
      if (Page_SetState(page, PAGE_STATE_ERASED) !=  EE_OK)
      {
        return EE_ERROR_ITF_FLASH;
      }
    }
    else /* erase_type == EE_CONDITIONAL_ERASE */
    {
      switch (Page_GetState(page))
      {
        case PAGE_STATE_ERASED:
          /* Check if Page is not yet fully erased */
          if (VerifyPageFullyErased(GetPageAddress(page)) != EE_OK)
          {
            /* If Erase operation was failed, a Flash error code is returned */
            if (Page_SetState(page, PAGE_STATE_ERASED) !=  EE_OK)
            {
              return EE_ERROR_ITF_FLASH;
            }
            else
            {
              /* Page is now fully erased */
              break;
            }
          }
        default:
          /* Force page erase independently of its content */
          if (Page_SetState(page, PAGE_STATE_ERASED) !=  EE_OK)
          {
            return EE_ERROR_ITF_FLASH;
          }
          break;
      }
    }
  }

  /* Set first Page in Active State */
  /* If program operation was failed, a Flash error code is returned */
  if (Page_SetState(EE_START_PAGE, PAGE_STATE_ACTIVE) != EE_OK)
  {
    return EE_ERROR_ITF_FLASH;
  }

  /* Reset global variables */
  ee_internal_state.uhNbWrittenElements = (uint16_t)0U;
  ee_internal_state.ubCurrentActivePage = EE_START_PAGE;
  ee_internal_state.ublatestValidPage   = EE_NO_PAGE_FOUND;
  ee_internal_state.uwAddressNextWrite  = EE_PAGE_HEADER_SIZE; /* Initialize write position just after page header */
  return EE_OK;
}

/**
  * @brief  Returns the last stored variable data corresponding to the passed virtual address.
  * @param virt_address: Variable virtual address on 16 bits (cannot be 0x0000 or 0xFFFF)
  * @param p_data: 96-bit data to be written, pointed to by a uint64_t pointer
  * @retval ee_status Refer to @ref ee_status for possible return values
  */
ee_status EE_FLITF_ReadVariable96bits(uint16_t virt_address, uint64_t *p_data)
{
#if defined(EE_FRAME_LINE_SIZE) && (EE_FRAME_LINE_SIZE == 16)
  ee_status status;
  ee_frame_t   dataframe;
  /* Read variable of size EE_DATA_TYPE */
  status = ReadVariable(virt_address, &dataframe);

  p_data[0] = *((uint64_t *) & (dataframe.frame_t.data[0]));
  p_data[1] = *((uint32_t *) & (dataframe.frame_t.data[8]));

  return status;
#else
  (void)(virt_address);
  (void)(p_data);
  return EE_INFO_NOTSUPPORTED;
#endif /* EE_FRAME_LINE_SIZE == 16 */
}

/**
  * @brief  Returns the last stored variable data corresponding to the passed virtual address.
  * @param  virt_address: Variable virtual address on 16 bits (cannot be 0x0000 or 0xFFFF).
  * @param  p_data Variable containing the 32-bit read variable value.
  * @retval ee_status Refer to @ref ee_status for possible return values.
  */
ee_status EE_FLITF_ReadVariable32bits(uint16_t virt_address, uint32_t *p_data)
{
  ee_status status;
  ee_frame_t dataframe;
  /* Read variable of size EE_DATA_TYPE, then cast it to 32bits */
  status = ReadVariable(virt_address, &dataframe);
  *p_data = *(uint32_t *)(&dataframe.frame_t.data[0]);
  return status;
}

/**
  * @brief  Returns the last stored variable data, if found, which correspond to the passed virtual address.
  * @param virt_address Variable virtual address on 16 bits (can't be 0x0000 or 0xFFFF)
  * @param  p_data Variable containing the 16bits read variable value
  * @retval ee_status @ref ee_status
  */
ee_status EE_FLITF_ReadVariable16bits(uint16_t virt_address, uint16_t *p_data)
{
  ee_status status;
  ee_frame_t dataframe;

  /* Read variable of size EE_DATA_TYPE, then cast it to 16bits */
  status = ReadVariable(virt_address, &dataframe);
  *p_data = *(uint16_t *)(&dataframe.frame_t.data[0]);
  return status;
}

/**
  * @brief  Returns the last stored variable data corresponding to the passed virtual address.
  * @param virt_address: Variable virtual address on 16 bits (cannot be 0x0000 or 0xFFFF)
  * @param p_data: Variable containing the 8-bit read variable value
  * @retval ee_status: Refer to @ref ee_status for possible return values
  */
ee_status EE_FLITF_ReadVariable8bits(uint16_t virt_address, uint8_t *p_data)
{
  ee_status status = EE_OK;
  ee_frame_t dataframe;
  /* Read variable of size EE_DATA_TYPE, then cast it to 8bits */
  status = ReadVariable(virt_address, &dataframe);
  *p_data = dataframe.frame_t.data[0];
  return status;
}

/**
  * @brief  Writes/updates variable data in EEPROM.
  *         Triggers internal pages transfer if half of the pages are full.
  * @param  virt_address Variable virtual address on 16 bits (cannot be 0x0000 or 0xFFFF).
  * @param  p_data 96-bit data to be written, pointed to by a uint64_t pointer.
  * @warning This function is not reentrant.
  * @retval ee_status Refer to @ref ee_status for possible return values.
  */
ee_status EE_FLITF_WriteVariable96bits(uint16_t virt_address, uint64_t *p_data)
{
  ee_status status;
  ee_frame_t lframe = {0};

  /* construction of the frame to write */
  /* 1 - stored the virtual address */
  lframe.frame_t.virtual_address = virt_address;
  /* 2 - fill the data */
  for (uint8_t index = 0U; index < EE_FRAME_DATA_SIZE; index++)
  {
    lframe.frame_t.data[index] = ((uint8_t *)p_data)[index];
  }
  /* 3 - calcul the CRC */
  if (EE_ITF_CRC_Calcul16Bit((uint8_t *)&lframe.frame_t.virtual_address, EE_FRAME_DATA_SIZE + 2U,
                             &lframe.frame_t.crc) != EE_ITF_CRC_OK)
  {
    status = EE_ERROR_ITF_CRC;
  }
  else
  {
    status = WriteVariable(&lframe);
  }
  return status;
}

/**
  * @brief  Writes/updates variable data in EEPROM.
  *         Triggers internal pages transfer if half of the pages are full.
  * @param  virt_address Variable virtual address on 16 bits (cannot be 0x0000 or 0xFFFF).
  * @param  data 32-bit data to be written.
  * @warning  This function is not reentrant.
  * @retval ee_status Refer to @ref ee_status for possible return values.
  */
ee_status EE_FLITF_WriteVariable32bits(uint16_t virt_address, uint32_t data)
{
  ee_status status;
  ee_frame_t frame = {0U};

  /* construction of the frame to write */
  /* 1 - stored the virtual address */
  frame.frame_t.virtual_address = virt_address;
  /* 2 - fill the data */
  for (uint8_t index = 0U; index < EE_FRAME_DATA_SIZE; index++)
  {
    if (index < sizeof(uint32_t))
    {
      frame.frame_t.data[index] = ((uint8_t *)&data)[index];
    }
    else
    {
      frame.frame_t.data[index] = 0U;
    }
  }
  /* 3 - calcul the CRC */
  if (EE_ITF_CRC_Calcul16Bit((uint8_t *)&frame.frame_t.virtual_address, EE_FRAME_DATA_SIZE + 2U,
                             &frame.frame_t.crc) != EE_ITF_CRC_OK)
  {
    status = EE_ERROR_ITF_CRC;
  }
  else
  {
    /* write the formatted frame */
    status = WriteVariable(&frame);
  }

  return status;
}

/**
  * @brief  Writes/updates variable data in EEPROM.
  *         Triggers internal pages transfer if half of the pages are full.
  * @param  virt_address: Variable virtual address on 16 bits (cannot be 0x0000 or 0xFFFF).
  * @param  data 16-bit data to be written.
  * @warning This function is not reentrant.
  * @retval ee_status Refer to @ref ee_status for possible return values.
  */
ee_status EE_FLITF_WriteVariable16bits(uint16_t virt_address, uint16_t data)
{
  ee_status status;
  ee_frame_t frame = {0U};

  /* construction of the frame to write */
  /* 1 - stored the virtual address */
  frame.frame_t.virtual_address = virt_address;
  /* 2 - fill the data */
  for (uint8_t index = 0U; index < EE_FRAME_DATA_SIZE; index++)
  {
    if (index < sizeof(uint16_t))
    {
      frame.frame_t.data[index] = ((uint8_t *)&data)[index];
    }
    else
    {
      frame.frame_t.data[index] = 0U;
    }
  }
  /* 3 - calcul the CRC */
  if (EE_ITF_CRC_Calcul16Bit((uint8_t *)&frame.frame_t.virtual_address, EE_FRAME_DATA_SIZE + 2U,
                             &frame.frame_t.crc) != EE_ITF_CRC_OK)
  {
    status = EE_ERROR_ITF_CRC;
  }
  else
  {
    /* write the formatted frame */
    status = WriteVariable(&frame);
  }
  return status;
}

/**
  * @brief  Writes/updates variable data in EEPROM.
  *         Triggers internal pages transfer if half of the pages are full.
  * @param  virt_address: Variable virtual address on 16 bits (cannot be 0x0000 or 0xFFFF)
  * @param  data: 8-bit data to be written.
  * @warning This function is not reentrant.
  * @retval ee_status: Refer to @ref ee_status for possible return values.
  */
ee_status EE_FLITF_WriteVariable8bits(uint16_t virt_address, uint8_t data)
{
  ee_status status;
  ee_frame_t frame = {0U};

  /* construction of the frame to write */
  /* 1 - stored the virtual address */
  frame.frame_t.virtual_address = virt_address;
  /* 2 - fill the data */
  for (uint8_t index = 0U; index < EE_FRAME_DATA_SIZE; index++)
  {
    if (index < sizeof(uint8_t))
    {
      frame.frame_t.data[index] = ((uint8_t *)&data)[index];
    }
    else
    {
      frame.frame_t.data[index] = 0U;
    }
  }
  /* 3 - calcul the CRC */
  if (EE_ITF_CRC_Calcul16Bit((uint8_t *)&frame.frame_t.virtual_address, EE_FRAME_DATA_SIZE + 2U,
                             &frame.frame_t.crc) != EE_ITF_CRC_OK)
  {
    status = EE_ERROR_ITF_CRC;
  }
  else
  {
    /* write the formatted frame */
    status = WriteVariable(&frame);
  }
  return status;
}


/**
  * @brief  Erase a group of pages that are in erasing state, in polling mode.
  *         The group could be either the first half or the second half of the total number of pages.
  * @note   This function must be called when EE_WriteVariableXXbits has returned
  *         EE_CLEANUP_REQUIRED status (and only in that case).
  * @retval ee_status: Refer to @ref ee_status for possible return values.
  */
ee_status EE_FLITF_CleanUp(void)
{
  uint32_t first_page = 0U;
  uint16_t page_counter = 0U;
  uint32_t last_page;
  uint32_t page_address;
  ee_page_state_t pagestatus = PAGE_STATE_INVALID;
  ee_status status = EE_OK;

  EE_DEBUG_PRINT("EE_FLITF_CleanUp\n");

  /* check Garbage collector operation level */
  if (ee_internal_state.uhNbWrittenElements >= (uint16_t)EE_NB_MAX_WRITTEN_ELEMENTS)
  {
    /* free latest valid page to avoid any writing issue */
    if (Page_Removed(ee_internal_state.ublatestValidPage) != EE_OK)
    {
      status = EE_ERROR_ALGO;
      goto error;
    }
  }

  /* get the following page */
  first_page = FOLLOWING_PAGE(ee_internal_state.ubCurrentActivePage);

  /* get the first erasing page */
  for (; ((first_page != ee_internal_state.ubCurrentActivePage) && (pagestatus != PAGE_STATE_ERASING));
       first_page = FOLLOWING_PAGE(first_page))
  {
    pagestatus = Page_GetState(first_page);
  }

  first_page = PREVIOUS_PAGE(first_page);

  if (pagestatus == PAGE_STATE_ERASING)
  {
    /* Get the latest ERASING page */
    for (last_page = first_page;
         last_page < (EE_START_PAGE + EE_PAGES_NUMBER); last_page++)
    {
      if (Page_GetState(last_page) == PAGE_STATE_ERASING)
      {
        /* increment page erasing counter */
        page_counter++;
      }
      else
      {
        break;
      }
    }

    page_address = GetPageAddress(first_page);

    /* Erase all the pages of the group */
    if (EE_ITF_FLASH_PageErase(page_address, page_counter) != EE_ITF_FLASH_OK)
    {
      /* erase operation fails, a Flash error code is returned */
      status = EE_ERROR_ITF_FLASH;
      goto error;
    }

    /* decrease the erasing page counter */
    ee_internal_state.ubNbErasingPage -= page_counter;

    /*  refresh the page state */
    for (uint16_t index = 0; index < page_counter; index++)
    {
      ee_internal_state.Page_state[first_page + index - EE_START_PAGE] = PAGE_STATE_ERASED;
    }
#ifdef EE_DEBUG
    for (uint8_t index = 0; index < page_counter; index++)
    {
      EE_DEBUG_PRINT("___setPAGE[%d]=_%s_\n", first_page + index, ee_state_type_string[PAGE_STATE_ERASED]);
    }
#endif /* EE_DEBUG */
  }

error :
  return status;
}

/**
  * @brief  Erase a group of pages that are in erasing state, in interrupt mode.
  *         The group could be either the first half or the second half of the total number of pages.
  * @param  operation_complete callback used to return operation status
  * @note   if operation_complete is NULL the status call be read with API EE_FLITF_GetStatusCleanUp_IT
  * @note   This function must be called when EE_WriteVariableXXbits has returned EE_CLEANUP_REQUIRED
  *         status (and only in that case).
  * @retval ee_status: Refer to @ref ee_status for possible return values.
  */
ee_status EE_FLITF_CleanUp_IT(ee_operation_callback_t operation_complete)
{
  uint32_t first_page = 0U;
  uint16_t page_counter = 0U;
  uint32_t last_page;
  uint32_t page_address;
  ee_page_state_t pagestatus = PAGE_STATE_INVALID;
  ee_status status = EE_OK;

  /* store the operation callback */
  ee_internal_state.ope_status = EE_OPERATION_NONE;
  ee_internal_state.operation_callback = operation_complete;

  /* check if Garbage collector operation is required */
  if (ee_internal_state.uhNbWrittenElements >= (uint16_t)EE_NB_MAX_WRITTEN_ELEMENTS)
  {
    /* removed the latest valid page */
    if (Page_Removed(ee_internal_state.ublatestValidPage) != EE_OK)
    {
      status = EE_ERROR_ALGO;
      goto error;
    }
  }

  /* get the following page */
  first_page = FOLLOWING_PAGE(ee_internal_state.ubCurrentActivePage);

  for (; ((first_page != ee_internal_state.ubCurrentActivePage) && (pagestatus != PAGE_STATE_ERASING));
       first_page = FOLLOWING_PAGE(first_page))
  {
    pagestatus = Page_GetState(first_page);
  }

  first_page = PREVIOUS_PAGE(first_page);

  if (pagestatus == PAGE_STATE_ERASING)
  {
    /* Get the latest ERASING page */
    for (last_page = first_page;
         last_page < (EE_START_PAGE + EE_PAGES_NUMBER); last_page++)
    {
      if (Page_GetState(last_page) == PAGE_STATE_ERASING)
      {
        page_counter++;
      }
      else
      {
        break;
      }
    }

    page_address = GetPageAddress(first_page);

    ee_internal_state.ope_status = EE_OPERATION_ONGOING;
    ee_internal_state.erase_it_ope  = 1U;
    ee_internal_state.erase_it_page_counter = page_counter;
    ee_internal_state.erase_it_first_page = first_page;

    /* Erase all the pages of the group */
    /* If erase operation fails, a Flash error code is returned */
    if (EE_ITF_FLASH_PageErase_IT(page_address, page_counter) != EE_ITF_FLASH_OK)
    {
      ee_internal_state.erase_it_ope = 0U;
      ee_internal_state.ope_status = EE_OPERATION_ERROR;
      status = EE_ERROR_ITF_FLASH;
      goto error;
    }

    ee_internal_state.ubNbErasingPage -= page_counter;
  }

error :
  return status;
}

/**
  * @brief  return the status of cleanup interrupt operation.
  * @note   This function must be called when EE_FLITF_CleanUp_IT to control the end of the operation
  *         and before any new eeprom operation.
  * @retval ee_operation_status: Refer to @ref ee_operation_status for possible return values.
  */
ee_operation_status EE_FLITF_GetStatusCleanUp_IT(void)
{
  return ee_internal_state.ope_status;
}
/**
  * @}
  */

/* Private functions ---------------------------------------------------------*/
/** @addtogroup EEPROM_EMULATION_ALGO_FLITF_PRIVATE_FUNCTIONS
  * @{
  */

/**
  * @brief  Returns the last stored variable data, if found, which correspond to the passed virtual address.
  * @param virt_address Variable virtual address on 16 bits (can't be 0x0000 or 0xFFFF)
  * @param  p_data Variable containing the EE_DATA_TYPE read variable value
  * @retval ee_status @ref ee_status
  */
static ee_status ReadVariable(uint16_t virt_address, ee_frame_t *p_data)
{
  uint32_t page;
  uint32_t pageaddress = 0U;
  uint32_t counter = 0U;
  ee_page_state_t pagestate = PAGE_STATE_INVALID;
  uint16_t crc_value;

  /* Get active Page for data read operation */
  page =  ee_internal_state.ubCurrentActivePage;

  /* Get page info */
  pageaddress = GetPageAddress(page);
  pagestate = Page_GetState(page);

  /* Search variable in active page and valid pages until another type of page is found */
  while ((pagestate == PAGE_STATE_ACTIVE) || (pagestate == PAGE_STATE_VALID))
  {
    /* Set counter index to last element position in the page */
    counter = EE_FLASH_PAGE_SIZE - EE_ELEMENT_SIZE;
    if (pagestate == PAGE_STATE_ACTIVE)
    {
      if (ee_internal_state.uwAddressNextWrite ==  EE_PAGE_HEADER_SIZE)
      {
        /* page contains no data */
        counter = EE_PAGE_HEADER_SIZE;
      }
      else if (ee_internal_state.uwAddressNextWrite !=  EE_FLASH_PAGE_SIZE)
      {
        /* page is partially written */
        counter = ((((uint32_t)ee_internal_state.uhNbWrittenElements % EE_NB_MAX_ELEMENTS_BY_PAGE) - 1U) \
                   * EE_ELEMENT_SIZE) + EE_PAGE_HEADER_SIZE;
      }
      else
      {
        /* current page is full, so we must read all element in the flash */
      }
    }

    /* Check each page address starting from end */
    while (counter >= EE_PAGE_HEADER_SIZE)
    {
      /* Get the current location content to be compared with virtual address */
      if (EE_ITF_FLASH_OK  == EE_ITF_FLASH_ReadData(pageaddress + counter, (uint8_t *)p_data->data_64_bits_t.data64,
                                                    (uint16_t)(sizeof(ee_frame_t) / sizeof(uint8_t))))
      {
#if EE_FRAME_LINE_SIZE == 16
        if (!((p_data->data_64_bits_t.data64[0] == EE_PAGESTAT_ERASED)
              && (p_data->data_64_bits_t.data64[1] == EE_PAGESTAT_ERASED)))
#elif EE_FRAME_LINE_SIZE == 8
        if (!(p_data->data_64_bits_t.data64[0] == EE_PAGESTAT_ERASED))
#else
#error "this use cas is not handled by EE"
#endif /* EE_FRAME_LINE_SIZE */
        {
          /* Compare the read address with the virtual address */
          if (p_data->frame_t.virtual_address == virt_address)
          {
            if (EE_ITF_CRC_Calcul16Bit((uint8_t *)&p_data->frame_t.virtual_address,
                                       (uint16_t)(EE_FRAME_DATA_SIZE + 2UL), &crc_value) == EE_ITF_CRC_OK)
            {
              if (p_data->frame_t.crc == crc_value)
              {
                return EE_OK;
              }
            }
          }
        }
      }
      /* Next address location */
      counter -= EE_ELEMENT_SIZE;
    }

    /* Decrement page index circularly, among pages allocated to eeprom emulation */
    page = PREVIOUS_PAGE(page);
    pageaddress = GetPageAddress(page);
    pagestate = Page_GetState(page);
  }

  /* Variable is not found */
  return EE_INFO_NODATA;
}

/**
  * @brief  Writes/updates variable data in EEPROM.
  *         Trig internal Pages transfer if half of the pages are full
  * @param  data EE_DATA_TYPE data to be written
  * @retval ee_status @ref ee_status
  */
static ee_status WriteVariable(ee_frame_t *data)
{
  ee_status status;

  /* check if a page must be removed */
  if (ee_internal_state.uhNbWrittenElements \
      >= (uint16_t)(EE_NB_MAX_WRITTEN_ELEMENTS + (EE_NB_MAX_ELEMENTS_BY_PAGE / 2U)))
  {
    /* free latest valid page to avoid any writing issue */
    status = Page_Removed(ee_internal_state.ublatestValidPage);
    if (status != EE_OK)
    {
      goto error;
    }
  }

  /* Write the variable virtual address and value in the EEPROM, if not full */
  status = Frame_Add(data);

  if (status == EE_OK) /* to avoid the erase of an error */
  {
    if ((ee_internal_state.uhNbWrittenElements > (uint16_t)EE_NB_MAX_WRITTEN_ELEMENTS)
        || (ee_internal_state.ubNbErasingPage > 0UL))
    {
      status = EE_INFO_CLEANUP_REQUIRED;
    }
  }
error:
  /* Return last operation status */
  return status;
}

/**
  * @brief  Verify if specified page is fully erased.
  * @param  address page address
  * @retval ee_status
  *           - @ref EE_ERROR_ALGO : if Page not erased
  *           - @ref EE_OK    : if Page erased
  */
static ee_status VerifyPageFullyErased(uint32_t address)
{
  ee_status readstatus = EE_OK;
  uint32_t counter = 0U;
  ee_frame_t data;

  /* Check each element in the page */
  while (counter < EE_FLASH_PAGE_SIZE)
  {
    if (EE_ITF_FLASH_ReadData(address + counter, (uint8_t *)data.data_64_bits_t.data64,
                              (uint16_t)(sizeof(ee_frame_t) / sizeof(uint8_t))) == EE_ITF_FLASH_ERROR_ECCC)
    {
      /* if an ECC error occurs the page must be erased */
      data.data_64_bits_t.data64[0] = 0;
    }
    /* Compare the read address with the virtual address */
#if defined(EE_FRAME_LINE_SIZE) && (EE_FRAME_LINE_SIZE == 16)
    if ((data.data_64_bits_t.data64[0] != EE_PAGESTAT_ERASED) && (data.data_64_bits_t.data64[1] != EE_PAGESTAT_ERASED))
#else
    if (data.data_64_bits_t.data64[0] != EE_PAGESTAT_ERASED)
#endif /* defined(EE_FRAME_LINE_SIZE) && (EE_FRAME_LINE_SIZE == 16) */
    {
      /* In case one element is not erased, reset readstatus flag */
      readstatus = EE_ERROR_ALGO;
      break;
    }
    /* Next address location */
    counter = counter + EE_ELEMENT_SIZE;
  }

  /* Return readstatus value */
  return readstatus;
}

/**
  * @brief get the write page for operation.
  * @return the page number or EE_NO_PAGE_FOUND.
  */
static uint32_t Page_GetWrite(void)
{
  ee_page_state_t currentpagestatus;
  uint32_t page_return = EE_NO_PAGE_FOUND;

  /* Get currentpage status */
  currentpagestatus = Page_GetState(ee_internal_state.ubCurrentActivePage);

  /* Normal operation, no page transfer on going */
  if (currentpagestatus == PAGE_STATE_ACTIVE)
  {
    /* Check if active page is not full */
    if (ee_internal_state.uwAddressNextWrite < EE_FLASH_PAGE_SIZE)
    {
      /* Return current Active page */
      page_return = ee_internal_state.ubCurrentActivePage;
    }
    else /* No more space in current active page */
    {
      EE_DEBUG_PRINT("No more space in the current active page\n");
      uint32_t page = ee_internal_state.ubCurrentActivePage;

      /* Get followingpage status */
      uint32_t followingpage = FOLLOWING_PAGE(ee_internal_state.ubCurrentActivePage);
      ee_page_state_t followingpagestatus = Page_GetState(followingpage);

      switch (followingpagestatus)
      {
        case PAGE_STATE_ERASING:  /* Check if following page is erasing state */
          /* Force Cleanup, as not yet performed by user */
          if (Page_SetState(followingpage, PAGE_STATE_ERASED) != EE_OK)
          {
            goto error;
          }
        case PAGE_STATE_ERASED:
          /* Set following page as active */
          if (Page_SetState(followingpage, PAGE_STATE_ACTIVE) != EE_OK)
          {
            goto error;
          }
          /* Set current active page in valid state */
          if (Page_SetState(page, PAGE_STATE_VALID) != EE_OK)
          {
            goto error;
          }
          ee_internal_state.uwAddressNextWrite = EE_PAGE_HEADER_SIZE;   /* Skip page header */
          ee_internal_state.ubCurrentActivePage = followingpage;        /* moved the current page */
          page_return = followingpage;                                  /* Following page is now active one */
          break;
        default:
          break;
      }
    }
  }
error :
  return page_return;
}

/**
  * @brief  remove a page from the eeprom system.
  *         All the data present in the frame are added in the system except if more recent value is detected.
  * @param  page page number
  * @retval ee_status Refer to @ref ee_status for possible return values.
  */
static ee_status Page_Removed(uint32_t page)
{
  ee_status status = EE_ERROR_ITF_FLASH;
  ee_frame_t data, data_tmp;
  uint32_t page_address = GetPageAddress(page);

  /* Put the page that were are transferring in an erasing state */
  if (Page_SetState(page, PAGE_STATE_REMOVED) != EE_OK)
  {
    goto error;
  }

  /* check that all element of a page exist */
  for (uint32_t address_read = page_address + EE_FLASH_PAGE_SIZE - EE_ELEMENT_SIZE;
       address_read >= (page_address + EE_PAGE_HEADER_SIZE);
       address_read = address_read - EE_ELEMENT_SIZE)
  {
    if (Frame_Get(address_read, &data) == EE_OK)
    {
      if (ReadVariable(data.frame_t.virtual_address, &data_tmp) == EE_INFO_NODATA)
      {
        status = Frame_Add(&data);
        if (status != EE_OK)
        {
          /* the function return an error */
          goto error;
        }
      }
    }
  }

  /* page set to erasing; ie ready to be erased */
  if (Page_SetState(page, PAGE_STATE_ERASING) != EE_OK)
  {
    goto error;
  }

  /* if next page is VALID set them has valid page */
  if (Page_GetState(FOLLOWING_PAGE(page)) == PAGE_STATE_VALID)
  {
    ee_internal_state.ublatestValidPage = FOLLOWING_PAGE(page);
  }
  else
  {
    ee_internal_state.ublatestValidPage = EE_NO_PAGE_FOUND;
  }

  /* decrease the number of element */
  ee_internal_state.uhNbWrittenElements -= EE_NB_MAX_ELEMENTS_BY_PAGE;

  status = EE_OK;
error:
  return status;
}

/**
  * @brief  get the frame data.
  * @param  address
  * @param  dataframe pointer to ee_frame_t data read from the flash.
  * @retval ee_status
  *           - EE_OK: on success
  *           - EE_INFO_NODATA: data at the address is not a valid data
  */
static ee_status Frame_Get(uint32_t address, ee_frame_t *dataframe)
{
  ee_status status = EE_INFO_NODATA;
  uint16_t crc_value;
  if (EE_ITF_FLASH_OK  == EE_ITF_FLASH_ReadData(address, (uint8_t *)dataframe->data_64_bits_t.data64,
                                                (uint16_t)(sizeof(ee_frame_t) / sizeof(uint8_t))))
  {
    /* Compare the read address with the virtual address */
    if (EE_ITF_CRC_Calcul16Bit((uint8_t *)&dataframe->frame_t.virtual_address, EE_FRAME_DATA_SIZE + 2U,
                               &crc_value) == EE_ITF_CRC_OK)
    {
      if (dataframe->frame_t.crc == crc_value)
      {
        status = EE_OK;
      }
    }
  }
  return status;
}

/**
  * @brief  Verify if pages are full then if not the case, writes variable in EEPROM.
  * @param  p_data pointer to ee_frame_t data to be written as variable value
  * @retval ee_status
  *           - EE_OK: on success
  *           - EE_ERROR_ALGO: if half pages are full
  *           - EE_FLASH_USED: flash currently used by CPU2
  *           - EEE_ERROR_ITF_FLASH: if an error occurs
  */
static ee_status Frame_Add(ee_frame_t *p_data)
{
  /* Get active Page for write operation, it will set a next page if the current page is full */
  uint32_t activepage = Page_GetWrite();
  uint32_t activepageaddress;

  /* Check if there is no active page */
  if (activepage == EE_NO_PAGE_FOUND)
  {
    return EE_ERROR_ALGO;
  }

  activepageaddress = GetPageAddress(activepage);

  if (EE_ITF_FLASH_WriteData(activepageaddress + ee_internal_state.uwAddressNextWrite,
                             (uint8_t *)p_data->data_64_bits_t.data64,
                             (uint16_t)(sizeof(ee_frame_t) / sizeof(uint8_t))) != EE_ITF_FLASH_OK)
  {
    return EE_ERROR_ITF_FLASH;
  }

  /* Increment global variables relative to write operation done*/
  ee_internal_state.uwAddressNextWrite += EE_ELEMENT_SIZE;
  ee_internal_state.uhNbWrittenElements++;
  return EE_OK;
}

/**
  * @brief  Set page state in the page header.
  * @param  page Index of the page
  * @param  state state of the page
  * @retval ee_status @ref ee_status
  */
static ee_status Page_SetState(uint32_t page, ee_page_state_t state)
{
  ee_frame_t dataFrame;
  ee_status status = EE_OK;
  uint32_t header_Address = GetPageAddress(page);

  switch (state)
  {
    case PAGE_STATE_ACTIVE:
    {
      /* Set new Page status to STATE_PAGE_RECEIVE status */
      SET_DATA_FRAME(dataFrame, EE_PAGESTAT_ACTIVE)

      if (EE_ITF_FLASH_WriteData(header_Address, (uint8_t *)dataFrame.data_64_bits_t.data64,
                                 (uint16_t)(sizeof(ee_frame_t) / sizeof(uint8_t))) != EE_ITF_FLASH_OK)
      {
        status = EE_ERROR_ITF_FLASH;
      }
      else
      {
        ee_internal_state.ubCurrentActivePage = page;
      }
      break;
    }
    case PAGE_STATE_VALID:
    {
      /* Set new Page status to PAGE_STATE_ACTIVE status */
      SET_DATA_FRAME(dataFrame, EE_PAGESTAT_VALID)
      header_Address += EE_ELEMENT_SIZE;

      if (EE_ITF_FLASH_WriteData(header_Address, (uint8_t *)dataFrame.data_64_bits_t.data64,
                                 (uint16_t)(sizeof(ee_frame_t) / sizeof(uint8_t))) != EE_ITF_FLASH_OK)
      {
        status = EE_ERROR_ITF_FLASH;
      }

      if (ee_internal_state.ublatestValidPage == EE_NO_PAGE_FOUND)
      {
        ee_internal_state.ublatestValidPage = page;
      }
      break;
    }
    case PAGE_STATE_REMOVED:
    {
      /* Set new Page status to PAGE_STATE_VALID status */
      SET_DATA_FRAME(dataFrame, EE_PAGESTAT_REMOVED)
      header_Address += 2U * EE_ELEMENT_SIZE;

      if (EE_ITF_FLASH_WriteData(header_Address, (uint8_t *)dataFrame.data_64_bits_t.data64,
                                 (uint16_t)(sizeof(ee_frame_t) / sizeof(uint8_t))) != EE_ITF_FLASH_OK)
      {
        status = EE_ERROR_ITF_FLASH;
      }
      break;
    }
    case PAGE_STATE_ERASING:
    {
      ee_internal_state.ubNbErasingPage++;
      /* Set new page status to PAGE_STATE_ERASING status */
      SET_DATA_FRAME(dataFrame, EE_PAGESTAT_ERASING);
      header_Address += 3U * EE_ELEMENT_SIZE;

      if (EE_ITF_FLASH_WriteData(header_Address, (uint8_t *)dataFrame.data_64_bits_t.data64,
                                 (uint16_t)(sizeof(ee_frame_t) / sizeof(uint8_t))) != EE_ITF_FLASH_OK)
      {
        status = EE_ERROR_ITF_FLASH;
      }
      break;
    }
    case PAGE_STATE_ERASED:
    {
      if (ee_internal_state.ubNbErasingPage > 0U) { ee_internal_state.ubNbErasingPage--; }

      if (EE_ITF_FLASH_PageErase(header_Address, 1U) != EE_ITF_FLASH_OK)
      {
        return EE_ERROR_ITF_FLASH;
      }
      break;
    }
    default:
    {
      status = EE_ERROR_ALGO;
      break;
    }
  }

  /*  get the complete view of the page state */
  ee_internal_state.Page_state[page - EE_START_PAGE] = state;

#ifdef EE_DEBUG
  EE_DEBUG_PRINT("___setPAGE[%d]=_%s_\n", page, ee_state_type_string[state]);
#endif /* EE_DEBUG */

  /* Return last operation flash status */
  return status;
}

/**
  * @brief Get the state of the page header for a given page number.
  * @param page page index of a given page
  * @return ee_page_state_t The state of the page
  */
static ee_page_state_t Page_GetState(uint32_t page)
{
  ee_page_state_t pageState;

  if (ee_internal_state.ubInitDone == 1U)
  {
    /* read state from internal state*/
    pageState = ee_internal_state.Page_state[page - EE_START_PAGE];
  }
  else
  {
    /* get the address of the page */
    ee_frame_t          FrameData_h;
    ee_itf_flash_status EccStatus;
    uint32_t address = GetPageAddress(page) + (3UL * sizeof(ee_frame_t));
    pageState = PAGE_STATE_ERASED;

    /* Get page state information from page header (4 first elements of the page) */
    /* It is important to perform read corresponding to data sizing for ECC aspect */
    EccStatus = EE_ITF_FLASH_ReadData(address, (uint8_t *)&FrameData_h,
                                      (uint16_t)((sizeof(ee_frame_t) / sizeof(uint8_t))));

    /* Return erasing status, if element4 is not EE_PAGESTAT_ERASED value */
    if ((!COMP_DATA_FRAME(FrameData_h,  EE_PAGESTAT_ERASED)) || (EccStatus == EE_ITF_FLASH_ERROR_ECCC))
    {
      pageState = PAGE_STATE_ERASING;
      goto end;
    }

    address -= sizeof(ee_frame_t);
    EccStatus = EE_ITF_FLASH_ReadData(address, (uint8_t *)&FrameData_h,
                                      (uint16_t)((sizeof(ee_frame_t) / sizeof(uint8_t))));

    /* Return removed status, if element3 is not EE_PAGESTAT_ERASED value */
    if ((!COMP_DATA_FRAME(FrameData_h,  EE_PAGESTAT_ERASED)) || (EccStatus == EE_ITF_FLASH_ERROR_ECCC))
    {
      pageState = PAGE_STATE_REMOVED;
      goto end;
    }

    address -= sizeof(ee_frame_t);
    EccStatus = EE_ITF_FLASH_ReadData(address, (uint8_t *)&FrameData_h,
                                      (uint16_t)((sizeof(ee_frame_t) / sizeof(uint8_t))));

    /* Return valid status, if element2 is not EE_PAGESTAT_ERASED value */
    if ((!COMP_DATA_FRAME(FrameData_h,  EE_PAGESTAT_ERASED)) || (EccStatus == EE_ITF_FLASH_ERROR_ECCC))
    {
      pageState = PAGE_STATE_VALID;
      goto end;
    }

    address -= sizeof(ee_frame_t);
    EccStatus = EE_ITF_FLASH_ReadData(address, (uint8_t *)&FrameData_h,
                                      (uint16_t)((sizeof(ee_frame_t) / sizeof(uint8_t))));

    /* Return active status, if element1 is not EE_PAGESTAT_ERASED value */
    if ((!COMP_DATA_FRAME(FrameData_h,  EE_PAGESTAT_ERASED)) || (EccStatus == EE_ITF_FLASH_ERROR_ECCC))
    {
      pageState = PAGE_STATE_ACTIVE;
      goto end;
    }
  }

end:
  /* Return erased status, if 4 first elements are EE_PAGESTAT_ERASED value */
  return pageState;
}

/**
  * @brief forward the status of interrupt operation.
  * @param operation_status operation status @ref ee_itf_flash_operation
  */
static void Operation(ee_itf_flash_operation operation_status)
{
  ee_operation_status app_status;

  /* report operation status */
  switch (operation_status)
  {
    case EE_ITF_FLASH_OPERATION_OK:
    {
      app_status = EE_OPERATION_COMPLETE;

      if (ee_internal_state.erase_it_ope == 1U)
      {
        /*  refresh the page state */
        for (uint8_t index = 0; index < ee_internal_state.erase_it_page_counter; index++)
        {
          ee_internal_state.Page_state[ee_internal_state.erase_it_first_page + index - EE_START_PAGE] \
            = PAGE_STATE_ERASED;
#ifdef EE_DEBUG
          EE_DEBUG_PRINT("___setPAGE[%d]=_%s_\n", first_page + index, ee_state_type_string[PAGE_STATE_ERASED]);
#endif /* EE_DEBUG */
        }
      }
      break;
    }
    /* case EE_ITF_FLASH_OPERATION_ERROR:*/
    default:
    {
      app_status = EE_OPERATION_ERROR;
      break;
    }
  }

  ee_internal_state.erase_it_ope = 0U;
  ee_internal_state.ope_status = app_status;
  if (ee_internal_state.operation_callback != NULL)
  {
    ee_internal_state.operation_callback(ee_internal_state.ope_status);
  }
}
/**
  * @}
  */

/**
  * @}
  */
