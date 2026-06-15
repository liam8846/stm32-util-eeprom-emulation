/**
  ******************************************************************************
  * @file           : eeprom_emulation_helpers.js
  * @brief          : Helper functions for EEPROM Emulation MX2 configuration.
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

const EE_NVM_MAX_CYCLES = 15;

/**
 * Parses a hexadecimal string without prefix.
 *
 * @param {string} value Hexadecimal string to parse.
 * @returns {number|undefined} Parsed integer value, or undefined when the input is not a valid hexadecimal string.
 */
function ee_parseHexString(value) {
  const normalizedValue = String(value ?? "");

  if (!/^[0-9a-fA-F]+$/.test(normalizedValue)) {
    return undefined;
  }

  return Number.parseInt(normalizedValue, 16);
}

/**
 * Parses a hexadecimal address string with 0x prefix.
 *
 * @param {string} value Address string to parse.
 * @returns {number|undefined} Parsed integer address, or undefined when the input is not a valid hexadecimal address.
 */
function ee_parseAddress(value) {
  const normalizedValue = String(value ?? "");

  if (!/^0x[0-9a-fA-F]+$/.test(normalizedValue)) {
    return undefined;
  }

  return Number.parseInt(normalizedValue, 16);
}

/**
 * Formats an integer value as an uppercase hexadecimal address.
 *
 * @param {number} value Value to format.
 * @returns {string} Address formatted as 0x-prefixed uppercase hexadecimal, or an empty string for unsupported input.
 */
function ee_toHexAddress(value) {
  return Number.isInteger(value) ? `0x${value.toString(16).toUpperCase()}` : "";
}

/**
 * Normalizes the raw hardware description shape used by MX2 helpers.
 *
 * Accepts direct hardware descriptions, wrapped descriptions, or arrays of
 * candidate descriptions and returns the first entry exposing NVM
 * sectorization data.
 *
 * @param {object|Array<object>} hardwareDescription Raw hardware description candidate.
 * @returns {object} Normalized hardware description object, or an empty object when no compatible shape is found.
 */
function ee_normalizeHardwareDescription(hardwareDescription) {
  if (!hardwareDescription || typeof hardwareDescription !== "object") {
    return {};
  }

  if (hardwareDescription.features?.__extra_info?.nvm_sectorization?.banks) {
    return hardwareDescription;
  }

  if (hardwareDescription.hardwareDescription?.features?.__extra_info?.nvm_sectorization?.banks) {
    return hardwareDescription.hardwareDescription;
  }

  if (Array.isArray(hardwareDescription)) {
    return hardwareDescription.find(entry => entry?.features?.__extra_info?.nvm_sectorization?.banks) ?? {};
  }

  return hardwareDescription;
}

/**
 * Extracts the effective security mode from the engine security value.
 *
 * @param {string|object} engineSecureValue Security context passed by the configuration engine.
 * @returns {string|undefined} Resolved security mode string, or undefined when unavailable.
 */
function ee_getEngineSecureValue(engineSecureValue) {
  if (typeof engineSecureValue === "string") {
    return engineSecureValue;
  }

  if (engineSecureValue && typeof engineSecureValue === "object") {
    return engineSecureValue.secure;
  }

  return undefined;
}

/**
 * Checks whether secure user-mode addresses should be preferred.
 *
 * @param {string|object} engineSecureValue Security context passed by the configuration engine.
 * @returns {boolean} True when secure or secure-only addresses must be used first.
 */
function ee_shouldUseSecureAddresses(engineSecureValue) {
  const normalizedEngineSecureValue = ee_getEngineSecureValue(engineSecureValue);

  return normalizedEngineSecureValue === "Secure" || normalizedEngineSecureValue === "Secure-only";
}

/**
 * Extracts secure and non-secure user-mode address ranges from a sub-area.
 *
 * @param {object} subArea NVM sub-area description.
 * @returns {{secure: ({start:number,end:number}|undefined), nonSecure: ({start:number,end:number}|undefined)}} Available secure and non-secure user-mode ranges.
 */
function ee_getUserModeRanges(subArea) {
  const secureStartAddress = ee_parseHexString(subArea?.user_mode?.s_start_address);
  const secureEndAddress = ee_parseHexString(subArea?.user_mode?.s_end_address);
  const nonSecureStartAddress = ee_parseHexString(subArea?.user_mode?.ns_start_address);
  const nonSecureEndAddress = ee_parseHexString(subArea?.user_mode?.ns_end_address);

  return {
    secure: Number.isInteger(secureStartAddress)
      && Number.isInteger(secureEndAddress)
      && secureStartAddress <= secureEndAddress
      ? {
        start: secureStartAddress,
        end: secureEndAddress
      }
      : undefined,
    nonSecure: Number.isInteger(nonSecureStartAddress)
      && Number.isInteger(nonSecureEndAddress)
      && nonSecureStartAddress <= nonSecureEndAddress
      ? {
        start: nonSecureStartAddress,
        end: nonSecureEndAddress
      }
      : undefined
  };
}

/**
 * Returns the NVM bank list from a hardware description.
 *
 * @param {object|Array<object>} hardwareDescription Raw or normalized hardware description.
 * @returns {Array<object>} NVM banks, or an empty array when no sectorization data is available.
 */
function ee_getNvmBanks(hardwareDescription) {
  const normalizedHardwareDescription = ee_normalizeHardwareDescription(hardwareDescription);

  return normalizedHardwareDescription?.features?.__extra_info?.nvm_sectorization?.banks ?? [];
}

/**
 * Resolves the active hardware description from the selected flash backend.
 *
 * @param {string} flashType Selected flash backend, typically NVM or FLITF.
 * @param {object} nvmHardwareDescription Candidate NVM hardware description.
 * @param {object} flashHardwareDescription Candidate FLASH hardware description.
 * @returns {object|undefined} Selected hardware description used for subsequent flash-area calculations.
 */
function ee_selectHardwareDescription(flashType, nvmHardwareDescription, flashHardwareDescription) {
  if (flashType === "NVM") {
    return nvmHardwareDescription;
  }

  if (flashType === "FLITF") {
    return flashHardwareDescription;
  }

  if (ee_getNvmBanks(nvmHardwareDescription).length > 0) {
    return nvmHardwareDescription;
  }

  if (ee_getNvmBanks(flashHardwareDescription).length > 0) {
    return flashHardwareDescription;
  }

  return nvmHardwareDescription ?? flashHardwareDescription;
}

/**
 * Resolves the active hardware description from the selected flash backend stored in basic.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {object} nvmHardwareDescription Candidate NVM hardware description.
 * @param {object} flashHardwareDescription Candidate FLASH hardware description.
 * @returns {object|undefined} Selected hardware description used for subsequent flash-area calculations.
 */
function ee_getSelectedHardwareDescriptionForBasic(basic, nvmHardwareDescription, flashHardwareDescription) {
  return ee_selectHardwareDescription(basic?.flash_type, nvmHardwareDescription, flashHardwareDescription);
}

/**
 * Returns the maximum allowed cycle count for the selected flash backend.
 *
 * @param {string} flashType Selected flash backend, typically NVM or FLITF.
 * @returns {number|undefined} Maximum cycle count for the selected backend, or undefined when no fixed backend cap applies.
 */
function ee_getMaximumCyclesForFlashType(flashType) {
  return flashType === "NVM" ? EE_NVM_MAX_CYCLES : undefined;
}

/**
 * Returns all sub-areas declared in a bank.
 *
 * @param {object} bank NVM bank description.
 * @returns {Array<object>} Flattened list of bank sub-areas.
 */
function ee_getBankSubAreas(bank) {
  return (bank?.areas ?? []).flatMap(area => area?.sub_areas ?? []);
}

/**
 * Checks whether a sub-area belongs to an EDATA-backend region.
 *
 * @param {object} subArea NVM sub-area description.
 * @returns {boolean} True when the sub-area is marked as EDATA through the area_data flag.
 */
function ee_isEdataSubArea(subArea) {
  return Number(subArea?.nvm?.area_data ?? 0) === 1;
}

/**
 * Checks whether a sub-area matches the legacy single-block EDATA layout.
 *
 * @param {object} subArea NVM sub-area description.
 * @returns {boolean} True when the sub-area is the legacy EDATA entry.
 */
function ee_isLegacyEdataSubArea(subArea) {
  return ee_isEdataSubArea(subArea)
    && typeof subArea?.name === "string"
    && subArea.name === "EDATA";
}

/**
 * Checks whether a sub-area matches the page-based column EDATA layout.
 *
 * This identifies the newer D44F-style EDATA representation exposed as
 * per-page column sub-areas.
 *
 * @param {object} subArea NVM sub-area description.
 * @returns {boolean} True when the sub-area is a column EDATA page entry.
 */
function ee_isEdataColumnPageSubArea(subArea) {
  const pageNumber = Number(subArea?.page_number);

  return ee_isEdataSubArea(subArea)
    && subArea?.addressing_mode === "column"
    && typeof subArea?.name === "string"
    && subArea.name.startsWith("EDATA Page")
    && Number.isInteger(pageNumber)
    && pageNumber >= 0;
}

/**
 * Checks whether a sub-area matches the page-based row EDATA layout.
 *
 * This identifies the D499-style NVM EDATA representation exposed as
 * per-page row sub-areas.
 *
 * @param {object} subArea NVM sub-area description.
 * @returns {boolean} True when the sub-area is a row EDATA page entry.
 */
function ee_isEdataRowPageSubArea(subArea) {
  const pageNumber = Number(subArea?.page_number);

  return ee_isEdataSubArea(subArea)
    && subArea?.addressing_mode === "row"
    && typeof subArea?.name === "string"
    && subArea.name.startsWith("EDATA Page")
    && Number.isInteger(pageNumber)
    && pageNumber >= 0;
}

/**
 * Returns the number of pages described by an address range.
 *
 * @param {{start:number,end:number}} range Address range to evaluate.
 * @param {number} pageSizeBytes Page size in bytes.
 * @returns {number|undefined} Page count, or undefined when the range is not page-aligned.
 */
function ee_getRangePageCount(range, pageSizeBytes) {
  if (!range || !Number.isInteger(pageSizeBytes) || pageSizeBytes <= 0) {
    return undefined;
  }

  const rangeLength = (range.end - range.start) + 1;
  if (rangeLength <= 0 || (rangeLength % pageSizeBytes) !== 0) {
    return undefined;
  }

  return rangeLength / pageSizeBytes;
}

/**
 * Extracts normalized geometry information from a legacy EDATA bank.
 *
 * @param {object} bank NVM bank description.
 * @returns {object|undefined} Normalized legacy EDATA bank geometry, or undefined when the bank does not expose a valid legacy EDATA block.
 */
