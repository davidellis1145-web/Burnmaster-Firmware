/*********************************************************************
*					 SEGGER Microcontroller GmbH					 *
*						 The Embedded Experts						 *
**********************************************************************

**********************************************************************
*	Based on Sanni's cartreader firmware.							 *
*	 (https://github.com/sanni/cartreader)							 *
*																	 *
*	  Modified by FunnyPlaying and others.							 *
*	  Final modifications by: David Ellis.							 *
*	   (https://github.com/davidellis1145-web)						 *
*--------------------------------------------------------------------*
* Special thanks to;												 *
*					 Martin Refseth (HDR)							 *
*					 Slade1972										 *
*					 Dart-Alex										 *
*					 BennyFischer									 *
*					 Mom						...And FunnyPlaying  *
*********************************************************************/

#include <gd32f10x.h>
#include "fatfs/ff.h"
#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "Common.h"
#include "Display.h"
#include "Operate.h"
#include "GB.h"
#include "GBA.h"
#include "fatfs/sdcard.h"
#include "gd32f10x_sdio.h"
#include "flashparam.h"

/* PC5 corresponds to the 3.3v (GBA) cart voltage setting (active low)
PB0 corresponds to the 5v (GB) cart voltage setting (active low)*/

#define TYPE_GBC (0)
#define TYPE_GBA (1)
#define TYPE_ALL (2)
#define TYPE_NONE (3)

uint8_t GetGBType()
{
	uint8_t s3v3 = gpio_input_bit_get(GPIOC,GPIO_PIN_5);
	uint8_t s5v = gpio_input_bit_get(GPIOB,GPIO_PIN_0);
	uint8_t ret = TYPE_NONE;

	if(s3v3 == RESET)ret = TYPE_GBA;
	if(s5v == RESET)ret = TYPE_GBC;
	if((s3v3 | s5v) == RESET)ret = TYPE_ALL;

	return ret;
}


// Icons data
uint8_t Icon_data_GBC[] =
{
	0x00,0xFC,0x02,0x02,0xE2,0x12,0x12,0x12,0x12,0x92,0x92,0x92,0x92,0x92,0x92,0x92,0x92,0x92,0x92,0x92,0x92,0x92,0x12,0x12,0x12,0x12,0xE2,0x02,0x02,0xFC,
	0x00,0xFF,0x00,0x00,0xFF,0x00,0x04,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0xFF,
	0x00,0xFF,0x00,0x00,0x7F,0x40,0x80,0x80,0x80,0x1F,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x1F,0x80,0x80,0x80,0x40,0x7F,0x00,0x00,0xFF,
	0x00,0xFF,0x00,0x00,0x00,0xC0,0xC0,0xF0,0xF0,0xC1,0xC1,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0xC1,0xC1,0xC1,0x01,0x00,0x70,0x70,0x70,0x00,0x00,0x00,0xFF,
	0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x03,0x03,0x00,0x00,0x40,0x40,0x40,0x00,0x00,0x40,0x40,0x41,0x01,0x01,0x00,0x80,0x40,0xA0,0x40,0xA0,0x40,0x00,0xFF,
	0x00,0x0F,0x10,0x20,0x20,0x20,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x42,0x45,0x42,0x25,0x22,0x21,0x10,0x0F
};

