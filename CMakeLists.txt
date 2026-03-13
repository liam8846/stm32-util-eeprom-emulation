# origin-pack: STMicroelectronics::eeprom_emulation@2.0.0
# file-format: 1.0.0
project(STMicroelectronics_eeprom_emulation_2_0_0)
cmake_minimum_required(VERSION 3.20)
add_library(STMicroelectronics_eeprom_emulation_2_0_0 INTERFACE)

# List of all CMSIS properties that influence conditions for this package
if(NOT DEFINED CMSIS_Dname)
  set(CMSIS_Dname "CMSIS_Dname not set")
  message(DEBUG "CMSIS_Dname is not set to any value")
endif()

if(NOT DEFINED CMSIS_Dvendor)
  set(CMSIS_Dvendor "CMSIS_Dvendor not set")
  message(DEBUG "CMSIS_Dvendor is not set to any value")
endif()


# Device specific defined symbols








# Enable all components in this package
if(CMSIS_ENTIRE_STMicroelectronics_eeprom_emulation_2_0_0)
  list(APPEND CMSIS_COMPONENTS_LIST "Cvendor:STMicroelectronics#Cclass:Utility#Cgroup:eeprom emulation#Csub:algo#Cvariant:FLITF#Cversion:1.0.0")
  list(APPEND CMSIS_COMPONENTS_LIST "Cvendor:STMicroelectronics#Cclass:Utility#Cgroup:eeprom emulation#Csub:algo#Cvariant:NVM#Cversion:1.0.0")
  list(APPEND CMSIS_COMPONENTS_LIST "Cvendor:STMicroelectronics#Cclass:Utility#Cgroup:eeprom emulation#Csub:itf_flash#Cvariant:FLITF_EDATA#Cversion:1.0.0")
  list(APPEND CMSIS_COMPONENTS_LIST "Cvendor:STMicroelectronics#Cclass:Utility#Cgroup:eeprom emulation#Csub:itf_flash#Cvariant:NVM_EDATA#Cversion:1.0.0")
  list(APPEND CMSIS_COMPONENTS_LIST "Cvendor:STMicroelectronics#Cclass:Utility#Cgroup:eeprom emulation#Csub:itf_flash#Cvariant:FLASH template#Cversion:1.0.0")
  list(APPEND CMSIS_COMPONENTS_LIST "Cvendor:STMicroelectronics#Cclass:Utility#Cgroup:eeprom emulation#Csub:itf_ecc#Cvariant:ECC#Cversion:1.0.0")
  list(APPEND CMSIS_COMPONENTS_LIST "Cvendor:STMicroelectronics#Cclass:Utility#Cgroup:eeprom emulation#Csub:itf_ecc#Cvariant:ECC template#Cversion:1.0.0")
  list(APPEND CMSIS_COMPONENTS_LIST "Cvendor:STMicroelectronics#Cclass:Utility#Cgroup:eeprom emulation#Csub:itf_crc#Cvariant:CRC#Cversion:1.0.0")
  list(APPEND CMSIS_COMPONENTS_LIST "Cvendor:STMicroelectronics#Cclass:Utility#Cgroup:eeprom emulation#Csub:itf_crc#Cvariant:CRC template#Cversion:1.0.0")
  list(APPEND CMSIS_COMPONENTS_LIST "Cvendor:STMicroelectronics#Cclass:Utility#Cgroup:eeprom emulation#Csub:core#Cversion:1.0.0")
endif()

# All conditions used by this package

# condition: STMicroelectronics.eeprom_emulation.2.0.0
# description: 
if((CMSIS_Dname MATCHES "STM32.*" AND CMSIS_Dvendor STREQUAL "STMicroelectronics:13"))
  set(STMicroelectronics.eeprom_emulation.2.0.0 "1")
  message(DEBUG "CMSIS condition STMicroelectronics.eeprom_emulation.2.0.0 enabled")
else()
  set(STMicroelectronics.eeprom_emulation.2.0.0 0)
endif()