function ee_getLegacyEdataBankInfo(bank) {
  const legacyEdataSubArea = ee_getBankSubAreas(bank).find(ee_isLegacyEdataSubArea);
  if (!legacyEdataSubArea) {
    return undefined;
  }

  const userModeRanges = ee_getUserModeRanges(legacyEdataSubArea);
  const pageSizeBytes = Number(legacyEdataSubArea?.sub_area_size_B ?? 0);
  const busSizeBytes = Number(legacyEdataSubArea?.bus_size_b ?? 0);
  const pageCount = ee_getRangePageCount(userModeRanges.nonSecure, pageSizeBytes)
    ?? ee_getRangePageCount(userModeRanges.secure, pageSizeBytes);

  if (!Number.isInteger(pageSizeBytes)
      || pageSizeBytes <= 0
      || !Number.isInteger(busSizeBytes)
      || busSizeBytes <= 0
      || !Number.isInteger(pageCount)
      || pageCount <= 0) {
    return undefined;
  }

  return {
    pageSizeBytes,
    busSizeBytes,
    nonSecureStartAddress: userModeRanges.nonSecure?.start,
    nonSecureEndAddress: userModeRanges.nonSecure?.end,
    secureStartAddress: userModeRanges.secure?.start,
    secureEndAddress: userModeRanges.secure?.end
  };
}

/**
 * Extracts normalized geometry information from a page-based EDATA bank.
 *
 * @param {object} bank NVM bank description.
 * @param {(subArea: object) => boolean} pageMatcher Predicate selecting the relevant EDATA pages.
 * @returns {object|undefined} Normalized EDATA bank geometry, or undefined when the bank does not expose valid matching EDATA pages.
 */
function ee_getPagedEdataBankInfo(bank, pageMatcher) {
  const edataPages = ee_getBankSubAreas(bank)
    .filter(pageMatcher)
    .sort((leftPage, rightPage) => Number(leftPage.page_number) - Number(rightPage.page_number));

  if (edataPages.length === 0) {
    return undefined;
  }

  const firstPage = edataPages[0];
  const lastPage = edataPages[edataPages.length - 1];
  const firstPageNumber = Number(firstPage?.page_number);
  const lastPageNumber = Number(lastPage?.page_number);
  const pageSizeBytes = Number(firstPage?.sub_area_size_B ?? 0);
  const busSizeBytes = Number(firstPage?.bus_size_b ?? 0);
  const firstPageRanges = ee_getUserModeRanges(firstPage);
  const lastPageRanges = ee_getUserModeRanges(lastPage);
  const pageCount = (lastPageNumber - firstPageNumber) + 1;

  if (!Number.isInteger(firstPageNumber)
      || !Number.isInteger(lastPageNumber)
      || firstPageNumber < 0
      || lastPageNumber < firstPageNumber
      || !Number.isInteger(pageSizeBytes)
      || pageSizeBytes <= 0
      || !Number.isInteger(busSizeBytes)
      || busSizeBytes <= 0
      || pageCount <= 0) {
    return undefined;
  }

  return {
    pageSizeBytes,
    busSizeBytes,
    nonSecureStartAddress: firstPageRanges.nonSecure?.start,
    nonSecureEndAddress: lastPageRanges.nonSecure?.end,
    secureStartAddress: firstPageRanges.secure?.start,
    secureEndAddress: lastPageRanges.secure?.end
  };
}

/**
 * Extracts normalized geometry information from a page-based column EDATA bank.
 *
 * @param {object} bank NVM bank description.
 * @returns {object|undefined} Normalized column EDATA bank geometry, or undefined when the bank does not expose valid column EDATA pages.
 */
function ee_getColumnEdataBankInfo(bank) {
  return ee_getPagedEdataBankInfo(bank, ee_isEdataColumnPageSubArea);
}

/**
 * Extracts normalized geometry information from a page-based row EDATA bank.
 *
 * @param {object} bank NVM bank description.
 * @returns {object|undefined} Normalized row EDATA bank geometry, or undefined when the bank does not expose valid row EDATA pages.
 */
function ee_getRowEdataBankInfo(bank) {
  return ee_getPagedEdataBankInfo(bank, ee_isEdataRowPageSubArea);
}

/**
 * Returns the address used to sort normalized EDATA banks.
 *
 * @param {object} bankInfo Normalized EDATA bank geometry.
 * @returns {number} Non-secure start address when available, otherwise secure start address or a large fallback value.
 */
function ee_getEdataBankSortAddress(bankInfo) {
  return bankInfo?.nonSecureStartAddress ?? bankInfo?.secureStartAddress ?? Number.MAX_SAFE_INTEGER;
}

/**
 * Returns normalized EDATA bank geometries from the hardware description.
 *
 * Legacy and page-based EDATA layouts are normalized into the same bank-info
 * structure and sorted by user-mode address.
 *
 * @param {object|Array<object>} hardwareDescription Raw or normalized hardware description.
 * @returns {Array<object>} Sorted normalized EDATA bank geometries.
 */
function ee_getEdataBankInfos(hardwareDescription) {
  return ee_getNvmBanks(hardwareDescription)
    .map(bank => ee_getColumnEdataBankInfo(bank) ?? ee_getRowEdataBankInfo(bank) ?? ee_getLegacyEdataBankInfo(bank))
    .filter(bankInfo => bankInfo !== undefined)
    .sort((leftBank, rightBank) => ee_getEdataBankSortAddress(leftBank) - ee_getEdataBankSortAddress(rightBank));
}

/**
 * Returns the first normalized EDATA bank geometry.
 *
 * @param {object|Array<object>} hardwareDescription Raw or normalized hardware description.
 * @returns {object|undefined} Primary EDATA bank geometry.
 */
function ee_getPrimaryEdataBankInfo(hardwareDescription) {
  return ee_getEdataBankInfos(hardwareDescription)[0];
}

/**
 * Returns normalized EDATA ranges for the selected security view.
 *
 * Secure addresses are preferred for secure contexts, with fallback to the
 * alternate user-mode view when only one range is available.
 *
 * @param {object|Array<object>} hardwareDescription Raw or normalized hardware description.
 * @param {string|object} engineSecureValue Security context.
 * @returns {Array<{start:number,end:number,busSizeBytes:number,pageSizeBytes:number}>} Sorted EDATA ranges.
 */
function ee_getEdataRanges(hardwareDescription, engineSecureValue) {
  const shouldUseSecureAddresses = ee_shouldUseSecureAddresses(engineSecureValue);

  return ee_getEdataBankInfos(hardwareDescription)
    .map(bankInfo => {
      const preferredRange = shouldUseSecureAddresses
        ? {
          start: bankInfo.secureStartAddress,
          end: bankInfo.secureEndAddress
        }
        : {
          start: bankInfo.nonSecureStartAddress,
          end: bankInfo.nonSecureEndAddress
        };
      const fallbackRange = shouldUseSecureAddresses
        ? {
          start: bankInfo.nonSecureStartAddress,
          end: bankInfo.nonSecureEndAddress
        }
        : {
          start: bankInfo.secureStartAddress,
          end: bankInfo.secureEndAddress
        };
      const selectedRange = Number.isInteger(preferredRange.start)
        && Number.isInteger(preferredRange.end)
        && preferredRange.start <= preferredRange.end
        ? preferredRange
        : fallbackRange;

      return {
        start: selectedRange.start,
        end: selectedRange.end,
        busSizeBytes: bankInfo.busSizeBytes,
        pageSizeBytes: bankInfo.pageSizeBytes
      };
    })
    .filter(range => Number.isInteger(range.start)
      && Number.isInteger(range.end)
      && Number.isInteger(range.busSizeBytes)
      && Number.isInteger(range.pageSizeBytes)
      && range.busSizeBytes > 0
      && range.pageSizeBytes > 0
      && range.start <= range.end)
    .sort((leftRange, rightRange) => leftRange.start - rightRange.start);
}

/**
 * Checks whether two EDATA ranges can be merged.
 *
 * @param {object} previousRange Previous normalized EDATA range.
 * @param {object} currentRange Current normalized EDATA range.
 * @returns {boolean} True when both ranges are adjacent and share the same geometry.
 */
function ee_areEdataRangesContiguous(previousRange, currentRange) {
  return previousRange.pageSizeBytes === currentRange.pageSizeBytes
    && previousRange.busSizeBytes === currentRange.busSizeBytes
    && (previousRange.end + 1) === currentRange.start;
}

/**
 * Returns merged contiguous EDATA ranges for the selected security view.
 *
 * @param {object|Array<object>} hardwareDescription Raw or normalized hardware description.
 * @param {string|object} engineSecureValue Security context.
 * @returns {Array<{start:number,end:number,busSizeBytes:number,pageSizeBytes:number}>} Merged EDATA ranges.
 */
function ee_getMergedEdataRanges(hardwareDescription, engineSecureValue) {
  const edataRanges = ee_getEdataRanges(hardwareDescription, engineSecureValue);

  return edataRanges.reduce((mergedRanges, range) => {
    const previousRange = mergedRanges[mergedRanges.length - 1];

    if (!previousRange) {
      mergedRanges.push({
        start: range.start,
        end: range.end,
        busSizeBytes: range.busSizeBytes,
        pageSizeBytes: range.pageSizeBytes
      });
      return mergedRanges;
    }

    if (ee_areEdataRangesContiguous(previousRange, range)) {
      previousRange.end = range.end;
      return mergedRanges;
    }

    mergedRanges.push({
      start: range.start,
      end: range.end,
      busSizeBytes: range.busSizeBytes,
      pageSizeBytes: range.pageSizeBytes
    });
    return mergedRanges;
  }, []);
}

/**
 * Returns a non-negative integer value or a fallback.
 *
 * @param {string|number|undefined} value Raw value to normalize.
 * @param {number} [defaultValue=0] Fallback value.
 * @returns {number} Non-negative integer value, or the fallback.
 */
function ee_getPositiveInteger(value, defaultValue = 0) {
  const numericValue = Number(value);

  if (!Number.isInteger(numericValue) || numericValue < 0) {
    return defaultValue;
  }

  return numericValue;
}

/**
 * Returns the effective field value after applying an override.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {string} fieldName Field name to read.
 * @param {string} overrideFieldName Field currently overridden.
 * @param {string|number} overrideValue Temporary override value.
 * @returns {string|number|undefined} Effective field value.
 */
function ee_getOverriddenValue(basic, fieldName, overrideFieldName, overrideValue) {
  if (fieldName === overrideFieldName) {
    return overrideValue;
  }

  return basic?.[fieldName];
}

/**
 * Returns the effective overridden field value as a non-negative integer.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {string} fieldName Field name to read.
 * @param {string} overrideFieldName Field currently overridden.
 * @param {string|number} overrideValue Temporary override value.
 * @returns {number} Effective non-negative integer value.
 */
function ee_getOverriddenInteger(basic, fieldName, overrideFieldName, overrideValue) {
  return ee_getPositiveInteger(ee_getOverriddenValue(basic, fieldName, overrideFieldName, overrideValue));
}

/**
 * Checks whether a value is an integer inside an optional range.
 *
 * @param {string|number} value Raw value to validate.
 * @param {number} minimum Inclusive minimum value.
 * @param {number} maximum Inclusive maximum value.
 * @returns {boolean} True when the value is an integer within the provided bounds.
 */
function ee_isIntegerInRange(value, minimum, maximum) {
  const numericValue = Number(value);

  if (!Number.isInteger(numericValue)) {
    return false;
  }

  if (minimum !== undefined && numericValue < minimum) {
    return false;
  }

  if (maximum !== undefined && numericValue > maximum) {
    return false;
  }

  return true;
}