uint8_t Icon_data_GBA[] =
{
	0x00,0xC0,0xF8,0x48,0x6C,0x34,0x16,0x1A,0x0A,0x0A,0x06,0x06,0x02,0x01,0xE1,0xE1,0x31,0x31,0x39,0x39,0x39,0x39,0x39,0x39,0x39,0x39,0x39,0x39,0x39,0x39,0x39,0x39,
	0x39,0x39,0x39,0x39,0x39,0x39,0x31,0x31,0xE1,0xE1,0x01,0x02,0x86,0x86,0x0A,0x0A,0x1A,0x16,0x34,0x6C,0x48,0xF8,0xC0,0x00,0x00,0xFF,0x00,0x00,0x00,0x30,0x30,0xFC,
	0xFC,0x30,0x30,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0xFF,0xFF,0x00,0x00,0xC1,0xC1,0xC0,0x00,0x00,0x70,0x70,0x70,0x00,0x00,0xFF,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xD8,0xD8,0x00,0x00,0x00,0xFF,0xFF,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,0xA1,0xA1,0xA1,0x50,
	0x50,0x50,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x01,0x02,0x04,0x08,0x08,0x08,0x10,0x10,0x10,0x10,0x20,0x20,0x20,0x23,0x47,0x46,0x4E,0x4E,0x4E,0x4E,0x4E,0x4E,0x4E,
	0x4E,0x4E,0x4E,0x4E,0x4E,0x4E,0x4E,0x4E,0x4E,0x4E,0x4E,0x4E,0x4E,0x4E,0x4E,0x46,0x47,0x23,0x20,0x20,0x22,0x12,0x12,0x11,0x11,0x09,0x08,0x08,0x04,0x02,0x01,0x00
};

uint8_t Icon_data_DGE[] =
{
	0xFE,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0xB2,0xB2,0xB2,0xB2,0xB2,0xB2,0xB2,0xB2,0x02,0x32,0x32,0x32,0x32,0x32,0x02,0xB2,0xB2,0xB2,0xB2,0xB2,
	0xB2,0xB2,0xB2,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0xFE,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,
	0x01,0x01,0x01,0x01,0x00,0x00,0x20,0x70,0x20,0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,
	0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x70,0x00,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x00,0x70,0x20,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0xFC,0x04,0x04,0x04,0xF8,0x00,0x90,0x50,0x50,0xE0,
	0x00,0x00,0x70,0x80,0x00,0x80,0x70,0x00,0xE0,0x50,0x50,0x50,0x20,0x00,0x00,0x10,0x0C,0x04,0x00,0x20,0x50,0x50,0x50,0x90,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,
	0xFF,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x00,0x00,0x78,0x85,0xA5,0xA4,0xE9,0x00,0x48,0xA8,0xA9,0x70,0x80,0x00,0xF0,0x09,0xF1,0x09,0xF0,0x00,0x70,0xA8,
	0xA8,0xA8,0x10,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0xFF,0x80,0x80,0x80,0x9F,0x95,0x95,0x91,0x80,0x9C,0x82,0x9C,0x82,0x9C,0x80,0x9E,
	0x8A,0x8A,0x84,0x80,0x8C,0x92,0x92,0x8C,0x80,0x82,0x9C,0x82,0x84,0x80,0x9D,0x80,0x8E,0x90,0x90,0x8E,0x90,0x80,0x9C,0x82,0x9C,0x82,0x9C,0x80,0x80,0x80,0xFF,0x00
};


// Game Boy Menu
/**********************
  Menu
**********************/
// GBx start menu
static const char gbxMenuItem1[] = "Game Boy (Color)";
static const char gbxMenuItem2[] = "Game Boy Advance";
static const char gbxMenuTests[] = "Cart Tests";
static const char gbxMenuTestAll[] = "Full Test";
static const char gbxMenuTestFast[] = "Quick Test";
static const char gbxAbout[] = "About...";
static const char gbxReset[] = "Reset";
static const char gbxMenuItemDebug[] = "Debug Menu";
static const char gbxDebug1[] = "String fnt_sz 8";
static const char gbxDebug2[] = "String fnt_sz 16";
static const char gbxDebug3[] = "String Y over-run";
static const char gbxDebug4[] = "ShowPic over-run";

static const char* const menuOptionsGBC[] = {gbxMenuItem1,gbxMenuTests,gbxAbout,gbxMenuItemDebug};
static const char* const menuOptionsGBA[] = {gbxMenuItem2,gbxMenuTests,gbxAbout,gbxMenuItemDebug};
static const char* const menuOptionsGBx[] = {gbxMenuItem1,gbxMenuItem2,gbxMenuItemDebug};
static const char* const menuOptionsGBT[] = {gbxMenuTestFast,gbxMenuTestAll,gbxReset};
static const char* const menuOptionsDebug[] = {gbxDebug1,gbxDebug2,gbxDebug3,gbxDebug4,gbxReset};


