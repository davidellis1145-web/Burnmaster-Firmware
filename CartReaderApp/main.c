/*********************************************************************
*                    SEGGER Microcontroller GmbH                     *
*                        The Embedded Experts                        *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------
*/
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


//PC5是卡带3V3电压档位，低电平有效，PB0是卡带5V电压档位，低电平有效

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

  return  ret;
}


// Icons data
uint8_t Icon_data_GBC[] =
{
0xFC, 0x02, 0x02, 0xF2, 0x12, 0x12, 0x12, 0xD2, 0xD2, 0xD2, 0xD2, 0xD2, 0xD2, 0xD2, 0xD2, 0xD2, 0xD2, 0xD2, 0xD2, 0xD2, 0xD2, 0xD2, 0x12, 0x12, 0x12, 0xE2, 0x02, 0x06, 0xFC,
0xFF, 0x00, 0x00, 0xFF, 0x18, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF,
0xFF, 0x00, 0x00, 0x7F, 0x80, 0x80, 0x80, 0xBF, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xBF, 0x80, 0x80, 0x40, 0x3F, 0x00, 0x00, 0xFF,
0xFF, 0x00, 0x00, 0x80, 0x80, 0xE0, 0x20, 0xE0, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x40, 0xC0, 0x00, 0x00, 0xFF,
0xFF, 0x00, 0x00, 0x03, 0x02, 0x0E, 0x08, 0x0E, 0x02, 0x03, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x07, 0x05, 0x07, 0x00, 0x00, 0x01, 0x41, 0x81, 0x00, 0x00, 0xFF,
0x3F, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x42, 0x41, 0x40, 0x40, 0x42, 0x41, 0x40, 0x40, 0x40, 0x40, 0x44, 0x48, 0x51, 0x42, 0x44, 0x60, 0x31, 0x18, 0x0F
};

uint8_t Icon_data_GBA[] =
{
0x00, 0xC0, 0xF8, 0x48, 0x6C, 0x34, 0x16, 0x1A, 0x0A, 0x0A, 0x06, 0x06, 0x02, 0x01, 0xE1, 0xE1, 0x31, 0x31, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39,
0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x31, 0x31, 0xE1, 0xE1, 0x01, 0x02, 0x86, 0x86, 0x0A, 0x0A, 0x1A, 0x16, 0x34, 0x6C, 0x48, 0xF8, 0xC0, 0x00,
0x00, 0xFF, 0x00, 0x00, 0x00, 0x30, 0x30, 0xFC, 0xFC, 0x30, 0x30, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xC1, 0xC1, 0xC0, 0x00, 0x00, 0x70, 0x70, 0x70, 0x00, 0x00, 0xFF, 0x00,
0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8, 0xD8, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xA1, 0xA1, 0xA1, 0x50, 0x50, 0x50, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00,
0x00, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08, 0x10, 0x10, 0x10, 0x10, 0x20, 0x20, 0x20, 0x23, 0x47, 0x46, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E,
0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x46, 0x47, 0x23, 0x20, 0x20, 0x22, 0x12, 0x12, 0x11, 0x11, 0x09, 0x08, 0x08, 0x04, 0x02, 0x01, 0x00
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

static const char* const menuOptionsGBC[] = {gbxMenuItem1,gbxMenuTests,gbxAbout};
static const char* const menuOptionsGBA[] = {gbxMenuItem2,gbxMenuTests,gbxAbout};
static const char* const menuOptionsGBx[] = {gbxMenuItem1,gbxMenuItem2};
static const char* const menuOptionsGBT[] = {gbxMenuTestFast,gbxMenuTestAll,gbxReset};


void aboutScreen()
{
  OledClear();
  OledShowString(0,0,(char *)("Game Boy"),16); // font size 16 takes 2 lines
  OledShowString(5,2,(char *)("Flash Master"),16);
  // Contains custom integration for Spansion S29GL128N | S29GL256N | S29GL512N
  // Based on Funnyplaying v1.10 release.
  OledShowString(20,4,(char *)("Ver:1.02"),8);
  OledShowString(20,5,(char *)("Mar. 28, 2026"),8);
  OledShowString(3,6,(char *)("Dave's Game Emporium"),8);
  OledShowString(0,7,(char *)("Press OK Button..."),8);
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
    LED_GREEN_OFF; // make sure GB mode led is off LED_CLEAR() not working...
    LED_BLUE_ON;   // make sure GBA mode led is on
    OledClear();
    OledShowPicData(64,4,56,4,Icon_data_GBA); // draws GBA icon
	
	// create menu with title and 3 options to choose from  
    // wait for user choice to come back from the question box menu
    gbType = questionBox_OLED("Game Boy Flash Master", menuOptionsGBA, 3, 1, 1, 0);    
    switch (gbType)
    {
      case 0:  // cancel btn clicked
        gbxScreen();
        break;
      case 1:
        gbaScreen();
        break;
      case 2:
        gbaTestsScreen(); //testing test screen for gba
        break;
      case 3:
        aboutScreen();
        break;
    }
  }
  else if(gbxtype == TYPE_GBC)
  {
    LED_BLUE_OFF;  // make sure GBA mode led is off
    LED_GREEN_ON;  // make sure GB mode led is on
    OledClear();
    OledShowPicData(86,2,29,6,Icon_data_GBC);  // draws GB icon

	// create menu with title and 3 options to choose from  
    // wait for user choice to come back from the question box menu
    gbType = questionBox_OLED("Game Boy Flash Master", menuOptionsGBC, 3, 1, 1, 0);    
    switch (gbType)
    {
      case 0:  // cancel btn clicked
        gbxScreen();
        break;
      case 1:
        gbScreen();
        break;
      case 2:
        gbTestsScreen(); //testing test screen for gb
        break;
      case 3:
        aboutScreen();
        break;
    }
  }
  else 
  {
    gbType = questionBox_OLED("Game Boy Flash Master", menuOptionsGBx, 2, 1, 1, 1);

    switch (gbType)
    {
      case 0:  // cancel btn clicked
        gbxScreen();
        break;
      case 1:
        gbScreen();
        break;
      case 2:
        gbaScreen();
        break;
    }
  }
  return  bret;
}