/**
 * Checks whether an effective field value is an integer inside an optional range.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {string} fieldName Field name to validate.
 * @param {string} overrideFieldName Field currently overridden.
 * @param {string|number} overrideValue Temporary override value.
 * @param {number} minimum Inclusive minimum value.
 * @param {number} maximum Inclusive maximum value.
 * @returns {boolean} True when the effective field value satisfies the range.
 */
function ee_isFieldIntegerInRange(basic, fieldName, overrideFieldName, overrideValue, minimum, maximum) {
  return ee_isIntegerInRange(ee_getOverriddenValue(basic, fieldName, overrideFieldName, overrideValue), minimum, maximum);
}

/**
 * Returns the effective EEPROM emulation start address.
 *
 * This prefers the explicit EEPROM start address and falls back to the flash
 * area start address when no explicit value is configured.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {string} overrideFieldName Field currently overridden.
 * @param {string} overrideValue Temporary override value.
 * @returns {string|undefined} Effective start address value.
 */
function ee_getConfiguredStartAddress(basic, overrideFieldName, overrideValue) {
  const startAddress = ee_getOverriddenValue(basic, "eeprom_emulation_start_address", overrideFieldName, overrideValue);

  if (startAddress !== undefined && startAddress !== null && String(startAddress) !== "") {
    return startAddress;
  }

  if (overrideFieldName === "eeprom_emulation_start_address") {
    return startAddress;
  }

  return basic?.flash_area_start_address;
}

/**
 * Returns the effective total variable count for FLITF mode.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {string} overrideFieldName Field currently overridden.
 * @param {string|number} overrideValue Temporary override value.
 * @returns {number} Effective FLITF total variable count.
 */
function ee_getFlitfTotalVariables(basic, overrideFieldName, overrideValue) {
  return ee_getOverriddenInteger(basic, "total_variables_number", overrideFieldName, overrideValue);
}

/**
 * Returns the requested payload size per cycle in bytes.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {string} overrideFieldName Field currently overridden.
 * @param {string|number} overrideValue Temporary override value.
 * @returns {number} Requested payload size per cycle in bytes.
 */
function ee_getRequestedBytesPerCycle(basic, overrideFieldName, overrideValue) {
  const flashType = basic?.flash_type;
  const variables8Bit = flashType === "FLITF"
    ? ee_getFlitfTotalVariables(basic, overrideFieldName, overrideValue)
    : ee_getOverriddenInteger(basic, "variables_number_8bit", overrideFieldName, overrideValue);
  const variables16Bit = flashType === "NVM"
    ? ee_getOverriddenInteger(basic, "variables_number_16bit", overrideFieldName, overrideValue)
    : 0;
  const variables32Bit = flashType === "NVM"
    ? ee_getOverriddenInteger(basic, "variables_number_32bit", overrideFieldName, overrideValue)
    : 0;
  const bytesPer8BitVariable = flashType === "FLITF" ? 8 : 4;

  return (variables8Bit * bytesPer8BitVariable) + ((variables16Bit + variables32Bit) * 8);
}

/**
 * Returns the effective cycle count for the selected flash backend.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {string} overrideFieldName Field currently overridden.
 * @param {string|number} overrideValue Temporary override value.
 * @returns {number} Effective cycle count, or 0 when unusable.
 */
function ee_getCycles(basic, overrideFieldName, overrideValue) {
  const cyclesFieldName = basic?.flash_type === "FLITF"
    ? "cycles_numbers_flitf"
    : "cycles_numbers_nvm";
  const cycles = ee_getOverriddenInteger(basic, cyclesFieldName, overrideFieldName, overrideValue);

  return cycles > 0 ? cycles : 0;
}

/**
 * Returns the writing_cycle value exposed by the selected EDATA area.
 *
 * Column-page EDATA layouts are preferred over legacy single-block layouts so
 * page-based D44F descriptors use the same source as the EEPROM geometry
 * helpers.
 *
 * @param {object|Array<object>} hardwareDescription Raw or normalized hardware description.
 * @returns {string} writing_cycle value, or an empty string when unavailable.
 */
function ee_getSelectedWritingCycle(hardwareDescription) {
  const edataSubAreas = ee_getNvmBanks(hardwareDescription)
    .flatMap(bank => ee_getBankSubAreas(bank))
    .filter(ee_isEdataSubArea);

  const selectedSubArea = edataSubAreas.find(ee_isEdataColumnPageSubArea)
    ?? edataSubAreas.find(ee_isLegacyEdataSubArea)
    ?? edataSubAreas[0];

  return String(selectedSubArea?.writing_cycle ?? "").trim();
}

/**
 * Parses a compact cycle-count string such as 10K or 1M.
 *
 * @param {string} value Compact cycle-count string.
 * @returns {number|undefined} Parsed cycle count, or undefined when the format is unsupported.
 */
function ee_parseCompactCycleCount(value) {
  const normalizedValue = String(value ?? "").trim().toUpperCase();
  const match = normalizedValue.match(/^(\d+(?:\.\d+)?)([KMG])?$/);

  if (!match) {
    return undefined;
  }

  const numericValue = Number.parseFloat(match[1]);
  const factor = {
    "": 1,
    K: 1000,
    M: 1000000,
    G: 1000000000
  }[match[2] ?? ""];

  if (!Number.isFinite(numericValue) || !Number.isFinite(factor)) {
    return undefined;
  }

  return numericValue * factor;
}

/**
 * Formats a cycle count using compact K/M/G suffixes when possible.
 *
 * @param {number} value Cycle count to format.
 * @returns {string} Compact cycle-count string, or an empty string when unusable.
 */
function ee_formatCompactCycleCount(value) {
  if (!Number.isFinite(value) || value <= 0) {
    return "";
  }

  const units = [
    { suffix: "G", factor: 1000000000 },
    { suffix: "M", factor: 1000000 },
    { suffix: "K", factor: 1000 }
  ];

  for (const unit of units) {
    if (value >= unit.factor && Number.isInteger(value / unit.factor)) {
      return `${value / unit.factor}${unit.suffix}`;
    }
  }

  return String(value);
}

/**
 * Returns the dynamic description for the Number of cycles parameter.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {object|Array<object>} hardwareDescription Raw or normalized hardware description.
 * @param {string|object} engineSecureValue Security context.
 * @returns {string} Description enriched with the current Number of cycles * writing_cycle information.
 */
function ee_getCyclesDescription(basic, hardwareDescription, engineSecureValue) {
  const baseDescription = basic?.flash_type === "FLITF"
    ? "Set the number of write cycles for the FLITF algorithm."
    : `Set the number of write cycles for the NVM algorithm. Allowed range is 1 to ${EE_NVM_MAX_CYCLES}.`;
  const maximumCycles = ee_getMaximumCyclesForFlashType(basic?.flash_type);

  if (!ee_hasEdata(hardwareDescription, engineSecureValue)) {
    return baseDescription;
  }

  const cycles = ee_getCycles(basic, undefined, undefined);
  const writingCycle = ee_getSelectedWritingCycle(hardwareDescription);

  if (cycles <= 0 || (Number.isInteger(maximumCycles) && cycles > maximumCycles) || writingCycle === "") {
    return baseDescription;
  }

  const totalCycleCount = ee_parseCompactCycleCount(writingCycle);
  const multiplierInformation = totalCycleCount === undefined
    ? `${cycles} * ${writingCycle}`
    : `${cycles} * ${writingCycle} = ${ee_formatCompactCycleCount(cycles * totalCycleCount)}`;

  return `${baseDescription} This information is provided for indication only. It helps relate the selected Number of cycles to the FLASH writing_cycle: ${multiplierInformation}.`;
}

/**
 * Returns the dynamic title for the Number of cycles parameter.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {object|Array<object>} hardwareDescription Raw or normalized hardware description.
 * @param {string|object} engineSecureValue Security context.
 * @returns {string} Title enriched with the selected writing_cycle information when available.
 */
function ee_getCyclesTitle(basic, hardwareDescription, engineSecureValue) {
  const baseTitle = "Number of cycles";

  if (!ee_hasEdata(hardwareDescription, engineSecureValue)) {
    return baseTitle;
  }

  const writingCycle = ee_getSelectedWritingCycle(hardwareDescription);

  return writingCycle === ""
    ? baseTitle
    : `${baseTitle} (writing cycle: ${writingCycle})`;
}

/**
 * Returns the common FLITF sizing context used to compute dynamic maxima.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {object|Array<object>} hardwareDescription Raw or normalized hardware description.
 * @param {string|object} engineSecureValue Security context.
 * @returns {{availablePages:number,pageSizeBytes:number,maxElementsByPage:number}|undefined} FLITF sizing context, or undefined when unavailable.
 */
function ee_getFlitfLimitContext(basic, hardwareDescription, engineSecureValue) {
  if (basic?.flash_type !== "FLITF") {
    return undefined;
  }

  const startAddressValue = ee_getConfiguredStartAddress(basic, undefined, undefined);
  const startAddress = ee_parseAddress(startAddressValue);
  const edataRanges = ee_getMergedEdataRanges(hardwareDescription, engineSecureValue);
  const pageSizeBytes = edataRanges[0]?.pageSizeBytes ?? 0;
  const elementSizeBytes = ee_getPrimaryEdataBankInfo(hardwareDescription)?.busSizeBytes ?? 0;
  const maxElementsByPage = ee_getFlitfMaxElementsByPage(pageSizeBytes, elementSizeBytes);

  if (startAddress === undefined
      || edataRanges.length === 0
      || pageSizeBytes <= 0
      || maxElementsByPage <= 0
      || !ee_isAddressInEdata(hardwareDescription, startAddressValue, engineSecureValue)
      || !ee_isPageStart(hardwareDescription, startAddressValue, engineSecureValue)) {
    return undefined;
  }

  const availablePages = ee_getAvailablePagesFromStart(hardwareDescription, startAddress, engineSecureValue);

  if (!Number.isInteger(availablePages) || availablePages <= 0) {
    return undefined;
  }

  return {
    availablePages,
    pageSizeBytes,
    maxElementsByPage
  };
}

/**
 * Appends FLITF maximum-value information to a field description.
 *
 * @param {string} baseDescription Static field description.
 * @param {number|undefined} maximumValue Current maximum allowed value.
 * @returns {string} Description enriched with the current maximum, or a no-valid-value message.
 */
function ee_getFlitfMaximumValueDescription(baseDescription, maximumValue) {
  if (Number.isInteger(maximumValue) && maximumValue >= 0) {
    return `${baseDescription} With the current FLITF settings, the maximum allowed value is ${maximumValue}.`;
  }

  return `${baseDescription}`;
}

/**
 * Returns the common NVM sizing context used to compute dynamic maxima.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {object|Array<object>} hardwareDescription Raw or normalized hardware description.
 * @param {string|object} engineSecureValue Security context.
 * @returns {{availablePages:number,pageSizeBytes:number}|undefined} NVM sizing context, or undefined when unavailable.
 */
