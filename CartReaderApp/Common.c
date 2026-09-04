#include "Common.h"

// SD Card
FIL myDir;
FIL myFile;
// Array that holds the data
FATFS fs;
byte sdBuffer[512];

// Remember folder number to create a new folder for every save
int foldern;

// File browser
char filePath[FILEPATH_LENGTH];
word currPage;
word lastPage;
word numPages;
boolean root = 0;

// Common
char romName[64];
unsigned long sramSize = 0;
int romType = 0;
int manufacturerid = 0;
byte saveType;
word romSize = 0;
word numBanks = 128;
char checksumStr[5];
bool errorLvl = 0;
boolean ignoreError = 0;
char flashid[5];
bool wrapped = false;
bool all_clear = 0;
bool alreadyWaited = 0;
char targetFolder[128];
char targetFile[256];

// Variable to count errors
unsigned long writeErrors;

/********************************************************************************
 * Find the highest numbered folder in the given path							*
 * Example: If folders 0, 1, 3 exist in "GB/SAVE/GAME/", returns 3				*
 * Returns -1 if no folders found, otherwise returns the highest folder number	*
 * Note: Functions that call this use (foldern = (return value) + 1) -1 + 1 = 0	*
 * Note: Generated with AI assistance (GitHub Copilot) on 2/24/26				*
 * Note: Reveiwed by David Ellis on 9/1/26										*
 ********************************************************************************/
int findHighestFolder(const char* basePath)
{
	DIR dir;
	FILINFO finfo;
	int maxFolder = -1;

	// Debugging...
	OledClearLine(0);
	OledShowString(0, 0, "bPath =", 8);
	OledShowString(42, 0, basePath, 8);
	WaitOKBtn(1);
	OledClear();
	// ...End

	if (f_opendir(&dir, basePath) != FR_OK)
	{
		OledShowString(0, 0, "bummer dude...", 8);
		WaitOKBtn(1);
		return -1; // Directory doesn't exist yet
	}

	while (f_readdir(&dir, &finfo) == FR_OK && finfo.fname[0] != '\0')
	{
		if ((finfo.fattrib & AM_DIR) && (finfo.fname[0] >= '0' && finfo.fname[0] <= '9'))
		{
			int folderNum = atoi(finfo.fname);

			if (folderNum > maxFolder)
			{
				maxFolder = folderNum;
			}
		}
	}
	f_closedir(&dir);
	sprintf(tmsg, "maxFolder = %d", maxFolder);
	OledShowString(0, 0, "tmsg", 8);
	WaitOKBtn(1);
	
	return maxFolder;
}


/**********************
  System base parts
**********************/

static volatile int ticks = 0;
volatile uint8_t cart_activity = 0; // 0 = Idle, 1 = Reading, 2 = Writing

void SysClockInit()
{
	// Ensure SystemCoreClock is up-to date
	SystemCoreClockUpdate();

	// Enable SysTick ms timer
	SysTick->LOAD = (SystemCoreClock / 1000) - 1;
	SysTick->VAL = 0;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;

	// Enable precision DWT hardware tracker for delayMicroseconds(us)
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CYCCNT = 0;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}


void SysTick_Handler(void)
{
	ticks++;
}


int getSystick()
{
	return ticks;
}


void delay(int n)
{
	int startTicks = ticks;
	while ((ticks - startTicks) < n)
	{
		// Wait for ms to pass
	}
}


void ResetSystem()
{
	OledClear();		// Keeps display from spazzing on system reset
	delay(10);
	__set_FAULTMASK(1); // Disable global interrupts
	NVIC_SystemReset(); // Request Restart
}


void SysClockFree()
{
	// Disable SysTick interrupt
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}


void delayMicroseconds(uint32_t us)
{
	// Calculate number of raw CPU cycles to wait
	uint32_t start_cycles = DWT->CYCCNT;
	uint32_t total_cycles = us * (SystemCoreClock / 1000000);

	// Block execution
	while ((DWT->CYCCNT - start_cycles) < total_cycles)
	{
		// Wait for cycles to pass
	}
}

void Set_Cart_Activity(uint8_t activity_type)
{
	cart_activity = activity_type;

	if (GetGBType() == TYPE_GBA)
	{
		LED_GREEN_ON;
	}
	else
	{
		LED_BLUE_ON;
	}

	if (activity_type == 1)		  // Reading
	{
		timer_autoreload_value_config(TIMER1, SPEED_READ);
	}
	else if (activity_type == 2)  // Writing
	{
		timer_autoreload_value_config(TIMER1, SPEED_WRITE);
	}
	else						  // Idle
	{
		// Reset the LEDs back to their proper idle states
		LED_RESET(0);
	}
}


void SetErrorLvl(uint8_t errorStat)
{
	if (errorStat)
	{
		errorLvl = 1; // Check if this would already be set by errorLvl argument in SetErrorLvl
		LED_CLEAR();
		//LED_RED_ON;
		timer_autoreload_value_config(TIMER1, SPEED_ERROR);
	}
	else
	{
		errorLvl = 0;
		LED_RESET(1);
	}
}