# condition: STMicroelectronics.eeprom_emulation.2.0.0:CRC_interface
# description: condition to include a CRC component
set(STMicroelectronics.eeprom_emulation.2.0.0_CRC_interface "$<OR:$<NOT:$<STREQUAL:$<FILTER:${CMSIS_COMPONENTS_LIST},INCLUDE,.*Cclass:Utility#.*Cgroup:eeprom emulation#.*Csub:itf_crc#.*Cvariant:CRC(#.*|$)>,>>,$<NOT:$<STREQUAL:$<FILTER:${CMSIS_COMPONENTS_LIST},INCLUDE,.*Cclass:Utility#.*Cgroup:eeprom emulation#.*Csub:itf_crc#.*Cvariant:CRC template(#.*|$)>,>>>")
message(DEBUG "CMSIS condition STMicroelectronics.eeprom_emulation.2.0.0_CRC_interface enabled")


# condition: STMicroelectronics.eeprom_emulation.2.0.0:Core
# description: 
set(STMicroelectronics.eeprom_emulation.2.0.0_Core "$<OR:$<AND:$<OR:$<NOT:$<STREQUAL:$<FILTER:${CMSIS_COMPONENTS_LIST},INCLUDE,.*Cclass:Utility#.*Cgroup:eeprom emulation#.*Csub:itf_crc#.*Cvariant:CRC(#.*|$)>,>>,$<NOT:$<STREQUAL:$<FILTER:${CMSIS_COMPONENTS_LIST},INCLUDE,.*Cclass:Utility#.*Cgroup:eeprom emulation#.*Csub:itf_crc#.*Cvariant:CRC template(#.*|$)>,>>>,$<OR:$<NOT:$<STREQUAL:$<FILTER:${CMSIS_COMPONENTS_LIST},INCLUDE,.*Cclass:Utility#.*Cgroup:eeprom emulation#.*Csub:itf_flash#.*Cvariant:FLITF_EDATA(#.*|$)>,>>,$<NOT:$<STREQUAL:$<FILTER:${CMSIS_COMPONENTS_LIST},INCLUDE,.*Cclass:Utility#.*Cgroup:eeprom emulation#.*Csub:itf_flash#.*Cvariant:FLASH template(#.*|$)>,>>>,$<NOT:$<STREQUAL:$<FILTER:${CMSIS_COMPONENTS_LIST},INCLUDE,.*Cclass:Utility#.*Cgroup:eeprom emulation#.*Csub:algo#.*Cvariant:FLITF(#.*|$)>,>>>,$<AND:$<OR:$<NOT:$<STREQUAL:$<FILTER:${CMSIS_COMPONENTS_LIST},INCLUDE,.*Cclass:Utility#.*Cgroup:eeprom emulation#.*Csub:itf_crc#.*Cvariant:CRC(#.*|$)>,>>,$<NOT:$<STREQUAL:$<FILTER:${CMSIS_COMPONENTS_LIST},INCLUDE,.*Cclass:Utility#.*Cgroup:eeprom emulation#.*Csub:itf_crc#.*Cvariant:CRC template(#.*|$)>,>>>,$<OR:$<NOT:$<STREQUAL:$<FILTER:${CMSIS_COMPONENTS_LIST},INCLUDE,.*Cclass:Utility#.*Cgroup:eeprom emulation#.*Csub:itf_ecc#.*Cvariant:ECC(#.*|$)>,>>,$<NOT:$<STREQUAL:$<FILTER:${CMSIS_COMPONENTS_LIST},INCLUDE,.*Cclass:Utility#.*Cgroup:eeprom emulation#.*Csub:itf_ecc#.*Cvariant:ECC template(#.*|$)>,>>>,$<OR:$<NOT:$<STREQUAL:$<FILTER:${CMSIS_COMPONENTS_LIST},INCLUDE,.*Cclass:Utility#.*Cgroup:eeprom emulation#.*Csub:itf_flash#.*Cvariant:NVM_EDATA(#.*|$)>,>>,$<NOT:$<STREQUAL:$<FILTER:${CMSIS_COMPONENTS_LIST},INCLUDE,.*Cclass:Utility#.*Cgroup:eeprom emulation#.*Csub:itf_flash#.*Cvariant:FLASH template(#.*|$)>,>>>,$<NOT:$<STREQUAL:$<FILTER:${CMSIS_COMPONENTS_LIST},INCLUDE,.*Cclass:Utility#.*Cgroup:eeprom emulation#.*Csub:algo#.*Cvariant:NVM(#.*|$)>,>>>>")
message(DEBUG "CMSIS condition STMicroelectronics.eeprom_emulation.2.0.0_Core enabled")


