#include <gd32f10x.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Common.h"
#include "Display.h"
#include "Operate.h"

/**********************
  Keyboard
**********************/
void KeyBrdInit()
{
	gpio_init(GPIOE, GPIO_MODE_IPU, GPIO_OSPEED_2MHZ, 0x3F);
	gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_2MHZ, GPIO_PIN_1); // Low battery handling
}


uint8_t keyState()
{
	return GPIO_ISTAT(GPIOE)&(0x3F);
}


uint8_t checkButton()
{
	uint8_t keycode = keyState();
	delay(44);

	if(gpio_input_bit_get(GPIOB,GPIO_PIN_1) == RESET)
	{
		LED_RED_ON;
	}
	else
	{
		LED_RED_OFF;
	}

	if(keyState() != keycode)
	{
		return (~keycode)&0x3F;
	}
	else
	{
		return BTNNONE;
	}
}


void WaitOKBtn()
{
	while(checkButton() != BTNOK)
	{
	}
}


// Display a question box with selectable answers. Make sure default choice is in (0, num_answers]
unsigned char questionBox_OLED(char * question, const char* const answers[7], int num_answers, int default_choice, uint8_t rollselect, uint8_t clrScr)
{
	// Clear the screen
	if(clrScr > 0)OledClear();

	// Print menu
	QBoxShowString(0,0,question);
	char tanswer[21] = {0};
	for (unsigned char i = 0; i < num_answers; i++)
	{
		memcpy(tanswer,answers[i],20);
		QBoxShowString(6,i+1,tanswer);
	}

	// Start with the default choice
	unsigned char choice = default_choice;
	unsigned char choice_ori = default_choice;

	// Draw selection bullet
	QBoxShowChar(0,choice,'*');
	uint8_t currentColor = 0;
	uint32_t scroll_tick = 0;
	uint8_t scroll_start = 0;

	// Wait until user makes their choice
	while (1)
	{
		int b = checkButton();
		if(b==BTNNONE)
		{
			scroll_tick = scroll_tick + 1;
			if((scroll_tick > 14) && (scroll_tick%3 == 1))
			{
				if(QBoxShowString(6,choice,answers[choice - 1] + scroll_start) > 0)
				{
					scroll_start++;
				}
			}
		}
		else
		{
			printf("getKey-%d\n",b);
			scroll_tick = 0;
			scroll_start = 0;
		}
		if(b==BTNLEFT)

		{
			if(rollselect)
			{
			}
			else
			{
				choice = MENU_PGUP;
				break;
			}
		}
		else if (b == BTNRIGHT)
		{
			if(rollselect)
			{
			}
			else
			{
				choice = MENU_PGDN;
				break;
			}
		}
		else if (b == BTNUP)
		{
			choice--;
			if(choice <= 0)
			{
				if(rollselect)
				{
					choice = num_answers;
				}
				else
				{
					choice = MENU_UPUP;
					break;
				}
			}
		}
		else if (b == BTNDOWN)
		{
			choice++;
			if(choice > num_answers)
			{
				if(rollselect)
				{
					choice = 1;
				}
				else
				{
					choice = MENU_DOWNDOWN;
					break;
				}
			}
		}
		else if (b == BTNCANCEL)
		{
			choice = MENU_CANCEL;
			break;
		}
		else if (b == BTNOK)
		{
			break;
		}

		// Show menu item selected
		if(choice != choice_ori)
		{
			QBoxShowChar(0,choice_ori,' ');
			QBoxShowString(6,choice_ori,answers[choice_ori-1]);
			QBoxShowChar(0,choice,'*');
			choice_ori=choice;
		}
	}

	// Pass on user choice
	return choice;
}