uint8_t gbTestsMenu() // gb tests menu testing
{
  // Show warning
  OledClear();
  OledShowString(35,1,(char *)("**WARNING**"),8);
  OledShowString(3,2,(char *)("The following tests"),8);
  OledShowString(2,3,(char *)("will erase the cart!"),8);
  OledShowString(0,7,(char *)("Press OK Button..."),8);
  WaitOKBtn();
  OledClear();

  // create menu with title and 3 options to choose from
  unsigned char gbCTest;
  gbCTest = questionBox_OLED("GB(C) Cart Tests", menuOptionsGBT, 3, 1, 1, 0);

  // wait for user choice to come back from the question box menu
  switch (gbCTest)
  {
    case 0: // cancel btn pressed
      gbxScreen();
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
  return 0;
}


uint8_t gbaTestsMenu() //gba tests menu testing
{
  // Show warning
  OledClear();
  OledShowString(35,1,(char *)("**WARNING**"),8);
  OledShowString(3,2,(char *)("The following tests"),8);
  OledShowString(2,3,(char *)("will erase the cart!"),8);
  OledShowString(0,7,(char *)("Press OK Button..."),8);
  WaitOKBtn();
  OledClear();

  // create menu with title and 3 options to choose from
  unsigned char gbaCTest;
  gbaCTest = questionBox_OLED("GBA Cart Tests", menuOptionsGBT, 3, 1, 1, 0);

  // wait for user choice to come back from the question box menu
  switch (gbaCTest)
  {
    case 0: // cancel btn pressed
      gbxScreen();
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
  return 0;
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
  while(1)
  {
    uint8_t b = gbTestsMenu();
    
    if(b>0)break;
  }
}


void gbaTestsScreen() // Cart tests for GBA
{
  while(1)
  {
    uint8_t b = gbaTestsMenu();
    
    if(b>0)break;
  }
}


/**************************
  SD Card Init Functions
**************************/

/*!
    \brief      initialize the card, get the card information, set the bus mode and transfer mode
    \param[in]  none
    \param[out] none
    \retval     sd_error_enum
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
        // set data transfer mode
        status = sd_transfer_mode_config( SD_POLLING_MODE );
        printf("Set SD to Polling Mode.\r\n");
    }
    return status;
}

/*!
    \brief      get the card information and print it out by USRAT
    \param[in]  none
    \param[out] none
    \retval     none
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
  // initialize the card
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
  
  // get the information of the card and print it out by USART
  card_info_get();

  //在外部SPI Flash挂载文件系统，文件系统挂载时会对SPI设备初始化
  res_sd = f_mount(&fs,"",1);


  /*----------------------- 格式化测试 ---------------------------  
     如果没有文件系统就格式化创建创建文件系统 */
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
  foldern = load_dword();
}

int main(void) 
{
  PriInit();
  LEDSInit();
  OledInit();
  OledClear();
  KeyBrdInit();
  SDCardInit();
  delay(200);
  gbxScreen();
  f_mount(NULL,"",1);
  SysClockFree();
  exit(EXIT_SUCCESS);
}
