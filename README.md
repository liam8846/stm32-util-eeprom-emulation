# EEPROM Emulation Software Pack

![tag](https://img.shields.io/badge/tag-2.0.0-brightgreen.svg)
[![release note](https://img.shields.io/badge/release_note-view_html-gold.svg)](https://htmlpreview.github.io/?https://github.com/STMicroelectronics/stm32-util-eeprom-emulation/blob/main/Release_Notes.html)

## Overview

The EEPROM Emulation Utility Module provides a robust and efficient way to emulate EEPROM using Flash memory on STM32 microcontrollers.
It supports multiple use cases and ensures data integrity through CRC and ECC interfaces.

## Description and Usage

### Description

The EEPROM Emulation Utility is ideal for applications that require non-volatile storage but do not have dedicated EEPROM hardware.

The main features of EEPROM Emulation are:

- Emulation capacity is configurable with variable size and Flash region selection.
- Algorithm options include FLITF or NVM, aligned with device constraints and performance.
- Power-loss recovery restores FLITF pages and cleans up corrupted NVM addresses after reset.
- Data integrity options provide CRC for corruption detection and ECC (BCH) for bit-error correction.
- Interfaces are pluggable, Flash, CRC, and ECC drivers and templates are available.

### Usage

The start-up EEPROM Emulation can be split into two steps:

### Step 1: EEPROM Emulation initialization

Initialize EEPROM Emulation by calling **EE_Init(ee_object_t *object, ee_erase_type erase_type)**. The object parameter is an ee_object_t structure that represents the EEPROM Emulation object. This function performs the following steps:
  - Initialization of all the EEPROM Emulation interfaces, assuming that hardware peripherals are already initialized.
  - Restore the pages to a known state when using the FLITF algorithm.
  - Delete corrupted addresses after a reset operation when using the NVM algorithm.

### Step 2: Accessing EEPROM variables

 The core API provides functions to read and write variables of different sizes
 - Write variable using EE_WriteVariableXbits() functions.
 - Read variable using EE_ReadVariableXbits() functions.