# condition: STMicroelectronics.eeprom_emulation.2.0.0:ECC_interface
# description: condition to include a CRC component
set(STMicroelectronics.eeprom_emulation.2.0.0_ECC_interface "$<OR:$<NOT:$<STREQUAL:$<FILTER:${CMSIS_COMPONENTS_LIST},INCLUDE,.*Cclass:Utility#.*Cgroup:eeprom emulation#.*Csub:itf_ecc#.*Cvariant:ECC(#.*|$)>,>>,$<NOT:$<STREQUAL:$<FILTER:${CMSIS_COMPONENTS_LIST},INCLUDE,.*Cclass:Utility#.*Cgroup:eeprom emulation#.*Csub:itf_ecc#.*Cvariant:ECC template(#.*|$)>,>>>")
message(DEBUG "CMSIS condition STMicroelectronics.eeprom_emulation.2.0.0_ECC_interface enabled")


# condition: STMicroelectronics.eeprom_emulation.2.0.0:FLASH FLITF
# description: EEPROM_Emulation required package for FLITF
set(STMicroelectronics.eeprom_emulation.2.0.0_FLASH_FLITF "$<AND:$<OR:$<NOT:$<STREQUAL:$<FILTER:${CMSIS_COMPONENTS_LIST},INCLUDE,.*Cclass:Utility#.*Cgroup:eeprom emulation#.*Csub:itf_crc#.*Cvariant:CRC(#.*|$)>,>>,$<NOT:$<STREQUAL:$<FILTER:${CMSIS_COMPONENTS_LIST},INCLUDE,.*Cclass:Utility#.*Cgroup:eeprom emulation#.*Csub:itf_crc#.*Cvariant:CRC template(#.*|$)>,>>>,$<OR:$<NOT:$<STREQUAL:$<FILTER:${CMSIS_COMPONENTS_LIST},INCLUDE,.*Cclass:Utility#.*Cgroup:eeprom emulation#.*Csub:itf_flash#.*Cvariant:FLITF_EDATA(#.*|$)>,>>,$<NOT:$<STREQUAL:$<FILTER:${CMSIS_COMPONENTS_LIST},INCLUDE,.*Cclass:Utility#.*Cgroup:eeprom emulation#.*Csub:itf_flash#.*Cvariant:FLASH template(#.*|$)>,>>>,$<NOT:$<STREQUAL:$<FILTER:${CMSIS_COMPONENTS_LIST},INCLUDE,.*Cclass:Utility#.*Cgroup:eeprom emulation#.*Csub:algo#.*Cvariant:FLITF(#.*|$)>,>>>")
message(DEBUG "CMSIS condition STMicroelectronics.eeprom_emulation.2.0.0_FLASH_FLITF enabled")


