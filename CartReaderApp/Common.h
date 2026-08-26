#ifndef _COMMON_H_
#define _COMMON_H_

#include <gd32f10x.h>
#include <stdio.h>
#include <stdlib.h>
#include "Display.h"
#include "Operate.h"
#include "fatfs/ff.h"

#define byte uint8_t
#define word uint16_t
#define boolean uint8_t
#define bool uint8_t
#define true (1)
#define false (0)

#define FILENAME_LENGTH 64
#define FILEPATH_LENGTH 256
#define FILEOPTS_LENGTH 20
#define SD_LOCKED_ERROR 198

#define SPEED_READ	2499 // Fast blink (250ms toggle) for reading
#define SPEED_WRITE	624  // Hyper-fast flicker (62.5ms toggle) for writing
#define SPEED_ERROR	4999 // Normal blink (500ms toggle) for error messages

#define TYPE_GBC (0)
#define TYPE_GBA (1)
#define TYPE_ALL (2)
#define TYPE_NONE (3)

extern int foldern;
extern char folder[36];
extern FATFS fs;

extern char fileName[FILENAME_LENGTH];
extern char filePath[FILEPATH_LENGTH];
extern word currPage;
extern word lastPage;
extern word numPages;
extern boolean root;
extern boolean filebrowse;
extern bool errorLvl;
extern boolean ignoreError;
extern char flashid[5];
extern int manufacturerid;
extern bool wrapped;
extern volatile uint8_t cart_activity;

// Variable to count errors
extern unsigned long writeErrors;

extern char romName[64];
extern unsigned long sramSize;
extern int romType;
extern byte saveType;
extern word romSize;
extern word numBanks;
extern char checksumStr[5];

// SD Card
extern byte sdBuffer[512];

int getSystick();
void SysClockInit();
void SysTick_Handler(void);
void delay(int n);
void ResetSystem();
void SysClockFree();

void delayMicroseconds(uint16_t us);
int findHighestFolder(const char* basePath);

void Set_Cart_Activity(uint8_t activity_type);
uint8_t GetGBType();

#endif
