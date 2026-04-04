#ifndef __FLASH_PARAM_H__
#define __FLASH_PARAM_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */


#define FMC_PAGE_SIZE           ((uint16_t)0x400U)
#define FMC_WRITE_START_ADDR    ((uint32_t)0x0803FC00U)
#define FMC_WRITE_END_ADDR	((uint32_t)0x0803FFFFU)

// Separate addresses for different cartridge types
// Note: Generated with AI assistance (GitHub Copilot)
#define FMC_GB_SAVE_COUNTER_ADDR   ((uint32_t)0x0803FC00U)  // Game Boy save counter
#define FMC_GB_ROM_COUNTER_ADDR    ((uint32_t)0x0803FC04U)  // Game Boy ROM counter
#define FMC_GBA_ROM_COUNTER_ADDR   ((uint32_t)0x0803FC08U)  // Game Boy Advance ROM counter
#define FMC_GBA_SAVE_COUNTER_ADDR  ((uint32_t)0x0803FC0CU)  // Game Boy Advance save counter
#define FMC_GBM_SAVE_COUNTER_ADDR  ((uint32_t)0x0803FC10U)  // Game Boy Memory save counter

void save_dwordGB(uint32_t data);
uint32_t load_dwordGB();
void save_dword_at(uint32_t address, uint32_t data);
uint32_t load_dword_at(uint32_t address);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif