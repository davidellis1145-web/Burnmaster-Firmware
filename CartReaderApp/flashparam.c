#include <stdio.h>
#include <string.h>
#include <gd32f10x.h>
#include "flashparam.h"
 

void save_dword(uint32_t data)
{
  // For backward compatibility, save to GB save counter address
  save_dword_at(FMC_GB_SAVE_COUNTER_ADDR, data);
}

uint32_t load_dword()
{
  // For backward compatibility, load from GB save counter address
  return load_dword_at(FMC_GB_SAVE_COUNTER_ADDR);
}

void save_dword_at(uint32_t address, uint32_t data)
{
  fmc_unlock();
  fmc_word_program(address, data);
  // lock the main FMC after the program operation
  fmc_lock();
}

uint32_t load_dword_at(uint32_t address)
{
  uint32_t *ptr = (uint32_t *)address;
  return ptr[0];
}