# condition: STMicroelectronics.eeprom_emulation.2.0.0:FLASH NVM
# description: EEPROM_Emulation required package for NVM
set(STMicroelectronics.eeprom_emulation.2.0.0_FLASH_NVM "$<AND:$<OR:$<NOT:$<STREQUAL:$<FILTER:${CMSIS_COMPONENTS_LIST},INCLUDE,.*Cclass:Utility#.*Cgroup:eeprom emulation#.*Csub:itf_crc#.*Cvariant:CRC(#.*|$)>,>>,$<NOT:$<STREQUAL:$<FILTER:${CMSIS_COMPONENTS_LIST},INCLUDE,.*Cclass:Utility#.*Cgroup:eeprom emulation#.*Csub:itf_crc#.*Cvariant:CRC template(#.*|$)>,>>>,$<OR:$<NOT:$<STREQUAL:$<FILTER:${CMSIS_COMPONENTS_LIST},INCLUDE,.*Cclass:Utility#.*Cgroup:eeprom emulation#.*Csub:itf_ecc#.*Cvariant:ECC(#.*|$)>,>>,$<NOT:$<STREQUAL:$<FILTER:${CMSIS_COMPONENTS_LIST},INCLUDE,.*Cclass:Utility#.*Cgroup:eeprom emulation#.*Csub:itf_ecc#.*Cvariant:ECC template(#.*|$)>,>>>,$<OR:$<NOT:$<STREQUAL:$<FILTER:${CMSIS_COMPONENTS_LIST},INCLUDE,.*Cclass:Utility#.*Cgroup:eeprom emulation#.*Csub:itf_flash#.*Cvariant:NVM_EDATA(#.*|$)>,>>,$<NOT:$<STREQUAL:$<FILTER:${CMSIS_COMPONENTS_LIST},INCLUDE,.*Cclass:Utility#.*Cgroup:eeprom emulation#.*Csub:itf_flash#.*Cvariant:FLASH template(#.*|$)>,>>>,$<NOT:$<STREQUAL:$<FILTER:${CMSIS_COMPONENTS_LIST},INCLUDE,.*Cclass:Utility#.*Cgroup:eeprom emulation#.*Csub:algo#.*Cvariant:NVM(#.*|$)>,>>>")
message(DEBUG "CMSIS condition STMicroelectronics.eeprom_emulation.2.0.0_FLASH_NVM enabled")


# condition: STMicroelectronics.eeprom_emulation.2.0.0:FLITF_interface
# description: condition to include a flitf interface component
set(STMicroelectronics.eeprom_emulation.2.0.0_FLITF_interface "$<OR:$<NOT:$<STREQUAL:$<FILTER:${CMSIS_COMPONENTS_LIST},INCLUDE,.*Cclass:Utility#.*Cgroup:eeprom emulation#.*Csub:itf_flash#.*Cvariant:FLITF_EDATA(#.*|$)>,>>,$<NOT:$<STREQUAL:$<FILTER:${CMSIS_COMPONENTS_LIST},INCLUDE,.*Cclass:Utility#.*Cgroup:eeprom emulation#.*Csub:itf_flash#.*Cvariant:FLASH template(#.*|$)>,>>>")
message(DEBUG "CMSIS condition STMicroelectronics.eeprom_emulation.2.0.0_FLITF_interface enabled")


# condition: STMicroelectronics.eeprom_emulation.2.0.0:NVM_interface
# description: condition to include a nvm interface component
set(STMicroelectronics.eeprom_emulation.2.0.0_NVM_interface "$<OR:$<NOT:$<STREQUAL:$<FILTER:${CMSIS_COMPONENTS_LIST},INCLUDE,.*Cclass:Utility#.*Cgroup:eeprom emulation#.*Csub:itf_flash#.*Cvariant:NVM_EDATA(#.*|$)>,>>,$<NOT:$<STREQUAL:$<FILTER:${CMSIS_COMPONENTS_LIST},INCLUDE,.*Cclass:Utility#.*Cgroup:eeprom emulation#.*Csub:itf_flash#.*Cvariant:FLASH template(#.*|$)>,>>>")
message(DEBUG "CMSIS condition STMicroelectronics.eeprom_emulation.2.0.0_NVM_interface enabled")


# condition: STMicroelectronics.eeprom_emulation.2.0.0:STM32 HAL CRC
# description: STMicroelectronics CRC HAL Driver as low level component
set(STMicroelectronics.eeprom_emulation.2.0.0_STM32_HAL_CRC "$<NOT:$<STREQUAL:$<FILTER:${CMSIS_COMPONENTS_LIST},INCLUDE,.*Cclass:Device#.*Cgroup:STM32 HAL#.*Csub:CRC(#.*|$)>,>>")
message(DEBUG "CMSIS condition STMicroelectronics.eeprom_emulation.2.0.0_STM32_HAL_CRC enabled")