function ee_getNvmLimitContext(basic, hardwareDescription, engineSecureValue) {
  if (basic?.flash_type !== "NVM") {
    return undefined;
  }

  const startAddressValue = ee_getConfiguredStartAddress(basic, undefined, undefined);
  const startAddress = ee_parseAddress(startAddressValue);
  const edataRanges = ee_getMergedEdataRanges(hardwareDescription, engineSecureValue);
  const pageSizeBytes = edataRanges[0]?.pageSizeBytes ?? 0;

  if (startAddress === undefined
      || edataRanges.length === 0
      || pageSizeBytes <= 0
      || !ee_isAddressInEdata(hardwareDescription, startAddressValue, engineSecureValue)
      || !ee_isPageStart(hardwareDescription, startAddressValue, engineSecureValue)) {
    return undefined;
  }

  const availablePages = ee_getAvailablePagesFromStart(hardwareDescription, startAddress, engineSecureValue);

  if (!Number.isInteger(availablePages) || availablePages <= 0) {
    return undefined;
  }

  return {
    availablePages,
    pageSizeBytes
  };
}

/**
 * Appends NVM maximum-value information to a field description.
 *
 * @param {string} baseDescription Static field description.
 * @param {number|undefined} maximumValue Current maximum allowed value.
 * @returns {string} Description enriched with the current maximum, or the base description when unavailable.
 */
function ee_getNvmMaximumValueDescription(baseDescription, maximumValue) {
  if (Number.isInteger(maximumValue) && maximumValue >= 0) {
    return `${baseDescription} With the current NVM settings, the maximum allowed value is ${maximumValue}.`;
  }

  return `${baseDescription}`;
}

/**
 * Returns the maximum payload size per cycle allowed by the current NVM settings.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {object|Array<object>} hardwareDescription Raw or normalized hardware description.
 * @param {string|object} engineSecureValue Security context.
 * @returns {number|undefined} Maximum payload size per cycle in bytes, or undefined when unavailable.
 */
function ee_getNvmMaximumPayloadBytesPerCycle(basic, hardwareDescription, engineSecureValue) {
  if (!ee_isFieldIntegerInRange(basic, "cycles_numbers_nvm", undefined, undefined, 1, EE_NVM_MAX_CYCLES)) {
    return undefined;
  }

  const context = ee_getNvmLimitContext(basic, hardwareDescription, engineSecureValue);
  const cycles = ee_getCycles(basic, undefined, undefined);

  if (!context || cycles <= 0) {
    return undefined;
  }

  const maximumPagesPerCycle = Math.floor(context.availablePages / cycles);

  if (maximumPagesPerCycle < 1) {
    return undefined;
  }

  const maximumPayloadBytes = (maximumPagesPerCycle * context.pageSizeBytes) - 1;

  return maximumPayloadBytes >= 0 ? maximumPayloadBytes : undefined;
}

/**
 * Returns the maximum NVM Number of 8-bit variables allowed by the current settings.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {object|Array<object>} hardwareDescription Raw or normalized hardware description.
 * @param {string|object} engineSecureValue Security context.
 * @returns {number|undefined} Maximum Number of 8-bit variables, or undefined when unavailable.
 */
function ee_getNvmMaximum8BitVariables(basic, hardwareDescription, engineSecureValue) {
  if (!ee_isFieldIntegerInRange(basic, "variables_number_16bit", undefined, undefined, 0)
      || !ee_isFieldIntegerInRange(basic, "variables_number_32bit", undefined, undefined, 0)
      || !ee_isFieldIntegerInRange(basic, "cycles_numbers_nvm", undefined, undefined, 1, EE_NVM_MAX_CYCLES)) {
    return undefined;
  }

  const maximumPayloadBytes = ee_getNvmMaximumPayloadBytesPerCycle(basic, hardwareDescription, engineSecureValue);
  const usedBytes = (ee_getOverriddenInteger(basic, "variables_number_16bit", undefined, undefined)
    + ee_getOverriddenInteger(basic, "variables_number_32bit", undefined, undefined)) * 8;

  if (!Number.isInteger(maximumPayloadBytes)) {
    return undefined;
  }

  const remainingBytes = maximumPayloadBytes - usedBytes;

  return remainingBytes >= 0 ? Math.floor(remainingBytes / 4) : undefined;
}

/**
 * Returns the maximum NVM Number of 16-bit variables allowed by the current settings.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {object|Array<object>} hardwareDescription Raw or normalized hardware description.
 * @param {string|object} engineSecureValue Security context.
 * @returns {number|undefined} Maximum Number of 16-bit variables, or undefined when unavailable.
 */
function ee_getNvmMaximum16BitVariables(basic, hardwareDescription, engineSecureValue) {
  if (!ee_isFieldIntegerInRange(basic, "variables_number_8bit", undefined, undefined, 0)
      || !ee_isFieldIntegerInRange(basic, "variables_number_32bit", undefined, undefined, 0)
      || !ee_isFieldIntegerInRange(basic, "cycles_numbers_nvm", undefined, undefined, 1, EE_NVM_MAX_CYCLES)) {
    return undefined;
  }

  const maximumPayloadBytes = ee_getNvmMaximumPayloadBytesPerCycle(basic, hardwareDescription, engineSecureValue);
  const usedBytes = (ee_getOverriddenInteger(basic, "variables_number_8bit", undefined, undefined) * 4)
    + (ee_getOverriddenInteger(basic, "variables_number_32bit", undefined, undefined) * 8);

  if (!Number.isInteger(maximumPayloadBytes)) {
    return undefined;
  }

  const remainingBytes = maximumPayloadBytes - usedBytes;

  return remainingBytes >= 0 ? Math.floor(remainingBytes / 8) : undefined;
}

/**
 * Returns the maximum NVM Number of 32-bit variables allowed by the current settings.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {object|Array<object>} hardwareDescription Raw or normalized hardware description.
 * @param {string|object} engineSecureValue Security context.
 * @returns {number|undefined} Maximum Number of 32-bit variables, or undefined when unavailable.
 */
function ee_getNvmMaximum32BitVariables(basic, hardwareDescription, engineSecureValue) {
  if (!ee_isFieldIntegerInRange(basic, "variables_number_8bit", undefined, undefined, 0)
      || !ee_isFieldIntegerInRange(basic, "variables_number_16bit", undefined, undefined, 0)
      || !ee_isFieldIntegerInRange(basic, "cycles_numbers_nvm", undefined, undefined, 1, EE_NVM_MAX_CYCLES)) {
    return undefined;
  }

  const maximumPayloadBytes = ee_getNvmMaximumPayloadBytesPerCycle(basic, hardwareDescription, engineSecureValue);
  const usedBytes = (ee_getOverriddenInteger(basic, "variables_number_8bit", undefined, undefined) * 4)
    + (ee_getOverriddenInteger(basic, "variables_number_16bit", undefined, undefined) * 8);

  if (!Number.isInteger(maximumPayloadBytes)) {
    return undefined;
  }

  const remainingBytes = maximumPayloadBytes - usedBytes;

  return remainingBytes >= 0 ? Math.floor(remainingBytes / 8) : undefined;
}

/**
 * Returns the maximum NVM Number of cycles allowed by the current settings.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {object|Array<object>} hardwareDescription Raw or normalized hardware description.
 * @param {string|object} engineSecureValue Security context.
 * @returns {number|undefined} Maximum Number of cycles, or undefined when unavailable.
 */
function ee_getNvmMaximumCycles(basic, hardwareDescription, engineSecureValue) {
  if (!ee_isFieldIntegerInRange(basic, "variables_number_8bit", undefined, undefined, 0)
      || !ee_isFieldIntegerInRange(basic, "variables_number_16bit", undefined, undefined, 0)
      || !ee_isFieldIntegerInRange(basic, "variables_number_32bit", undefined, undefined, 0)) {
    return undefined;
  }

  const context = ee_getNvmLimitContext(basic, hardwareDescription, engineSecureValue);
  const requestedBytesPerCycle = ee_getRequestedBytesPerCycle(basic, undefined, undefined);

  if (!context || !Number.isInteger(requestedBytesPerCycle) || requestedBytesPerCycle < 0) {
    return undefined;
  }

  const pagesPerCycle = Math.floor(requestedBytesPerCycle / context.pageSizeBytes) + 1;

  if (pagesPerCycle <= 0) {
    return undefined;
  }

  const maximumCycles = Math.floor(context.availablePages / pagesPerCycle);

  if (maximumCycles < 1) {
    return undefined;
  }

  return Math.min(EE_NVM_MAX_CYCLES, maximumCycles);
}

/**
 * Returns the NVM dynamic description for Number of 8-bit variables.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {object|Array<object>} hardwareDescription Raw or normalized hardware description.
 * @param {string|object} engineSecureValue Security context.
 * @returns {string} Description enriched with the current maximum allowed value.
 */
function ee_getNvm8BitVariablesDescription(basic, hardwareDescription, engineSecureValue) {
  const baseDescription = "Set the number of 8-bit variables to write in EEPROM Emulation.";

  if (basic?.flash_type !== "NVM"
      || !ee_isFieldIntegerInRange(basic, "variables_number_16bit", undefined, undefined, 0)
      || !ee_isFieldIntegerInRange(basic, "variables_number_32bit", undefined, undefined, 0)
      || !ee_isFieldIntegerInRange(basic, "cycles_numbers_nvm", undefined, undefined, 1, EE_NVM_MAX_CYCLES)) {
    return baseDescription;
  }

  return ee_getNvmMaximumValueDescription(
    baseDescription,
    ee_getNvmMaximum8BitVariables(basic, hardwareDescription, engineSecureValue)
  );
}

/**
 * Returns the NVM dynamic description for Number of 16-bit variables.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {object|Array<object>} hardwareDescription Raw or normalized hardware description.
 * @param {string|object} engineSecureValue Security context.
 * @returns {string} Description enriched with the current maximum allowed value.
 */
function ee_getNvm16BitVariablesDescription(basic, hardwareDescription, engineSecureValue) {
  const baseDescription = "Set the number of 16-bit variables to write in EEPROM Emulation.";

  if (basic?.flash_type !== "NVM"
      || !ee_isFieldIntegerInRange(basic, "variables_number_8bit", undefined, undefined, 0)
      || !ee_isFieldIntegerInRange(basic, "variables_number_32bit", undefined, undefined, 0)
      || !ee_isFieldIntegerInRange(basic, "cycles_numbers_nvm", undefined, undefined, 1, EE_NVM_MAX_CYCLES)) {
    return baseDescription;
  }

  return ee_getNvmMaximumValueDescription(
    baseDescription,
    ee_getNvmMaximum16BitVariables(basic, hardwareDescription, engineSecureValue)
  );
}

/**
 * Returns the NVM dynamic description for Number of 32-bit variables.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {object|Array<object>} hardwareDescription Raw or normalized hardware description.
 * @param {string|object} engineSecureValue Security context.
 * @returns {string} Description enriched with the current maximum allowed value.
 */
function ee_getNvm32BitVariablesDescription(basic, hardwareDescription, engineSecureValue) {
  const baseDescription = "Set the number of 32-bit variables to write in EEPROM Emulation.";

  if (basic?.flash_type !== "NVM"
      || !ee_isFieldIntegerInRange(basic, "variables_number_8bit", undefined, undefined, 0)
      || !ee_isFieldIntegerInRange(basic, "variables_number_16bit", undefined, undefined, 0)
      || !ee_isFieldIntegerInRange(basic, "cycles_numbers_nvm", undefined, undefined, 1, EE_NVM_MAX_CYCLES)) {
    return baseDescription;
  }

  return ee_getNvmMaximumValueDescription(
    baseDescription,
    ee_getNvmMaximum32BitVariables(basic, hardwareDescription, engineSecureValue)
  );
}