uint8_t my_mkdir(char * dir)
{
	uint8_t bret = false;
	bool opendir_err = 0;
	char SonPath[10][30];	// Up to 10 levels, max 30 characters each
	char RootPath[128];		// Lowest existing path
	memset(SonPath, '\0', sizeof(SonPath)); // Initialize
	strcpy(RootPath, dir);
	uint8_t num = 0;
	DIR W_Ddir;

	do						// Search for folder
	{
		char *dot = strrchr(RootPath, '\\');	// Search for last "\"
		if(dot == NULL)
			dot = strrchr(RootPath, '/');
		if(dot == NULL)
		{
			if(RootPath[0])
			{
				dot = RootPath;
			}
			else
				break;
		}

		strcpy(SonPath[num], dot);
		FRESULT W_Dresult = f_opendir(&W_Ddir, RootPath); // Try open directory
		if(W_Dresult == FR_OK)
		{
			printf("Exist[%s]\r\n",RootPath);
			f_closedir(&W_Ddir);
			break;
		}
		else
		{
			printf("Err - %d[%s]\r\n",W_Dresult, RootPath);
			if(W_Dresult == FR_NO_PATH)
			{
				opendir_err = 1;
				num ++;
				SonPath[num][0] = 0;
				dot[0] = 0x00;
			}
			else
				return bret;
		}
	}
	while(1);

	if(opendir_err == 1)
	{
		// Create folder
		opendir_err = 0;
		for(int i=0;i<num;i++)
		{
			strcat(RootPath, SonPath[num-i-1]);
			FRESULT W_Dresult = f_mkdir(RootPath);
			if(W_Dresult == FR_OK)
			{
				printf(">> Mk dir OK[%s]\r\n", RootPath);
				bret = true;
			}
			else
				printf(">> Err - %d [%s]\r\n",W_Dresult, RootPath);
		}
	}
	else
		bret = true;
		return bret;
}


/**********************
  File Browser Module
**********************/
char fileNames[128][100];
char answer1[100];
char answer2[100];
char answer3[100];
char answer4[100];
char answer5[100];
char answer6[100];
char answer7[100];
char* tanswers[7] = {answer1,answer2,answer3,answer4,answer5,answer6,answer7};