# condition: STMicroelectronics.eeprom_emulation.2.0.0:STM32 HAL FLASH
# description: STMicroelectronics FLASH HAL Driver as low level component
set(STMicroelectronics.eeprom_emulation.2.0.0_STM32_HAL_FLASH "$<AND:$<NOT:$<STREQUAL:$<FILTER:${CMSIS_COMPONENTS_LIST},INCLUDE,.*Cclass:Device#.*Cgroup:STM32 HAL#.*Csub:FLASH(#.*|$)>,>>,$<NOT:$<STREQUAL:$<FILTER:${CMSIS_COMPONENTS_LIST},INCLUDE,.*Cclass:Device#.*Cgroup:STM32 HAL#.*Csub:FLASH_ITF(#.*|$)>,>>>")
message(DEBUG "CMSIS condition STMicroelectronics.eeprom_emulation.2.0.0_STM32_HAL_FLASH enabled")


# condition: STMicroelectronics.eeprom_emulation.2.0.0:STM32 HAL NVM
# description: STMicroelectronics FLASH HAL Driver as low level component
set(STMicroelectronics.eeprom_emulation.2.0.0_STM32_HAL_NVM "$<AND:$<NOT:$<STREQUAL:$<FILTER:${CMSIS_COMPONENTS_LIST},INCLUDE,.*Cclass:Device#.*Cgroup:STM32 HAL#.*Csub:NVM(#.*|$)>,>>,$<NOT:$<STREQUAL:$<FILTER:${CMSIS_COMPONENTS_LIST},INCLUDE,.*Cclass:Device#.*Cgroup:STM32 HAL#.*Csub:NVM_ITF(#.*|$)>,>>>")
message(DEBUG "CMSIS condition STMicroelectronics.eeprom_emulation.2.0.0_STM32_HAL_NVM enabled")