/**
 * Returns the NVM dynamic description for Number of cycles.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {object|Array<object>} hardwareDescription Raw or normalized hardware description.
 * @param {string|object} engineSecureValue Security context.
 * @returns {string} Description enriched with the current maximum allowed value.
 */
function ee_getNvmCyclesDescription(basic, hardwareDescription, engineSecureValue) {
  const cyclesDescription = ee_getCyclesDescription(basic, hardwareDescription, engineSecureValue);

  if (basic?.flash_type !== "NVM"
      || !ee_isFieldIntegerInRange(basic, "variables_number_8bit", undefined, undefined, 0)
      || !ee_isFieldIntegerInRange(basic, "variables_number_16bit", undefined, undefined, 0)
      || !ee_isFieldIntegerInRange(basic, "variables_number_32bit", undefined, undefined, 0)) {
    return cyclesDescription;
  }

  return ee_getNvmMaximumValueDescription(
    cyclesDescription,
    ee_getNvmMaximumCycles(basic, hardwareDescription, engineSecureValue)
  );
}

/**
 * Returns the maximum FLITF Total variables number allowed by the current settings.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {object|Array<object>} hardwareDescription Raw or normalized hardware description.
 * @param {string|object} engineSecureValue Security context.
 * @returns {number|undefined} Maximum Total variables number, or undefined when unavailable.
 */
function ee_getFlitfMaximumTotalVariables(basic, hardwareDescription, engineSecureValue) {
  if (!ee_isFieldIntegerInRange(basic, "cycles_numbers_flitf", undefined, undefined, 1)
      || !ee_isFieldIntegerInRange(basic, "guard_pages_number", undefined, undefined, 0)) {
    return undefined;
  }

  const context = ee_getFlitfLimitContext(basic, hardwareDescription, engineSecureValue);
  const cycles = ee_getCycles(basic, undefined, undefined);
  const guardPages = ee_getGuardPages(basic, undefined, undefined);

  if (!context || cycles <= 0) {
    return undefined;
  }

  const maxPagesPerCycle = Math.floor((context.availablePages - guardPages) / cycles);
  const maxPagePairs = Math.floor(maxPagesPerCycle / 2);

  if (maxPagePairs < 1) {
    return undefined;
  }

  return (maxPagePairs * context.maxElementsByPage) - 1;
}

/**
 * Returns the maximum FLITF Number of cycles allowed by the current settings.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {object|Array<object>} hardwareDescription Raw or normalized hardware description.
 * @param {string|object} engineSecureValue Security context.
 * @returns {number|undefined} Maximum Number of cycles, or undefined when unavailable.
 */
function ee_getFlitfMaximumCycles(basic, hardwareDescription, engineSecureValue) {
  if (!ee_isFieldIntegerInRange(basic, "total_variables_number", undefined, undefined, 0)
      || !ee_isFieldIntegerInRange(basic, "guard_pages_number", undefined, undefined, 0)) {
    return undefined;
  }

  const context = ee_getFlitfLimitContext(basic, hardwareDescription, engineSecureValue);
  const guardPages = ee_getGuardPages(basic, undefined, undefined);

  if (!context) {
    return undefined;
  }

  const pagesPerCycle = ee_getFlitfPagesPerCycle(basic, hardwareDescription, undefined, undefined, context.pageSizeBytes);

  if (pagesPerCycle <= 0) {
    return undefined;
  }

  const maximumCycles = Math.floor((context.availablePages - guardPages) / pagesPerCycle);

  return maximumCycles >= 1 ? maximumCycles : undefined;
}

/**
 * Returns the maximum FLITF Number of guard pages allowed by the current settings.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {object|Array<object>} hardwareDescription Raw or normalized hardware description.
 * @param {string|object} engineSecureValue Security context.
 * @returns {number|undefined} Maximum Number of guard pages, or undefined when unavailable.
 */
function ee_getFlitfMaximumGuardPages(basic, hardwareDescription, engineSecureValue) {
  if (!ee_isFieldIntegerInRange(basic, "total_variables_number", undefined, undefined, 0)
      || !ee_isFieldIntegerInRange(basic, "cycles_numbers_flitf", undefined, undefined, 1)) {
    return undefined;
  }

  const context = ee_getFlitfLimitContext(basic, hardwareDescription, engineSecureValue);
  const cycles = ee_getCycles(basic, undefined, undefined);

  if (!context || cycles <= 0) {
    return undefined;
  }

  const pagesPerCycle = ee_getFlitfPagesPerCycle(basic, hardwareDescription, undefined, undefined, context.pageSizeBytes);

  if (pagesPerCycle <= 0) {
    return undefined;
  }

  const maximumGuardPages = context.availablePages - (pagesPerCycle * cycles);

  return maximumGuardPages >= 0 ? maximumGuardPages : undefined;
}

/**
 * Returns the FLITF dynamic description for Total variables number.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {object|Array<object>} hardwareDescription Raw or normalized hardware description.
 * @param {string|object} engineSecureValue Security context.
 * @returns {string} Description enriched with the current maximum allowed value.
 */
function ee_getFlitfTotalVariablesDescription(basic, hardwareDescription, engineSecureValue) {
  const baseDescription = "Set the total number of variables to write in EEPROM Emulation.";

  if (basic?.flash_type !== "FLITF"
      || !ee_isFieldIntegerInRange(basic, "cycles_numbers_flitf", undefined, undefined, 1)
      || !ee_isFieldIntegerInRange(basic, "guard_pages_number", undefined, undefined, 0)) {
    return baseDescription;
  }

  return ee_getFlitfMaximumValueDescription(
    baseDescription,
    ee_getFlitfMaximumTotalVariables(basic, hardwareDescription, engineSecureValue)
  );
}

/**
 * Returns the FLITF dynamic description for Number of cycles.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {object|Array<object>} hardwareDescription Raw or normalized hardware description.
 * @param {string|object} engineSecureValue Security context.
 * @returns {string} Description enriched with the current maximum allowed value.
 */
function ee_getFlitfCyclesDescription(basic, hardwareDescription, engineSecureValue) {
  const cyclesDescription = ee_getCyclesDescription(basic, hardwareDescription);

  if (basic?.flash_type !== "FLITF"
      || !ee_isFieldIntegerInRange(basic, "cycles_numbers_flitf", undefined, undefined, 1)
      || !ee_isFieldIntegerInRange(basic, "total_variables_number", undefined, undefined, 0)
      || !ee_isFieldIntegerInRange(basic, "guard_pages_number", undefined, undefined, 0)) {
    return cyclesDescription;
  }

  return ee_getFlitfMaximumValueDescription(
    cyclesDescription,
    ee_getFlitfMaximumCycles(basic, hardwareDescription, engineSecureValue)
  );
}

/**
 * Returns the FLITF dynamic description for Number of guard pages.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {object|Array<object>} hardwareDescription Raw or normalized hardware description.
 * @param {string|object} engineSecureValue Security context.
 * @returns {string} Description enriched with the current maximum allowed value.
 */
function ee_getFlitfGuardPagesDescription(basic, hardwareDescription, engineSecureValue) {
  const baseDescription = "Set the number of guard pages to reduce transfer frequency.";

  if (basic?.flash_type !== "FLITF"
      || !ee_isFieldIntegerInRange(basic, "total_variables_number", undefined, undefined, 0)
      || !ee_isFieldIntegerInRange(basic, "cycles_numbers_flitf", undefined, undefined, 1)) {
    return baseDescription;
  }

  return ee_getFlitfMaximumValueDescription(
    baseDescription,
    ee_getFlitfMaximumGuardPages(basic, hardwareDescription, engineSecureValue)
  );
}

/**
 * Returns the effective FLITF guard page count.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {string} overrideFieldName Field currently overridden.
 * @param {string|number} overrideValue Temporary override value.
 * @returns {number} Guard page count, or 0 outside FLITF mode.
 */
function ee_getGuardPages(basic, overrideFieldName, overrideValue) {
  if (basic?.flash_type !== "FLITF") {
    return 0;
  }

  const guardPages = ee_getOverriddenInteger(basic, "guard_pages_number", overrideFieldName, overrideValue);

  return guardPages > 0 ? guardPages : 0;
}

/**
 * Returns the maximum number of FLITF elements that fit in one page.
 *
 * @param {number} pageSizeBytes Page size in bytes.
 * @param {number} elementSizeBytes FLITF element size in bytes.
 * @returns {number} Maximum element count per page.
 */
function ee_getFlitfMaxElementsByPage(pageSizeBytes, elementSizeBytes) {
  if (!Number.isInteger(pageSizeBytes) || pageSizeBytes <= 0) {
    return 0;
  }

  if (!Number.isInteger(elementSizeBytes) || elementSizeBytes <= 0) {
    return 0;
  }

  const pageHeaderSizeBytes = elementSizeBytes * 4;
  const payloadBytes = pageSizeBytes - pageHeaderSizeBytes;

  if (payloadBytes <= 0) {
    return 0;
  }

  return Math.floor(payloadBytes / elementSizeBytes);
}

/**
 * Returns the number of FLITF pages required per cycle.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {object|Array<object>} hardwareDescription Raw or normalized hardware description.
 * @param {string} overrideFieldName Field currently overridden.
 * @param {string|number} overrideValue Temporary override value.
 * @param {number} pageSizeBytes Page size in bytes.
 * @returns {number} Required FLITF page count per cycle.
 */
function ee_getFlitfPagesPerCycle(basic, hardwareDescription, overrideFieldName, overrideValue, pageSizeBytes) {
  const nbOfVariables = ee_getFlitfTotalVariables(basic, overrideFieldName, overrideValue);
  const elementSizeBytes = ee_getPrimaryEdataBankInfo(hardwareDescription)?.busSizeBytes ?? 0;
  const maxElementsByPage = ee_getFlitfMaxElementsByPage(pageSizeBytes, elementSizeBytes);

  if (!Number.isInteger(nbOfVariables) || nbOfVariables < 0 || maxElementsByPage <= 0) {
    return 0;
  }

  return (Math.floor(nbOfVariables / maxElementsByPage) + 1) * 2;
}

/**
 * Returns the number of pages required per cycle for the selected backend.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {object|Array<object>} hardwareDescription Raw or normalized hardware description.
 * @param {string} overrideFieldName Field currently overridden.
 * @param {string|number} overrideValue Temporary override value.
 * @param {number} requestedBytesPerCycle Requested payload size per cycle in bytes.
 * @param {number} pageSizeBytes Page size in bytes.
 * @returns {number} Required page count per cycle.
 */