void aboutScreen()
{
	OledClear();
	OledShowString(0,0,"Game Boy",16);
	OledShowPicData(80,0,48,6,Icon_data_DGE);
	OledShowString(3,2,"Flash Master",8);
	OledShowString(8,4,"v1.0.4-a.9",8);
	OledShowString(2,5,"Aug 10, 2026",8);
	OledShowString(0,7,"Press OK Button...",8);
	WaitOKBtn();
}


// Start menu for both GB and GBA (main menu)
uint8_t gbxMenu()
{
	uint8_t bret = 0;
	uint8_t gbxtype = GetGBType();
	unsigned char gbType;

	if(gbxtype == TYPE_GBA)
	{
		LED_GREEN_OFF;	// Make sure GB mode led is off
		LED_BLUE_ON;	// Make sure GBA mode led is on
		OledClear();
		
		// Create menu with title and options to choose from
		// Wait for user choice to come back from the question box menu
		gbType = questionBox_OLED("Game Boy Flash Master", menuOptionsGBA, 4, 1, 1, 0);
		
		// Draw GBA icon here to avoid questionBox_OLED clearing it 
		OledShowPicData(70,3,56,4,Icon_data_GBA);
		
		switch (gbType)
		{
			case 0:	// Cancel btn clicked
				bret = 1;
				break;
			case 1:
				gbaScreen();
				break;
			case 2:
				gbaTestsScreen();
				break;
			case 3:
				aboutScreen();
				break;
			case 4:
				gbxDebugScreen();
				break;
		}
	}
	else if(gbxtype == TYPE_GBC)
	{
		LED_BLUE_OFF;	// Make sure GBA mode led is off
		LED_GREEN_ON;	// Make sure GB mode led is on
		OledClear();
		
		// Create menu with title and options to choose from
		// Wait for user choice to come back from the question box menu
		gbType = questionBox_OLED("Game Boy Flash Master", menuOptionsGBC, 4, 1, 1, 0);
		
		// Draw GBC icon here to avoid QuestionBox_OLED clearing it
		OledShowPicData(97,2,30,6,Icon_data_GBC);
		
		switch (gbType)
		{
			case 0:	// Cancel btn clicked
				bret = 1;
				break;
			case 1:
				gbScreen();
				break;
			case 2:
				gbTestsScreen();
				break;
			case 3:
				aboutScreen();
				break;
			case 4:
				gbxDebugScreen();
				break;
		}
	}
	else
	{
		gbType = questionBox_OLED("Game Boy Flash Master", menuOptionsGBx, 3, 1, 1, 1);
		switch (gbType)
		{
			case 0:	// Cancel btn clicked
				bret = 1;
				break;
			case 1:
				gbScreen();
				break;
			case 2:
				gbaScreen();
				break;
			case 3:
				gbxDebugScreen();
				break;
		}
	}
	return bret;
}


uint8_t gbTestsMenu(uint8_t skipWarning)
{
	// Only show warning if skipWarning is 0
	if (!skipWarning)
	{
		OledClear();
		OledShowString(30,1,"**WARNING**",8);
		OledShowString(3,3,"The following tests",8);
		OledShowString(2,4,"will erase the cart!",8);
		OledShowString(0,7,"Press OK Button...",8);
		WaitOKBtn();
	}

	OledClear();
	
	uint8_t bret = 0;

	// Create menu with title and options to choose from
	unsigned char gbCTest;
	gbCTest = questionBox_OLED("GB(C) Cart Tests", menuOptionsGBT, 3, 1, 1, 1);
	// Wait for user choice to come back from the question box menu
	switch (gbCTest)
	{
		case 0: // Cancel btn pressed
			bret = 1;
			break;
		case 1:
			TestMemGB(true);
			break;
		case 2:
			TestMemGB(false);
			break;
		case 3:
			ResetSystem();
			break;
	}
	return bret;
}