void fileBrowser(char * start_dir ,const char * browserTitle)
{
	int currFile = 0;
	int menucnt = 0;

	// Init Dir
	strncpy(filePath,start_dir,sizeof(filePath) - 1);
	filePath[sizeof(filePath) - 1] = '\0';
	DIR tdir;
	FRESULT fret;
	FILINFO finfo;
	bool bnomore;
	uint8_t mret;
	uint8_t default_select;
	bool dir_is_open = false;

browserstart:
	// Close the directory if it was left open
	if (dir_is_open)
	{
		f_closedir(&tdir);
		dir_is_open = false;
	}

	OledClear();
	//OledShowString(0,0,(char *)browserTitle,8);
	QBoxShowString(0,0,(char *)browserTitle);

	currFile = 0;
	currPage = 1;
	lastPage = 1;
	bnomore = false;

	if (f_opendir(&tdir,filePath) != FR_OK)
	{
		OledClear();
		print_Error("SD Error",true);
		return; // Return immediately to avoid an invalid pointer
	}
	dir_is_open = true;
	f_chdir(filePath);

next_page:
	menucnt = 0;
	while(1)
	{
		fret = f_readdir(&tdir,&finfo);
		if (fret == FR_OK)
		{
			if (finfo.fname[0] == 0x00)
			{
				bnomore = true;
				break;
			}

			// Protects against overflow in strings
			if (currFile < 128)
			{
				strncpy(fileNames[currFile],finfo.fname,100 - 1);
				fileNames[currFile][100 - 1] = '\0';

				if (menucnt < 7)
				{
					strncpy(tanswers[menucnt],fileNames[currFile],100 - 1);
					tanswers[menucnt][100 - 1] = '\0';
				}

				currFile++;
				menucnt++;
			}

			printf("\nfile:[%s]-[%s]",finfo.fname,finfo.altname);
			if (menucnt >= 7)
			{
				break;
			}
		}
		else
		{
			break;
		}
	}

	default_select = 1;

next_page1:
	mret = questionBox_OLED((char *)browserTitle,(const char **)tanswers,menucnt,default_select,0,1);

	switch(mret)
	{
		case MENU_CANCEL:
			{
				int len = strlen(filePath);

				// Check if we are already at the root directory
				if (len == 0 || (len == 1 && (filePath[0] == '/' || filePath[0] == '\\')))
				{
					if (dir_is_open)
					{
						f_closedir(&tdir); // Clean up FatFS directory handle before reboot
					}
					ResetSystem(); // Trigger the system reset
				}

				// Back-navigation logic if not at root
				bool chopped = false;
				for (int i = len - 1; i > 0; i--)
				{
					if (filePath[i] == '/' || filePath[i] == '\\')
					{
						filePath[i] = 0x00;
						chopped = true;
						break;
					}
				}
				if (!chopped && len > 0)
				{
					filePath[0] = 0x00;
				}
				goto browserstart;
			}
			break;
		case MENU_1:
		case MENU_2:
		case MENU_3:
		case MENU_4:
		case MENU_5:
		case MENU_6:
		case MENU_7:
			{
			int selected_idx = mret - 1;
			FIL tf;

			fret = f_open(&tf,tanswers[selected_idx],FA_OPEN_EXISTING);
			if (fret != FR_OK)
			{
				// It's a directory. Append to filePath and continue
				int current_len = strlen(filePath);
				if (current_len < (int)sizeof(filePath) - (int)strlen(tanswers[selected_idx]) - 2)
				{
					if (current_len > 0 && filePath[current_len - 1] != '/' && filePath[current_len - 1] != '\\')
					{
						strcat(filePath, "/");
					}
					strcat(filePath,tanswers[selected_idx]);
				}
				goto browserstart;
			}
			else
			{
				// It's a file. Append to path and return
				f_close(&tf);
				int current_len = strlen(filePath);
				if (current_len < (int)sizeof(filePath) - (int)strlen(tanswers[selected_idx]) - 2)
				{
					if (current_len > 0 && filePath[current_len - 1] != '/' && filePath[current_len - 1] != '\\')
					{
						strcat(filePath, "/");
					}
					strcat(filePath,tanswers[selected_idx]);
				}

				if (dir_is_open)
				{
					f_closedir(&tdir);
				}
				return;
			}
		}
		break;

		case MENU_PGUP:
		case MENU_UPUP:
			{
			if (currPage > 1)
			{
				currPage--;
				for (int i = 0; i < 7; i++)
				{
					int file_idx = (currPage - 1) * 7 + i;
					if (file_idx < currFile)
					{
						strncpy(tanswers[i],fileNames[file_idx],100 - 1);
						tanswers[i][100 - 1] = '\0';
					}
				}
				bnomore = false;
				menucnt = 7;
			}
			default_select = 1;
			goto next_page1;
		}
		break;

		case MENU_PGDN:
		case MENU_DOWNDOWN:
			{
			if (bnomore)
			{
				default_select = menucnt;
				goto next_page1;
			}
			currPage++;
			if (currPage > lastPage)
			{
				lastPage++;
				goto next_page;
			}
			else
			{
				menucnt = 0;
				bnomore = false;
				for (int i = 0; i < 7; i++)
				{
					int file_idx = (currPage - 1) * 7 + i;
					if (file_idx < currFile)
					{
						strncpy(tanswers[i],fileNames[file_idx],100 - 1);
						tanswers[i][100 - 1] = '\0';
						menucnt++;
					}
					else
					{
						bnomore = true;
						break;
					}
				}
				default_select = 1;
				goto next_page1;
			}
		}
		break;

		default:
			{
			if (dir_is_open)
			{
				f_closedir(&tdir);
			}
			print_Error("File Err...", 1);
		}
		break;
	}
}