function ee_getRequestedPagesPerCycle(basic, hardwareDescription, overrideFieldName, overrideValue, requestedBytesPerCycle, pageSizeBytes) {
  if (!Number.isInteger(pageSizeBytes) || pageSizeBytes <= 0) {
    return 0;
  }

  if (basic?.flash_type === "NVM") {
    if (!Number.isInteger(requestedBytesPerCycle) || requestedBytesPerCycle <= 0) {
      return 0;
    }

    return Math.floor(requestedBytesPerCycle / pageSizeBytes) + 1;
  }

  if (basic?.flash_type === "FLITF") {
    return ee_getFlitfPagesPerCycle(basic, hardwareDescription, overrideFieldName, overrideValue, pageSizeBytes);
  }

  if (!Number.isInteger(requestedBytesPerCycle) || requestedBytesPerCycle <= 0) {
    return 0;
  }

  return Math.ceil(requestedBytesPerCycle / pageSizeBytes);
}

/**
 * Returns the total number of 16-bit and 32-bit variables in NVM mode.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @returns {number} Sum of 16-bit and 32-bit variables, or 0 outside NVM mode.
 */
function helper_eeprom_get_16_32bit_variables_total(basic) {
  if (basic?.flash_type !== "NVM") {
    return 0;
  }

  return ee_getPositiveInteger(basic?.variables_number_16bit)
    + ee_getPositiveInteger(basic?.variables_number_32bit);
}

/**
 * Counts how many writable pages remain from the selected start address.
 *
 * @param {object|Array<object>} hardwareDescription Raw or normalized hardware description.
 * @param {number} startAddress Page-aligned start address.
 * @param {string|object} engineSecureValue Security context.
 * @returns {number} Number of writable pages available from the start address.
 */
function ee_getAvailablePagesFromStart(hardwareDescription, startAddress, engineSecureValue) {
  const mergedRanges = ee_getMergedEdataRanges(hardwareDescription, engineSecureValue);

  return mergedRanges.reduce((availablePages, range) => {
    if (startAddress > range.end) {
      return availablePages;
    }

    const firstWritableAddress = startAddress <= range.start
      ? range.start
      : range.start + (Math.ceil((startAddress - range.start) / range.pageSizeBytes) * range.pageSizeBytes);

    if (firstWritableAddress > range.end) {
      return availablePages;
    }

    return availablePages + Math.floor((range.end - firstWritableAddress) / range.pageSizeBytes) + 1;
  }, 0);
}

/**
 * Checks whether the hardware description exposes at least one EDATA range.
 *
 * @param {object|Array<object>} hardwareDescription Raw or normalized hardware description.
 * @param {string|object} engineSecureValue Security context.
 * @returns {boolean} True when at least one EDATA range is available.
 */
function ee_hasEdata(hardwareDescription, engineSecureValue) {
  return ee_getMergedEdataRanges(hardwareDescription, engineSecureValue).length > 0;
}

/**
 * Checks whether the selected flash backend exposes a usable flash area.
 *
 * @param {string} flashType Selected flash backend, typically NVM or FLITF.
 * @param {object} nvmHardwareDescription Candidate NVM hardware description.
 * @param {object} flashHardwareDescription Candidate FLASH hardware description.
 * @param {string|object} engineSecureValue Security context.
 * @returns {boolean} True when the selected backend exposes at least one flash area.
 */
function helper_eeprom_has_selected_flash_area(flashType, nvmHardwareDescription, flashHardwareDescription, engineSecureValue) {
  return ee_hasEdata(
    ee_selectHardwareDescription(flashType, nvmHardwareDescription, flashHardwareDescription),
    engineSecureValue
  );
}

/**
 * Returns the first writable EDATA start address.
 *
 * @param {object|Array<object>} hardwareDescription Raw or normalized hardware description.
 * @param {string|object} engineSecureValue Security context.
 * @returns {string} First writable EDATA start address, or an empty string when unavailable.
 */
function ee_getFirstEdataStartAddress(hardwareDescription, engineSecureValue) {
  return ee_toHexAddress(ee_getMergedEdataRanges(hardwareDescription, engineSecureValue)[0]?.start);
}

/**
 * Returns the first writable start address for the selected flash backend.
 *
 * @param {string} flashType Selected flash backend, typically NVM or FLITF.
 * @param {object} nvmHardwareDescription Candidate NVM hardware description.
 * @param {object} flashHardwareDescription Candidate FLASH hardware description.
 * @param {string|object} engineSecureValue Security context.
 * @returns {string} First writable flash-area start address, or an empty string when unavailable.
 */
function helper_eeprom_get_selected_flash_area_start_address(flashType, nvmHardwareDescription, flashHardwareDescription, engineSecureValue) {
  return ee_getFirstEdataStartAddress(
    ee_selectHardwareDescription(flashType, nvmHardwareDescription, flashHardwareDescription),
    engineSecureValue
  );
}

/**
 * Returns all valid page-aligned start addresses as a display string.
 *
 * @param {object|Array<object>} hardwareDescription Raw or normalized hardware description.
 * @param {string|object} engineSecureValue Security context.
 * @returns {Array<{address: string}>} Valid start addresses formatted for array rendering.
 */
function ee_getValidStartAddresses(hardwareDescription, engineSecureValue) {
  const mergedRanges = ee_getMergedEdataRanges(hardwareDescription, engineSecureValue);
  const validStartAddresses = [];

  for (const range of mergedRanges) {
    if (!Number.isInteger(range?.start) || !Number.isInteger(range?.end) || !Number.isInteger(range?.pageSizeBytes)) {
      continue;
    }

    for (let address = range.start; address <= range.end; address += range.pageSizeBytes) {
      validStartAddresses.push({ address: ee_toHexAddress(address) });
    }
  }

  return validStartAddresses;
}

/**
 * Returns all valid start addresses for the selected flash backend.
 *
 * @param {string} flashType Selected flash backend, typically NVM or FLITF.
 * @param {object} nvmHardwareDescription Candidate NVM hardware description.
 * @param {object} flashHardwareDescription Candidate FLASH hardware description.
 * @param {string|object} engineSecureValue Security context.
 * @returns {Array<{address: string}>} Valid flash-area start addresses for array rendering.
 */
function helper_eeprom_get_selected_flash_area_valid_start_addresses(flashType, nvmHardwareDescription, flashHardwareDescription, engineSecureValue) {
  return ee_getValidStartAddresses(
    ee_selectHardwareDescription(flashType, nvmHardwareDescription, flashHardwareDescription),
    engineSecureValue
  );
}

/**
 * Checks whether all inputs needed to compute required pages are valid.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {object|Array<object>} hardwareDescription Raw or normalized hardware description.
 * @param {string} overrideFieldName Field currently overridden.
 * @param {string|number} overrideValue Temporary override value.
 * @param {string|object} engineSecureValue Security context.
 * @returns {boolean} True when all required inputs are valid.
 */
function ee_hasValidRequiredPagesInputs(basic, hardwareDescription, overrideFieldName, overrideValue, engineSecureValue) {
  const flashType = basic?.flash_type;
  const startAddressValue = ee_getConfiguredStartAddress(basic, overrideFieldName, overrideValue);

  if (flashType !== "NVM" && flashType !== "FLITF") {
    return false;
  }

  if (ee_parseAddress(startAddressValue) === undefined) {
    return false;
  }

  if (!ee_isAddressInEdata(hardwareDescription, startAddressValue, engineSecureValue)) {
    return false;
  }

  if (!ee_isPageStart(hardwareDescription, startAddressValue, engineSecureValue)) {
    return false;
  }

  if (flashType === "NVM") {
    return ee_isFieldIntegerInRange(basic, "variables_number_8bit", overrideFieldName, overrideValue, 0)
      && ee_isFieldIntegerInRange(basic, "variables_number_16bit", overrideFieldName, overrideValue, 0)
      && ee_isFieldIntegerInRange(basic, "variables_number_32bit", overrideFieldName, overrideValue, 0)
      && ee_isFieldIntegerInRange(basic, "cycles_numbers_nvm", overrideFieldName, overrideValue, 1, EE_NVM_MAX_CYCLES);
  }

  return ee_isFieldIntegerInRange(basic, "total_variables_number", overrideFieldName, overrideValue, 0)
    && ee_isFieldIntegerInRange(basic, "cycles_numbers_flitf", overrideFieldName, overrideValue, 1)
    && ee_isFieldIntegerInRange(basic, "guard_pages_number", overrideFieldName, overrideValue, 0);
}

/**
 * Computes the raw number of pages required by the current configuration.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {object|Array<object>} hardwareDescription Raw or normalized hardware description.
 * @param {string} overrideFieldName Field currently overridden.
 * @param {string|number} overrideValue Temporary override value.
 * @param {string|object} engineSecureValue Security context.
 * @returns {number|undefined} Required page count, or undefined when inputs cannot be used.
 */
function ee_getRequiredPages(basic, hardwareDescription, overrideFieldName, overrideValue, engineSecureValue) {
  const flashType = basic?.flash_type;
  const edataRanges = ee_getMergedEdataRanges(hardwareDescription, engineSecureValue);
  const pageSizeBytes = edataRanges[0]?.pageSizeBytes ?? 0;
  const cycles = ee_getCycles(basic, overrideFieldName, overrideValue);
  const guardPages = ee_getGuardPages(basic, overrideFieldName, overrideValue);

  if (pageSizeBytes <= 0 || cycles <= 0) {
    return undefined;
  }

  if (flashType === "NVM") {
    const requestedBytesPerCycle = ee_getRequestedBytesPerCycle(basic, overrideFieldName, overrideValue);
    if (!Number.isInteger(requestedBytesPerCycle) || requestedBytesPerCycle < 0) {
      return undefined;
    }

    return (Math.floor(requestedBytesPerCycle / pageSizeBytes) + 1) * cycles;
  }

  if (flashType === "FLITF") {
    const requestedPagesPerCycle = ee_getFlitfPagesPerCycle(basic, hardwareDescription, overrideFieldName, overrideValue, pageSizeBytes);
    if (!Number.isInteger(requestedPagesPerCycle) || requestedPagesPerCycle <= 0) {
      return undefined;
    }

    return (requestedPagesPerCycle * cycles) + guardPages;
  }

  return undefined;
}

/**
 * Returns the required page count formatted for UI consumption.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {object|Array<object>} hardwareDescription Raw or normalized hardware description.
 * @param {string|object} engineSecureValue Security context.
 * @returns {string} Required page count as a string, or a status message when the configuration is not usable.
 */
function ee_getFormattedRequiredPages(basic, hardwareDescription, engineSecureValue) {
  if (!ee_hasValidRequiredPagesInputs(basic, hardwareDescription, undefined, undefined, engineSecureValue)) {
    return "Required pages cannot be calculated. Check the start address and EEPROM settings.";
  }

  const startAddress = ee_parseAddress(ee_getConfiguredStartAddress(basic, undefined, undefined));
  const requiredPages = ee_getRequiredPages(basic, hardwareDescription, undefined, undefined, engineSecureValue);
  const availablePages = Number.isInteger(startAddress)
    ? ee_getAvailablePagesFromStart(hardwareDescription, startAddress, engineSecureValue)
    : 0;

  if (!Number.isInteger(requiredPages) || requiredPages <= 0 || availablePages <= 0) {
    return "Required pages cannot be calculated. Check the start address and EEPROM settings.";
  }

  if (requiredPages > availablePages) {
    return "Exceeds the number of pages available";
  }

  return String(requiredPages);
}