uint8_t gbaTestsMenu(uint8_t skipWarning)
{
	// Only show warning if skipWarning is 0
	if (!skipWarning)
	{
		OledClear();
		OledShowString(30,1,"**WARNING**",8);
		OledShowString(3,3,"The following tests",8);
		OledShowString(2,4,"will erase the cart!",8);
		OledShowString(0,7,"Press OK Button...",8);
		WaitOKBtn();
	}

	OledClear();
	
	uint8_t bret = 0;

	// Create menu with title and options to choose from
	unsigned char gbaCTest;
	gbaCTest = questionBox_OLED("GBA Cart Tests", menuOptionsGBT, 3, 1, 1, 1);

	// Wait for user choice to come back from the question box men
	switch (gbaCTest)
	{
		case 0: // Cancel btn pressed
			bret = 1;
			break;
		case 1:
			TestMemGBA(true);
			break;
		case 2:
			TestMemGBA(false);
			break;
		case 3:
			ResetSystem();
			break;
	}
	return bret;
}


uint8_t gbxDebugMenu(uint8_t skipWarning)	// Debug Menu for dev testing
{
	if (!skipWarning) // Only show warning if skipWarning is 0
	{
		OledClear();
		OledShowString(30,1,"**WARNING**",8);
		OledShowString(0,3,"Entering debug menu",8);
		OledShowString(0,4,"don't use important",8);
		OledShowString(0,5,"carts, may corrupt.",8);
		OledShowString(0,7,"Press OK Button...",8);
		WaitOKBtn();
	}

	OledClear();
	uint8_t bret = 0;

	// Create menu with title and options to choose from
	unsigned char gbxDebug;
	gbxDebug = questionBox_OLED("**Debug Menu**", menuOptionsDebug, 5, 1, 1, 1);

	// Wait for user choice to come back from the question box menu
	switch (gbxDebug)
	{
		case 0: // Cancel btn pressed
			bret = 1;
			break;
		case 1:
			OledClear();
			OledShowString(0,0,"This string is too long",8);
			OledShowString(0,7,"Press OK...",8);
			WaitOKBtn();
			OledClear();
			OledShowString(0,0,"This string is too long\nand it has two newlines\nNewline two",8);
			OledShowString(0,7,"Press OK...",8);
			WaitOKBtn();
			break;
		case 2:
			OledClear();
			OledShowString(0,0,"This string is too long",16);
			OledShowString(0,7,"Press OK...",16);
			WaitOKBtn();
			OledClear();
			OledShowString(0,0,"This string is too long\nThis Newline is too long\nNewline",16);
			OledShowString(0,7,"Press OK...",16);
			WaitOKBtn();
			break;
		case 3:
			OledClear();
			OledShowString(0,0,"1",8);
			OledShowString(0,1,"2",8);
			OledShowString(0,2,"3",8);
			OledShowString(0,3,"4",8);
			OledShowString(0,4,"5",8);
			OledShowString(0,5,"6",8);
			OledShowString(0,6,"7",8);
			OledShowString(0,7,"8....Press OK button",8);
			OledShowString(0,8,"out of bounds",8);
			WaitOKBtn();
			OledClear();
			OledShowString(0,0,"This line is just a bit long",8);
			OledShowString(0,1,"This line is 21 chars\nwith a newline",8);
			OledShowString(0,3,"And this has too many\nnewlines\n5\n6 Press OK...\n7\noverflow",8);
			WaitOKBtn();
			break;
		case 4:
			OledClear();
			OledShowPicData(100,0,48,6,Icon_data_DGE);
			OledShowString(0,7,"Press OK...",8);
			WaitOKBtn();
			OledClear();
			OledShowPicData(80,4,48,6,Icon_data_DGE);
			OledShowString(0,7,"Press OK...",8);
			WaitOKBtn();
			break;
		case 5:
			ResetSystem();
			break;
	}
	return bret;
}