# Files and components in this package
if("Cvendor:STMicroelectronics#Cclass:Utility#Cgroup:eeprom emulation#Csub:algo#Cvariant:FLITF#Cversion:1.0.0" IN_LIST CMSIS_COMPONENTS_LIST)  # EEPROM_Emulation flitf algorithm
  message(DEBUG "Using component Utility_eeprom_emulation_algo_FLITF_1_0_0")
  target_compile_definitions(STMicroelectronics_eeprom_emulation_2_0_0 INTERFACE "-DCMSIS_USE_Utility_eeprom_emulation_algo_FLITF_1_0_0=1")
  target_include_directories(STMicroelectronics_eeprom_emulation_2_0_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/core/algo")
  target_sources(STMicroelectronics_eeprom_emulation_2_0_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/core/algo/eeprom_algo_flitf.c")
endif()

if("Cvendor:STMicroelectronics#Cclass:Utility#Cgroup:eeprom emulation#Csub:algo#Cvariant:NVM#Cversion:1.0.0" IN_LIST CMSIS_COMPONENTS_LIST)  # EEPROM_Emulation nvm algorithm
  message(DEBUG "Using component Utility_eeprom_emulation_algo_NVM_1_0_0")
  target_compile_definitions(STMicroelectronics_eeprom_emulation_2_0_0 INTERFACE "-DCMSIS_USE_Utility_eeprom_emulation_algo_NVM_1_0_0=1")
  target_include_directories(STMicroelectronics_eeprom_emulation_2_0_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/core/algo")
  target_sources(STMicroelectronics_eeprom_emulation_2_0_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/core/algo/eeprom_algo_nvm.c")
endif()

if("Cvendor:STMicroelectronics#Cclass:Utility#Cgroup:eeprom emulation#Csub:itf_flash#Cvariant:FLITF_EDATA#Cversion:1.0.0" IN_LIST CMSIS_COMPONENTS_LIST)  # EEPROM_Emulation flash flitf edata interface
  message(DEBUG "Using component Utility_eeprom_emulation_itf_flash_FLITF_EDATA_1_0_0")
  target_compile_definitions(STMicroelectronics_eeprom_emulation_2_0_0 INTERFACE "$<${STMicroelectronics.eeprom_emulation.2.0.0_STM32_HAL_FLASH}:-DCMSIS_USE_Utility_eeprom_emulation_itf_flash_FLITF_EDATA_1_0_0=1>")
  target_include_directories(STMicroelectronics_eeprom_emulation_2_0_0 INTERFACE "$<${STMicroelectronics.eeprom_emulation.2.0.0_STM32_HAL_FLASH}:${CMAKE_CURRENT_LIST_DIR}/interface_flash>")
  target_sources(STMicroelectronics_eeprom_emulation_2_0_0 INTERFACE "$<${STMicroelectronics.eeprom_emulation.2.0.0_STM32_HAL_FLASH}:${CMAKE_CURRENT_LIST_DIR}/interface_flash/eeprom_itfflash_flitf_edata.c>")
endif()

if("Cvendor:STMicroelectronics#Cclass:Utility#Cgroup:eeprom emulation#Csub:itf_flash#Cvariant:NVM_EDATA#Cversion:1.0.0" IN_LIST CMSIS_COMPONENTS_LIST)  # EEPROM_Emulation flash nvm edata interface
  message(DEBUG "Using component Utility_eeprom_emulation_itf_flash_NVM_EDATA_1_0_0")
  target_compile_definitions(STMicroelectronics_eeprom_emulation_2_0_0 INTERFACE "$<${STMicroelectronics.eeprom_emulation.2.0.0_STM32_HAL_NVM}:-DCMSIS_USE_Utility_eeprom_emulation_itf_flash_NVM_EDATA_1_0_0=1>")
  target_include_directories(STMicroelectronics_eeprom_emulation_2_0_0 INTERFACE "$<${STMicroelectronics.eeprom_emulation.2.0.0_STM32_HAL_NVM}:${CMAKE_CURRENT_LIST_DIR}/interface_flash>")
  target_sources(STMicroelectronics_eeprom_emulation_2_0_0 INTERFACE "$<${STMicroelectronics.eeprom_emulation.2.0.0_STM32_HAL_NVM}:${CMAKE_CURRENT_LIST_DIR}/interface_flash/eeprom_itfflash_nvm_edata.c>")
endif()

if("Cvendor:STMicroelectronics#Cclass:Utility#Cgroup:eeprom emulation#Csub:itf_flash#Cvariant:FLASH template#Cversion:1.0.0" IN_LIST CMSIS_COMPONENTS_LIST)  # EEPROM_Emulation flash template interface
  message(DEBUG "Using component Utility_eeprom_emulation_itf_flash_FLASH_template_1_0_0")
  target_compile_definitions(STMicroelectronics_eeprom_emulation_2_0_0 INTERFACE "-DCMSIS_USE_Utility_eeprom_emulation_itf_flash_FLASH_template_1_0_0=1")
  target_include_directories(STMicroelectronics_eeprom_emulation_2_0_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/interface_flash")
  target_sources(STMicroelectronics_eeprom_emulation_2_0_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/interface_flash/eeprom_itfflash_template.c")
endif()

if("Cvendor:STMicroelectronics#Cclass:Utility#Cgroup:eeprom emulation#Csub:itf_ecc#Cvariant:ECC#Cversion:1.0.0" IN_LIST CMSIS_COMPONENTS_LIST)  # EEPROM_Emulation ECC interface
  message(DEBUG "Using component Utility_eeprom_emulation_itf_ecc_ECC_1_0_0")
  target_compile_definitions(STMicroelectronics_eeprom_emulation_2_0_0 INTERFACE "-DCMSIS_USE_Utility_eeprom_emulation_itf_ecc_ECC_1_0_0=1")
  target_include_directories(STMicroelectronics_eeprom_emulation_2_0_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/interface_ecc")
  target_sources(STMicroelectronics_eeprom_emulation_2_0_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/interface_ecc/eeprom_itfecc_bch.c")
endif()

if("Cvendor:STMicroelectronics#Cclass:Utility#Cgroup:eeprom emulation#Csub:itf_ecc#Cvariant:ECC template#Cversion:1.0.0" IN_LIST CMSIS_COMPONENTS_LIST)  # EEPROM_Emulation ECC template interface
  message(DEBUG "Using component Utility_eeprom_emulation_itf_ecc_ECC_template_1_0_0")
  target_compile_definitions(STMicroelectronics_eeprom_emulation_2_0_0 INTERFACE "-DCMSIS_USE_Utility_eeprom_emulation_itf_ecc_ECC_template_1_0_0=1")
  target_include_directories(STMicroelectronics_eeprom_emulation_2_0_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/interface_ecc")
  target_sources(STMicroelectronics_eeprom_emulation_2_0_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/interface_ecc/eeprom_itfecc_template.c")
endif()

if("Cvendor:STMicroelectronics#Cclass:Utility#Cgroup:eeprom emulation#Csub:itf_crc#Cvariant:CRC#Cversion:1.0.0" IN_LIST CMSIS_COMPONENTS_LIST)  # EEPROM_Emulation CRC interface
  message(DEBUG "Using component Utility_eeprom_emulation_itf_crc_CRC_1_0_0")
  target_compile_definitions(STMicroelectronics_eeprom_emulation_2_0_0 INTERFACE "$<${STMicroelectronics.eeprom_emulation.2.0.0_STM32_HAL_CRC}:-DCMSIS_USE_Utility_eeprom_emulation_itf_crc_CRC_1_0_0=1>")
  target_include_directories(STMicroelectronics_eeprom_emulation_2_0_0 INTERFACE "$<${STMicroelectronics.eeprom_emulation.2.0.0_STM32_HAL_CRC}:${CMAKE_CURRENT_LIST_DIR}/interface_crc>")
  target_sources(STMicroelectronics_eeprom_emulation_2_0_0 INTERFACE "$<${STMicroelectronics.eeprom_emulation.2.0.0_STM32_HAL_CRC}:${CMAKE_CURRENT_LIST_DIR}/interface_crc/eeprom_itfcrc_crc.c>")
endif()

if("Cvendor:STMicroelectronics#Cclass:Utility#Cgroup:eeprom emulation#Csub:itf_crc#Cvariant:CRC template#Cversion:1.0.0" IN_LIST CMSIS_COMPONENTS_LIST)  # EEPROM_Emulation CRC template interface
  message(DEBUG "Using component Utility_eeprom_emulation_itf_crc_CRC_template_1_0_0")
  target_compile_definitions(STMicroelectronics_eeprom_emulation_2_0_0 INTERFACE "-DCMSIS_USE_Utility_eeprom_emulation_itf_crc_CRC_template_1_0_0=1")
  target_include_directories(STMicroelectronics_eeprom_emulation_2_0_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/interface_crc")
  target_sources(STMicroelectronics_eeprom_emulation_2_0_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/interface_crc/eeprom_itfcrc_template.c")
endif()

if("Cvendor:STMicroelectronics#Cclass:Utility#Cgroup:eeprom emulation#Csub:core#Cversion:1.0.0" IN_LIST CMSIS_COMPONENTS_LIST)  # EEPROM_Emulation core module
  message(DEBUG "Using component Utility_eeprom_emulation_core_1_0_0")
  target_compile_definitions(STMicroelectronics_eeprom_emulation_2_0_0 INTERFACE "$<${STMicroelectronics.eeprom_emulation.2.0.0_Core}:-DCMSIS_USE_Utility_eeprom_emulation_core_1_0_0=1>")
  target_include_directories(STMicroelectronics_eeprom_emulation_2_0_0 INTERFACE "$<${STMicroelectronics.eeprom_emulation.2.0.0_Core}:${CMAKE_CURRENT_LIST_DIR}/core>")
  target_sources(STMicroelectronics_eeprom_emulation_2_0_0 INTERFACE "$<${STMicroelectronics.eeprom_emulation.2.0.0_Core}:${CMAKE_CURRENT_LIST_DIR}/core/eeprom_emul_core.c>")
endif()