/**
 * Returns the formatted required page count for the selected flash backend.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {object} nvmHardwareDescription Candidate NVM hardware description.
 * @param {object} flashHardwareDescription Candidate FLASH hardware description.
 * @param {string|object} engineSecureValue Security context.
 * @returns {string} Required page count as a string, or a status message when unavailable.
 */
function helper_eeprom_get_selected_required_pages(basic, nvmHardwareDescription, flashHardwareDescription, engineSecureValue) {
  return ee_getFormattedRequiredPages(
    basic,
    ee_getSelectedHardwareDescriptionForBasic(basic, nvmHardwareDescription, flashHardwareDescription),
    engineSecureValue
  );
}

/**
 * Checks whether an address belongs to any writable EDATA range.
 *
 * @param {object|Array<object>} hardwareDescription Raw or normalized hardware description.
 * @param {string} value Address candidate to test.
 * @param {string|object} engineSecureValue Security context.
 * @returns {boolean} True when the address belongs to a writable EDATA range.
 */
function ee_isAddressInEdata(hardwareDescription, value, engineSecureValue) {
  const inputAddress = ee_parseAddress(value);

  if (inputAddress === undefined) {
    return false;
  }

  return ee_getMergedEdataRanges(hardwareDescription, engineSecureValue)
    .some(range => inputAddress >= range.start && inputAddress <= range.end);
}

/**
 * Checks whether an address belongs to the selected flash backend area.
 *
 * @param {string} flashType Selected flash backend, typically NVM or FLITF.
 * @param {object} nvmHardwareDescription Candidate NVM hardware description.
 * @param {object} flashHardwareDescription Candidate FLASH hardware description.
 * @param {string} value Address candidate to test.
 * @param {string|object} engineSecureValue Security context.
 * @returns {boolean} True when the address belongs to the selected flash area.
 */
function helper_eeprom_is_selected_address_in_flash_area(flashType, nvmHardwareDescription, flashHardwareDescription, value, engineSecureValue) {
  return ee_isAddressInEdata(
    ee_selectHardwareDescription(flashType, nvmHardwareDescription, flashHardwareDescription),
    value,
    engineSecureValue
  );
}

/**
 * Checks whether an address is aligned on an EDATA page boundary.
 *
 * @param {object|Array<object>} hardwareDescription Raw or normalized hardware description.
 * @param {string} value Address candidate to test.
 * @param {string|object} engineSecureValue Security context.
 * @returns {boolean} True when the address is a valid page start inside EDATA.
 */
function ee_isPageStart(hardwareDescription, value, engineSecureValue) {
  const inputAddress = ee_parseAddress(value);

  if (inputAddress === undefined) {
    return false;
  }

  return ee_getMergedEdataRanges(hardwareDescription, engineSecureValue)
    .some(range => inputAddress >= range.start
      && inputAddress <= range.end
      && ((inputAddress - range.start) % range.pageSizeBytes) === 0);
}

/**
 * Checks whether an address is a page start in the selected flash backend area.
 *
 * @param {string} flashType Selected flash backend, typically NVM or FLITF.
 * @param {object} nvmHardwareDescription Candidate NVM hardware description.
 * @param {object} flashHardwareDescription Candidate FLASH hardware description.
 * @param {string} value Address candidate to test.
 * @param {string|object} engineSecureValue Security context.
 * @returns {boolean} True when the address is a valid page start inside the selected flash area.
 */
function helper_eeprom_is_selected_flash_area_page_start(flashType, nvmHardwareDescription, flashHardwareDescription, value, engineSecureValue) {
  return ee_isPageStart(
    ee_selectHardwareDescription(flashType, nvmHardwareDescription, flashHardwareDescription),
    value,
    engineSecureValue
  );
}

/**
 * Checks whether the selected EEPROM emulation start address is invalid.
 *
 * A blank value does not count as an error here because the Start address field
 * already controls when it is prefilled and when its own validation should run.
 *
 * @param {string} flashType Selected flash backend, typically NVM or FLITF.
 * @param {object} nvmHardwareDescription Candidate NVM hardware description.
 * @param {object} flashHardwareDescription Candidate FLASH hardware description.
 * @param {string} value Start address candidate to test.
 * @param {string|object} engineSecureValue Security context.
 * @returns {boolean} True when the selected start address is present and invalid.
 */
function helper_eeprom_has_selected_start_address_error(flashType, nvmHardwareDescription, flashHardwareDescription, value, engineSecureValue) {
  if (value === undefined || value === "") {
    return true;
  }

  if (String(value).trim() === "") {
    return true;
  }

  if (!/^0x[0-9a-fA-F]+$/.test(String(value))) {
    return true;
  }

  return !helper_eeprom_is_selected_address_in_flash_area(
    flashType,
    nvmHardwareDescription,
    flashHardwareDescription,
    value,
    engineSecureValue
  ) || !helper_eeprom_is_selected_flash_area_page_start(
    flashType,
    nvmHardwareDescription,
    flashHardwareDescription,
    value,
    engineSecureValue
  );
}

/**
 * Returns the last address covered by a requested number of pages.
 *
 * @param {object|Array<object>} hardwareDescription Raw or normalized hardware description.
 * @param {number} startAddress Page-aligned start address.
 * @param {number} requestedPages Number of pages to allocate.
 * @param {string|object} engineSecureValue Security context.
 * @returns {number|undefined} Final covered address, or undefined when inputs cannot be used.
 */
function ee_getEndAddressForRequestedPages(hardwareDescription, startAddress, requestedPages, engineSecureValue) {
  if (!Number.isInteger(startAddress) || !Number.isInteger(requestedPages) || requestedPages <= 0) {
    return undefined;
  }

  const mergedRanges = ee_getMergedEdataRanges(hardwareDescription, engineSecureValue);
  const lastEdataEndAddress = mergedRanges[mergedRanges.length - 1]?.end;

  if (mergedRanges.length === 0 || !Number.isInteger(lastEdataEndAddress)) {
    return undefined;
  }

  let remainingPages = requestedPages;
  let currentStartAddress = startAddress;

  for (const range of mergedRanges) {
    if (currentStartAddress > range.end) {
      continue;
    }

    const firstWritableAddress = currentStartAddress <= range.start
      ? range.start
      : range.start + (Math.ceil((currentStartAddress - range.start) / range.pageSizeBytes) * range.pageSizeBytes);

    if (firstWritableAddress > range.end) {
      continue;
    }

    const pagesInRange = Math.floor((range.end - firstWritableAddress) / range.pageSizeBytes) + 1;

    if (remainingPages <= pagesInRange) {
      const lastPageStartAddress = firstWritableAddress + ((remainingPages - 1) * range.pageSizeBytes);
      const computedEndAddress = lastPageStartAddress + range.pageSizeBytes - 1;
      return Math.min(computedEndAddress, range.end);
    }

    remainingPages -= pagesInRange;
    currentStartAddress = range.end + 1;
  }

  return lastEdataEndAddress;
}

/**
 * Returns the normalized frame line size expected by the generated config.
 *
 * @param {object|Array<object>} hardwareDescription Raw or normalized hardware description.
 * @returns {number} 8 when raw line size is <=16 bytes, otherwise 16.
 */
function helper_eeprom_get_frame_line_size(hardwareDescription) {
  const rawLineSize = ee_getPrimaryEdataBankInfo(hardwareDescription)?.busSizeBytes ?? 0;

  /* Keep generated macro values limited to 8 or 16 per template expectation. */
  return rawLineSize <= 16 ? 8 : 16;
}

/**
 * Returns the sector size of the primary EDATA bank.
 *
 * @param {object|Array<object>} hardwareDescription Raw or normalized hardware description.
 * @returns {number} Sector size in bytes.
 */
function helper_eeprom_get_sector_size(hardwareDescription) {
  return ee_getPrimaryEdataBankInfo(hardwareDescription)?.pageSizeBytes ?? 0;
}

/**
 * Returns the formatted end address covered by the current configuration.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {object|Array<object>} hardwareDescription Raw or normalized hardware description.
 * @param {string|object} engineSecureValue Security context.
 * @returns {string} End address as a hexadecimal string, or an empty string when unavailable.
 */
function ee_getFormattedEndAddress(basic, hardwareDescription, engineSecureValue) {
  const startAddress = ee_parseAddress(basic?.eeprom_emulation_start_address);
  const edataRanges = ee_getMergedEdataRanges(hardwareDescription, engineSecureValue);
  const pageSizeBytes = edataRanges[0]?.pageSizeBytes ?? 0;
  const cycles = ee_getCycles(basic, undefined, undefined);
  const guardPages = ee_getGuardPages(basic, undefined, undefined);
  const requestedBytesPerCycle = ee_getRequestedBytesPerCycle(basic, undefined, undefined);

  if (startAddress === undefined || edataRanges.length === 0 || pageSizeBytes <= 0 || cycles <= 0) {
    return "";
  }

  const requestedPagesPerCycle = ee_getRequestedPagesPerCycle(basic, hardwareDescription, undefined, undefined, requestedBytesPerCycle, pageSizeBytes);
  if (requestedPagesPerCycle <= 0) {
    return "";
  }

  const requestedPages = (requestedPagesPerCycle * cycles) + guardPages;
  const endAddress = ee_getEndAddressForRequestedPages(hardwareDescription, startAddress, requestedPages, engineSecureValue);

  return ee_toHexAddress(endAddress);
}

/**
 * Returns the formatted final address of the selected EDATA area.
 *
 * @param {object|Array<object>} hardwareDescription Raw or normalized hardware description.
 * @param {string|object} engineSecureValue Security context.
 * @returns {string} Final EDATA address as a hexadecimal string, or an empty string when unavailable.
 */
function ee_getFormattedFlashAreaEndAddress(hardwareDescription, engineSecureValue) {
  const edataRanges = ee_getMergedEdataRanges(hardwareDescription, engineSecureValue);
  const lastEdataEndAddress = edataRanges[edataRanges.length - 1]?.end;

  return ee_toHexAddress(lastEdataEndAddress);
}

/**
 * Returns the formatted end address for the selected flash backend.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {object} nvmHardwareDescription Candidate NVM hardware description.
 * @param {object} flashHardwareDescription Candidate FLASH hardware description.
 * @param {string|object} engineSecureValue Security context.
 * @returns {string} End address as a hexadecimal string, or an empty string when unavailable.
 */
function helper_eeprom_get_selected_end_address(basic, nvmHardwareDescription, flashHardwareDescription, engineSecureValue) {
  const selectedHardwareDescription = ee_getSelectedHardwareDescriptionForBasic(basic, nvmHardwareDescription, flashHardwareDescription);

  if (helper_eeprom_has_selected_start_address_error(
    basic?.flash_type,
    nvmHardwareDescription,
    flashHardwareDescription,
    basic?.eeprom_emulation_start_address,
    engineSecureValue
  )) {
    return ee_getFormattedFlashAreaEndAddress(selectedHardwareDescription, engineSecureValue);
  }

  return ee_getFormattedEndAddress(
    basic,
    selectedHardwareDescription,
    engineSecureValue
  );
}