/**********************
  Menu to display
**********************/
void gbxScreen() // Main menu
{
	while(1)
	{
		uint8_t b = gbxMenu();
		if(b>0)break;
	}
}


void gbTestsScreen() // Cart tests for GB(C)
{
	uint8_t skip = 0; // Don't skip on first run
	while(1)
	{
		uint8_t b = gbTestsMenu(skip);
		if (b > 0)
		{
			break;
		}
		// Case 1+... set skip to 1 so it won't show on next loop
		skip = 1;
	}
}


void gbaTestsScreen() // Cart tests for GBA
{
	uint8_t skip = 0; // Don't skip on first run
	while(1)
	{
		uint8_t b = gbaTestsMenu(skip);
		if (b > 0)
		{
			ResetSystem();
			break;
		}
		// Case 1+... set skip to 1 so it won't show on next loop
		skip = 1;
	}
}


void gbxDebugScreen() // Debug menu loader
{
	uint8_t skip = 0; // Don't skip on first run
	while(1)
	{
		uint8_t b = gbxDebugMenu(skip);
		if (b > 0)
		{
			ResetSystem();
			break;
		}
		// Case 1+... set skip to 1 so it won't show on next loop
		skip = 1;
	}
}


/**************************
  SD Card Init Functions
**************************/

/*!
	\brief		initialize the card, get the card information, set the bus mode and transfer mode
	\param[in]	none
	\param[out] none
	\retval		sd_error_enum
*/
sd_error_enum sd_io_init(void)
{
	sd_error_enum status = SD_OK;
	uint32_t cardstate = 0;
	status = sd_init();
	if(SD_OK == status)
	{
		status = sd_card_information_get(&sd_cardinfo);
	}
	if(SD_OK == status)
	{
		status = sd_card_select_deselect(sd_cardinfo.card_rca);
	}
	status = sd_cardstatus_get(&cardstate);
	if(cardstate & 0x02000000)
	{
		printf("\r\n the card is locked!");
		while (1)
		{
		}
	}
	if ((SD_OK == status) && (!(cardstate & 0x02000000)))
	{
	}
	if (SD_OK == status)
	{
		// Set data transfer mode
		status = sd_transfer_mode_config( SD_POLLING_MODE );
		printf("Set SD to Polling Mode.\r\n");
	}
	return status;
}


