#pragma once

#define enable_GBX

uint8_t SDCardInit();

void gbxScreen();

uint8_t gbTestsMenu(uint8_t skipWarning);
uint8_t gbaTestsMenu(uint8_t skipWarning);
uint8_t gbxDebugMenu(uint8_t skipWarning);
void gbTestsScreen();
void gbaTestsScreen();
void gbxDebugScreen();