/**
 * Returns the dynamic description for the selected Number of cycles parameter.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {object} nvmHardwareDescription Candidate NVM hardware description.
 * @param {object} flashHardwareDescription Candidate FLASH hardware description.
 * @param {string|object} engineSecureValue Security context.
 * @returns {string} Description enriched with dynamic NVM information when applicable.
 */
function helper_eeprom_get_selected_cycles_description(basic, nvmHardwareDescription, flashHardwareDescription, engineSecureValue) {
  const selectedHardwareDescription = ee_getSelectedHardwareDescriptionForBasic(basic, nvmHardwareDescription, flashHardwareDescription);

  return basic?.flash_type === "NVM"
    ? ee_getNvmCyclesDescription(
      basic,
      selectedHardwareDescription,
      engineSecureValue
    )
    : ee_getCyclesDescription(
      basic,
      selectedHardwareDescription,
      engineSecureValue
    );
}

/**
 * Returns the dynamic title for the selected Number of cycles parameter.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {object} nvmHardwareDescription Candidate NVM hardware description.
 * @param {object} flashHardwareDescription Candidate FLASH hardware description.
 * @param {string|object} engineSecureValue Security context.
 * @returns {string} Title enriched with the selected writing_cycle information when available.
 */
function helper_eeprom_get_selected_cycles_title(basic, nvmHardwareDescription, flashHardwareDescription, engineSecureValue) {
  return ee_getCyclesTitle(
    basic,
    ee_getSelectedHardwareDescriptionForBasic(basic, nvmHardwareDescription, flashHardwareDescription),
    engineSecureValue
  );
}

/**
 * Returns the NVM dynamic description for Number of 8-bit variables.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {object} nvmHardwareDescription Candidate NVM hardware description.
 * @param {object} flashHardwareDescription Candidate FLASH hardware description.
 * @param {string|object} engineSecureValue Security context.
 * @returns {string} Description enriched with the current maximum allowed value.
 */
function helper_eeprom_get_selected_nvm_8bit_variables_description(basic, nvmHardwareDescription, flashHardwareDescription, engineSecureValue) {
  return ee_getNvm8BitVariablesDescription(
    basic,
    ee_getSelectedHardwareDescriptionForBasic(basic, nvmHardwareDescription, flashHardwareDescription),
    engineSecureValue
  );
}

/**
 * Returns the NVM dynamic description for Number of 16-bit variables.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {object} nvmHardwareDescription Candidate NVM hardware description.
 * @param {object} flashHardwareDescription Candidate FLASH hardware description.
 * @param {string|object} engineSecureValue Security context.
 * @returns {string} Description enriched with the current maximum allowed value.
 */
function helper_eeprom_get_selected_nvm_16bit_variables_description(basic, nvmHardwareDescription, flashHardwareDescription, engineSecureValue) {
  return ee_getNvm16BitVariablesDescription(
    basic,
    ee_getSelectedHardwareDescriptionForBasic(basic, nvmHardwareDescription, flashHardwareDescription),
    engineSecureValue
  );
}

/**
 * Returns the NVM dynamic description for Number of 32-bit variables.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {object} nvmHardwareDescription Candidate NVM hardware description.
 * @param {object} flashHardwareDescription Candidate FLASH hardware description.
 * @param {string|object} engineSecureValue Security context.
 * @returns {string} Description enriched with the current maximum allowed value.
 */
function helper_eeprom_get_selected_nvm_32bit_variables_description(basic, nvmHardwareDescription, flashHardwareDescription, engineSecureValue) {
  return ee_getNvm32BitVariablesDescription(
    basic,
    ee_getSelectedHardwareDescriptionForBasic(basic, nvmHardwareDescription, flashHardwareDescription),
    engineSecureValue
  );
}

/**
 * Returns the maximum NVM Number of cycles allowed by the selected settings.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {object} nvmHardwareDescription Candidate NVM hardware description.
 * @param {object} flashHardwareDescription Candidate FLASH hardware description.
 * @param {string|object} engineSecureValue Security context.
 * @returns {number|undefined} Maximum Number of cycles allowed by the current NVM settings, or undefined when unavailable.
 */
function helper_eeprom_get_selected_nvm_maximum_cycles(basic, nvmHardwareDescription, flashHardwareDescription, engineSecureValue) {
  return ee_getNvmMaximumCycles(
    basic,
    ee_getSelectedHardwareDescriptionForBasic(basic, nvmHardwareDescription, flashHardwareDescription),
    engineSecureValue
  );
}

/**
 * Returns the FLITF dynamic description for Total variables number.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {object} nvmHardwareDescription Candidate NVM hardware description.
 * @param {object} flashHardwareDescription Candidate FLASH hardware description.
 * @param {string|object} engineSecureValue Security context.
 * @returns {string} Description enriched with the current maximum allowed value.
 */
function helper_eeprom_get_selected_flitf_total_variables_description(basic, nvmHardwareDescription, flashHardwareDescription, engineSecureValue) {
  return ee_getFlitfTotalVariablesDescription(
    basic,
    ee_getSelectedHardwareDescriptionForBasic(basic, nvmHardwareDescription, flashHardwareDescription),
    engineSecureValue
  );
}

/**
 * Returns the FLITF dynamic description for Number of cycles.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {object} nvmHardwareDescription Candidate NVM hardware description.
 * @param {object} flashHardwareDescription Candidate FLASH hardware description.
 * @param {string|object} engineSecureValue Security context.
 * @returns {string} Description enriched with the current maximum allowed value.
 */
function helper_eeprom_get_selected_flitf_cycles_description(basic, nvmHardwareDescription, flashHardwareDescription, engineSecureValue) {
  return ee_getFlitfCyclesDescription(
    basic,
    ee_getSelectedHardwareDescriptionForBasic(basic, nvmHardwareDescription, flashHardwareDescription),
    engineSecureValue
  );
}

/**
 * Returns the maximum FLITF Number of cycles allowed by the selected settings.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {object} nvmHardwareDescription Candidate NVM hardware description.
 * @param {object} flashHardwareDescription Candidate FLASH hardware description.
 * @param {string|object} engineSecureValue Security context.
 * @returns {number|undefined} Maximum Number of cycles allowed by the current FLITF settings, or undefined when unavailable.
 */
function helper_eeprom_get_selected_flitf_maximum_cycles(basic, nvmHardwareDescription, flashHardwareDescription, engineSecureValue) {
  return ee_getFlitfMaximumCycles(
    basic,
    ee_getSelectedHardwareDescriptionForBasic(basic, nvmHardwareDescription, flashHardwareDescription),
    engineSecureValue
  );
}

/**
 * Returns the FLITF dynamic description for Number of guard pages.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {object} nvmHardwareDescription Candidate NVM hardware description.
 * @param {object} flashHardwareDescription Candidate FLASH hardware description.
 * @param {string|object} engineSecureValue Security context.
 * @returns {string} Description enriched with the current maximum allowed value.
 */
function helper_eeprom_get_selected_flitf_guard_pages_description(basic, nvmHardwareDescription, flashHardwareDescription, engineSecureValue) {
  return ee_getFlitfGuardPagesDescription(
    basic,
    ee_getSelectedHardwareDescriptionForBasic(basic, nvmHardwareDescription, flashHardwareDescription),
    engineSecureValue
  );
}

/**
 * Checks whether the requested configuration exceeds available pages.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {object|Array<object>} hardwareDescription Raw or normalized hardware description.
 * @param {string} overrideFieldName Field currently overridden.
 * @param {string|number} overrideValue Temporary override value.
 * @param {string|object} engineSecureValue Security context.
 * @returns {boolean} True when the requested configuration exceeds available pages.
 */
function ee_exceedsAvailablePages(basic, hardwareDescription, overrideFieldName, overrideValue, engineSecureValue) {
  const startAddress = ee_parseAddress(basic?.eeprom_emulation_start_address);
  const edataRanges = ee_getMergedEdataRanges(hardwareDescription, engineSecureValue);
  const pageSizeBytes = edataRanges[0]?.pageSizeBytes ?? 0;
  const cycles = ee_getCycles(basic, overrideFieldName, overrideValue);
  const guardPages = ee_getGuardPages(basic, overrideFieldName, overrideValue);
  const requestedBytesPerCycle = ee_getRequestedBytesPerCycle(basic, overrideFieldName, overrideValue);

  if (startAddress === undefined || edataRanges.length === 0 || pageSizeBytes <= 0 || cycles <= 0) {
    return false;
  }

  const requestedPagesPerCycle = ee_getRequestedPagesPerCycle(basic, hardwareDescription, overrideFieldName, overrideValue, requestedBytesPerCycle, pageSizeBytes);
  if (requestedPagesPerCycle <= 0) {
    return false;
  }

  const requestedPages = (requestedPagesPerCycle * cycles) + guardPages;
  const availablePages = ee_getAvailablePagesFromStart(hardwareDescription, startAddress, engineSecureValue);

  return availablePages > 0 && requestedPages > availablePages;
}

/**
 * Checks whether the selected flash backend exceeds its available pages.
 *
 * @param {object} basic Basic EEPROM Emulation configuration object.
 * @param {object} nvmHardwareDescription Candidate NVM hardware description.
 * @param {object} flashHardwareDescription Candidate FLASH hardware description.
 * @param {string} overrideFieldName Field currently overridden.
 * @param {string|number} overrideValue Temporary override value.
 * @param {string|object} engineSecureValue Security context.
 * @returns {boolean} True when the selected flash backend exceeds available pages.
 */
function helper_eeprom_selected_exceeds_available_pages(basic, nvmHardwareDescription, flashHardwareDescription, overrideFieldName, overrideValue, engineSecureValue) {
  return ee_exceedsAvailablePages(
    basic,
    ee_getSelectedHardwareDescriptionForBasic(basic, nvmHardwareDescription, flashHardwareDescription),
    overrideFieldName,
    overrideValue,
    engineSecureValue
  );
}

module.exports = {
  helper_eeprom_has_selected_flash_area,
  helper_eeprom_get_selected_flash_area_start_address,
  helper_eeprom_get_selected_required_pages,
  helper_eeprom_get_selected_flash_area_valid_start_addresses,
  helper_eeprom_get_frame_line_size,
  helper_eeprom_get_sector_size,
  helper_eeprom_get_selected_end_address,
  helper_eeprom_get_selected_cycles_title,
  helper_eeprom_get_selected_cycles_description,
  helper_eeprom_get_selected_nvm_8bit_variables_description,
  helper_eeprom_get_selected_nvm_16bit_variables_description,
  helper_eeprom_get_selected_nvm_32bit_variables_description,
  helper_eeprom_get_selected_nvm_maximum_cycles,
  helper_eeprom_get_selected_flitf_total_variables_description,
  helper_eeprom_get_selected_flitf_cycles_description,
  helper_eeprom_get_selected_flitf_maximum_cycles,
  helper_eeprom_get_selected_flitf_guard_pages_description,
  helper_eeprom_get_16_32bit_variables_total,
  helper_eeprom_has_selected_start_address_error,
  helper_eeprom_is_selected_address_in_flash_area,
  helper_eeprom_is_selected_flash_area_page_start,
  helper_eeprom_selected_exceeds_available_pages
};