/*!
	\brief		get the card information and print it out by USRAT
	\param[in]	none
	\param[out] none
	\retval		none
*/
void card_info_get(void)
{
	uint8_t sd_spec, sd_spec3, sd_spec4, sd_security;
	uint32_t block_count, block_size;
	uint16_t temp_ccc;
	printf("\r\n Card information:");
	sd_spec = (sd_scr[1] & 0x0F000000) >> 24;
	sd_spec3 = (sd_scr[1] & 0x00008000) >> 15;
	sd_spec4 = (sd_scr[1] & 0x00000400) >> 10;
	if(2 == sd_spec)
	{
		if(1 == sd_spec3)
		{
			if(1 == sd_spec4)
			{
				printf("\r\n## Card version 4.xx ##");
			}
			else
			{
				printf("\r\n## Card version 3.0x ##");
			}
		}
		else
		{
			printf("\r\n## Card version 2.00 ##");
		}
	}
	else if(1 == sd_spec)
	{
		printf("\r\n## Card version 1.10 ##");
	}
	else if(0 == sd_spec)
	{
		printf("\r\n## Card version 1.0x ##");
	}

	sd_security = (sd_scr[1] & 0x00700000) >> 20;
	if(2 == sd_security)
	{
		printf("\r\n## SDSC card ##");
	}
	else if(3 == sd_security)
	{
		printf("\r\n## SDHC card ##");
	}
	else if(4 == sd_security)
	{
		printf("\r\n## SDXC card ##");
	}

	block_count = (sd_cardinfo.card_csd.c_size + 1)*1024;
	block_size = 512;
	printf("\r\n## Device size is %dKB ##", sd_card_capacity_get());
	printf("\r\n## Block size is %dB ##", block_size);
	printf("\r\n## Block count is %d ##", block_count);

	if(sd_cardinfo.card_csd.read_bl_partial)
	{
		printf("\r\n## Partial blocks for read allowed ##" );
	}
	if(sd_cardinfo.card_csd.write_bl_partial)
	{
		printf("\r\n## Partial blocks for write allowed ##" );
	}
	temp_ccc = sd_cardinfo.card_csd.ccc;
	printf("\r\n## CardCommandClasses is: %x ##", temp_ccc);
	if((SD_CCC_BLOCK_READ & temp_ccc) && (SD_CCC_BLOCK_WRITE & temp_ccc))
	{
		printf("\r\n## Block operation supported ##");
	}
	if(SD_CCC_ERASE & temp_ccc)
	{
		printf("\r\n## Erase supported ##");
	}
	if(SD_CCC_WRITE_PROTECTION & temp_ccc)
	{
		printf("\r\n## Write protection supported ##");
	}
	if(SD_CCC_LOCK_CARD & temp_ccc)
	{
		printf("\r\n## Lock unlock supported ##");
	}
	if(SD_CCC_APPLICATION_SPECIFIC & temp_ccc)
	{
		printf("\r\n## Application specific supported ##");
	}
	if(SD_CCC_IO_MODE & temp_ccc)
	{
		printf("\r\n## I/O mode supported ##");
	}
	if(SD_CCC_SWITCH & temp_ccc)
	{
		printf("\r\n## Switch function supported ##");
	}
}


void SDCardInit()
{
	FRESULT res_sd;
	sd_error_enum sd_error;
	uint16_t i = 5;
	// Initialize the card
	do
	{
		sd_error = sd_io_init();
	}
	while((SD_OK != sd_error) && (--i));

	if(i)
	{
		printf("\r\nSD Card init success!\r\n");
		SD_Status = sd_error;
	}
	else
	{
		ignoreError = 0;
		print_Error("No SD Card detected!",true);
	}

	// Get the information of the card and print it out by USART
	card_info_get();

	// When mounting a FS on external SPI flash, it is initialized during mounting
	res_sd = f_mount(&fs,"",1);

	/*---------------- Formatting Test --------------------
		If no file system exists, format and create one */
	if(res_sd == FR_NO_FILESYSTEM)
	{
		printf("\r\n!No File System...");
	}
	else if(res_sd!=FR_OK)
	{
		printf("\r\n!Mount Failed(%d)",res_sd);
		while(1)
		{
		}
	}
	else
	{
		printf("\r\nMount OK!%d\r\n",res_sd);
	}
}


void PriInit()
{
	SysClockInit();
	__enable_irq();

	rcu_periph_clock_enable(RCU_GPIOA);
	rcu_periph_clock_enable(RCU_GPIOB);
	rcu_periph_clock_enable(RCU_GPIOC);
	rcu_periph_clock_enable(RCU_GPIOD);
	rcu_periph_clock_enable(RCU_GPIOE);

	gpio_init(GPIOA,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_2MHZ,GPIO_PIN_ALL);
	gpio_init(GPIOB,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_2MHZ,GPIO_PIN_ALL);
	gpio_init(GPIOC,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_2MHZ,GPIO_PIN_ALL);
	gpio_init(GPIOD,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_2MHZ,GPIO_PIN_ALL);
	gpio_init(GPIOE,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_2MHZ,GPIO_PIN_ALL);

	rcu_periph_clock_enable(RCU_AF);
	foldern = 0;
}


int main(void)
{
	PriInit();
	LEDSInit();
	OledInit();
	KeyBrdInit();
	SDCardInit();
	delay(200);
	gbxScreen();
	f_mount(NULL,"",1);
	SysClockFree();
	exit(EXIT_SUCCESS);
}
