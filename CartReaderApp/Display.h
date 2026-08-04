#pragma once

#define I2C1_SLAVE_ADDRESS7		0x78
#define SSD1306_ADDR			0x3c
#define MAX_COLUMN				127

#define LED1	(1)
#define LED_B	(2)
#define LED_G	(4)
#define LED_R	(8)

void I2cInit(void);
void SSD1306_WriteCmd(uint8_t var);
void SSD1306_WriteData(uint8_t var);

// Coordinate settings (where to display)
void OledSetPos(uint8_t x, uint8_t y);
// Enable OLED display
void OledDisplayOn(void);
// Turn off OLED display
void OledDisplayOff(void);
// Clears entire screen, making it all black as if it was not on
void OledClear(void);
void OledClearLine(uint8_t y);
// Displays a character, or part of one, at specified position
// x:0-127,y:0-7
// Char_Size: font size 16/8
uint8_t OledShowChar(uint8_t x,uint8_t y,uint8_t chr,uint8_t Char_Size);
// Display a string
uint8_t OledShowString(uint8_t x,uint8_t y,const char *str,uint8_t Char_Size);
// ShowChar for QBoxShowString
void QBoxShowChar(uint8_t x,uint8_t y,uint8_t chr);
// ShowString for QuestionBox (prevents wrapping for side scroll)
uint8_t QBoxShowString(uint8_t x,uint8_t y,const char *str,uint8_t scroll_offset);
// Display an image at specified location
void OledShowPicData(uint8_t x,uint8_t y,uint8_t wdt,uint8_t hgt,uint8_t *pPicData);
// Initialize display
void OledInit(void);

void print_Error(char *errorMessage, uint8_t forceReset);
void draw_progressbar(uint32_t processed, uint32_t total, uint8_t line);
void showPercent(uint32_t processed, uint32_t total, uint8_t x, uint8_t line);

// Leds
void LEDSInit();
void LED_ON(uint8_t LedNum);
void LED_OFF(uint8_t LedNum);
void LED_BLINK(uint8_t LedNum);
void LED_CLEAR(void);

#define LED_RED_ON LED_ON(LED_R)
#define LED_GREEN_ON LED_ON(LED_G)
#define LED_BLUE_ON LED_ON(LED_B)
#define LED_RED_OFF LED_OFF(LED_R)
#define LED_GREEN_OFF LED_OFF(LED_G)
#define LED_BLUE_OFF LED_OFF(LED_B)
#define LED_RED_BLINK LED_BLINK(LED_R)
#define LED_GREEN_BLINK LED_BLINK(LED_G)
#define LED_BLUE_BLINK LED_BLINK(LED_B)