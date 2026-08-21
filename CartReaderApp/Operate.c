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

	if(gpio_input_bit_get(GPIOB, GPIO_PIN_1) == RESET)
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
unsigned char questionBox_OLED(char * question, const char* const answers[7], int num_answers, int default_choice, uint8_t rollselect, uint8_t clrScr, uint8_t hasPicData)
{
	if(clrScr > 0)
	{
		OledClear();
	}
	OledShowString(0, 0, question, 8);

	// Prevent empty or out-of-bounds default_choice
	if (default_choice < 1)
	{
		default_choice = 1;
	}
	if (default_choice > num_answers)
	{
		default_choice = num_answers;
	}

	// Draw valid answers for this page
	char tanswer[21] = {0};
	for (unsigned char i = 0; i < num_answers; i++)
	{
		strncpy(tanswer, answers[i], 20);
		tanswer[20] = '\0'; // Forced null terminator
		QBoxShowString(6, i + 1, tanswer, 0);
	}

	if (!hasPicData)
	{
		// Blank out any remaining menu list lines
		// This prevents text artifacts from previous pages if this page has fewer items.
		for (unsigned char i = num_answers; i < 7; i++)
		{
			OledClearLine(i + 1);
		}
	}

	// Explicitly clear cursor column (x=0) for all rows on entry
	for (uint8_t i = 1; i <= 7; i++)
	{
		OledShowChar(0, i, ' ', 8);
	}

	unsigned char choice = default_choice;
	unsigned char choice_ori = default_choice;

	// Draw initial selection bullet
	OledShowChar(0, choice, '*', 8);

	uint32_t scroll_tick = 0;
	uint8_t scroll_start = 0;

	// Wait until user makes their choice
	while (1)
	{
		int b = checkButton();
		if(b == BTNNONE)
		{
			scroll_tick++;
			if((scroll_tick > 14) && (scroll_tick % 3 == 1))
			{
				if(QBoxShowString(6, choice, answers[choice - 1], scroll_start) > 0)
				{
					scroll_start++;
				}
			}
		}
		else
		{
			printf("getKey-%d\n", b);
			scroll_tick = 0;
			scroll_start = 0;
		}

		// Direct return logic keeps bad data away from choice loop
		if(b == BTNLEFT)
		{
			if(!rollselect)
			{
				return MENU_PGUP;
			}
		}
		else if (b == BTNRIGHT)
		{
			if(!rollselect)
			{
				return MENU_PGDN;
			}
		}
		else if (b == BTNUP)
		{
			if (choice <= 1)
			{
				if (rollselect)
				{
					choice = num_answers;
				}
				else
				{
					return MENU_UPUP;
				}
			}
			else
			{
				choice--;
			}
		}
		else if (b == BTNDOWN)
		{
			if (choice >= num_answers)
			{
				if (rollselect)
				{
					choice = 1;
				}
				else
				{
					return MENU_DOWNDOWN;
				}
			}
			else
			{
				choice++;
			}
		}
		else if (b == BTNCANCEL)
		{
			return MENU_CANCEL;
		}
		else if (b == BTNOK)
		{
			return choice;
		}

		// Move '*' to new selection
		if(choice != choice_ori)
		{
			// Erase old bullet
			OledShowChar(0, choice_ori, ' ', 8);

			// Reset ticker layout immediately for the item we just left
			QBoxShowString(6, choice_ori, answers[choice_ori - 1], 0);

			// Draw new bullet
			OledShowChar(0, choice, '*', 8);

			// Draw new line at un-scrolled starting position
			QBoxShowString(6, choice, answers[choice - 1], 0);

			choice_ori = choice;
		}
	}
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
			printf("Exist[%s]\r\n", RootPath);
			f_closedir(&W_Ddir);
			break;
		}
		else
		{
			printf("Err - %d[%s]\r\n", W_Dresult, RootPath);
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
				printf(">> Err - %d [%s]\r\n", W_Dresult, RootPath);
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
char* tanswers[7] = {answer1, answer2, answer3, answer4, answer5, answer6, answer7};


uint8_t fileBrowser(char * start_dir, const char * browserTitle)
{
	int currFile = 0;
	int menucnt = 0;

	if (start_dir[0] == '/' && start_dir[1] == '\0')
	{
		filePath[0] = '\0';
	}
	else
	{
		strncpy(filePath, start_dir, sizeof(filePath) - 1);
		filePath[sizeof(filePath) - 1] = '\0';
	}

	DIR tdir;
	FRESULT fret;
	FILINFO finfo;
	bool bnomore;
	uint8_t mret;
	uint8_t default_select = 1; // Set selection to item 1 on init
	bool dir_is_open = false;
	bool prnt_title  = false;

browserstart:
	// Close the directory if it was left open
	if (dir_is_open)
	{
		f_closedir(&tdir);
		dir_is_open = false;
	}
	for (uint8_t y = 1; y < 8; y++)
	{
		OledClearLine(y);
	}
	bool cleared = true;

	if (!prnt_title)
	{
		OledClearLine(0);
		OledShowString(0, 0, (char *)browserTitle, 8);
		prnt_title = true;
	}

	// Safely clear out our string cache to prevent cross-directory text pollution
	memset(fileNames, 0, sizeof(fileNames));

	currFile = 0;
	currPage = 1;
	lastPage = 1;
	bnomore = false;

	const char* dir_to_open = (strlen(filePath) == 0) ? "/" : filePath;

	if (f_opendir(&tdir, dir_to_open) != FR_OK)
	{
		OledClear();
		print_Error("SD Error", true);
		return 0;
	}

	dir_is_open = true;
	f_chdir(dir_to_open);

next_page:

	menucnt = 0;
	if (!cleared)
	{
		for (uint8_t y = 1; y < 8; y++)
		{
			OledClearLine(y);
		}
	}
	cleared = false;

	// Reset display answers buffers completely before loading new strings
	for (int i = 0; i < 7; i++)
	{
		tanswers[i][0] = '\0';
	}

	while(1)
	{
		fret = f_readdir(&tdir, &finfo);
		if (fret == FR_OK)
		{
			// Check ONLY for end of directory
			if (finfo.fname[0] == 0x00)
			{
				bnomore = true;
				break;
			}

			strncpy(fileNames[currFile], finfo.fname, 100 - 1);
			fileNames[currFile][100 - 1] = '\0';

			if (menucnt < 7)
			{
				strncpy(tanswers[menucnt], fileNames[currFile], 100 - 1);
				tanswers[menucnt][100 - 1] = '\0';
			}
			printf("\nfile:[%s]-[%s]", finfo.fname, finfo.altname);
			currFile++;
			menucnt++;

			// Check for file limit reached
			if (currFile >= 128)
			{
				bnomore = true;
				break;
			}

			// Peek ahead to see if end of dir reached, or goto next page
			if (menucnt >= 7)
			{
				// Save entire state of dir structure preserving LFNs
				DIR backup_tdir = tdir;
				FILINFO peek_finfo;

				if (f_readdir(&tdir, &peek_finfo) == FR_OK)
				{
					if (peek_finfo.fname[0] == 0x00)
					{
						bnomore = true; // No more files!
					}
				}

				// Restore dir structure
				tdir = backup_tdir;
				break;
			}
		}
		else
		{
			bnomore = true;
			break;
		}
	}

next_page1:
	mret = questionBox_OLED((char *)browserTitle, (const char **)tanswers, menucnt, default_select, 0, 0, 0);
	switch(mret)
	{
		case MENU_CANCEL:
		{
			int len = strlen(filePath);

			if (dir_is_open)
			{
				f_closedir(&tdir);
				dir_is_open = false;
			}
			// Check if we are at root dir
			if (len == 0 || (len == 1 && (filePath[0] == '/' || filePath[0] == '\\')))
			{
				prnt_title = false;
				return 0;
			}

			// Back-nav logic, strip last directory layer
			bool chopped = false;
			for (int i = len - 1; i >= 0; i--)
			{
				if (filePath[i] == '/' || filePath[i] == '\\')
				{
					filePath[i] = 0x00;
					chopped = true;
					break;
				}
			}
			if (!chopped || strlen(filePath) == 0)
			{
				filePath[0] = 0x00;
			}
			default_select = 1; // Move selector to top when navigating backwards
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
			int selected_idx = mret - 1; // 0-based index conversion
			FIL tf;
			fret = f_open(&tf, tanswers[selected_idx], FA_OPEN_EXISTING);
			if (fret != FR_OK)
			{
				// It's a directory. Append to filePath and continue
				int current_len = strlen(filePath);
				if (current_len < (int)sizeof(filePath) - (int)strlen(tanswers[selected_idx]) - 2)
				{
					if (current_len == 0)
					{
						int remaining = sizeof(filePath) - strlen(filePath) - 1;
						strncat(filePath, "/", remaining);
					}
					else if (filePath[current_len - 1] != '/' && filePath[current_len - 1] != '\\')
					{
						int remaining = sizeof(filePath) - strlen(filePath) - 1;
						strncat(filePath, "/", remaining);
					}
					int remaining = sizeof(filePath) - strlen(filePath) - 1;
					strncat(filePath, tanswers[selected_idx], remaining);
				}
				default_select = 1;
				goto browserstart;
			}
			else
			{
				// It's a file. Append to filePath and return
				f_close(&tf);
				int current_len = strlen(filePath);
				if (current_len < (int)sizeof(filePath) - (int)strlen(tanswers[selected_idx]) - 2)
				{
					if (current_len == 0)
					{
						int remaining = sizeof(filePath) - strlen(filePath) - 1;
						strncat(filePath, "/", remaining);
					}
					else if (filePath[current_len - 1] != '/' && filePath[current_len - 1] != '\\')
					{
						int remaining = sizeof(filePath) - strlen(filePath) - 1;
						strncat(filePath, "/", remaining);
					}
					int remaining = sizeof(filePath) - strlen(filePath) - 1;
					strncat(filePath, tanswers[selected_idx], remaining);
				}
				if (dir_is_open)
				{
					f_closedir(&tdir);
				}
				prnt_title = false;
				return 1;
			}
		}
		break;

		case MENU_PGUP:
		case MENU_UPUP:
		{
			if (currPage > 1)
			{
				currPage--;
				menucnt = 0;

				for (uint8_t y = 1; y < 8; y++)
				{
					OledClearLine(y);
				}

				// Reset layout arrays before back-copying
				for (int i = 0; i < 7; i++)
				{
					tanswers[i][0] = '\0';
				}

				for (int i = 0; i < 7; i++)
				{
					int file_idx = (currPage - 1) * 7 + i;
					if (file_idx < currFile)
					{
						strncpy(tanswers[i], fileNames[file_idx], 100 - 1);
						tanswers[i][100 - 1] = '\0';
						menucnt++;
					}
				}
				bnomore = false;
				default_select = menucnt; // Highlight last actual item of preceding page
			}
			else
			{
				default_select = 1; // Select item 1 if already on page 1
			}
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
				default_select = 1;
				goto next_page;
			}
			else
			{
				menucnt = 0;
				bnomore = false;

				for (uint8_t y = 1; y < 8; y++)
				{
					OledClearLine(y);
				}

				// Erase past text tracks from buffer before filling strings
				for (int i = 0; i < 7; i++)
				{
					tanswers[i][0] = '\0';
				}

				for (int i = 0; i < 7; i++)
				{
					int file_idx = (currPage - 1) * 7 + i;
					if (file_idx < currFile)
					{
						strncpy(tanswers[i], fileNames[file_idx], 100 - 1);
						tanswers[i][100 - 1] = '\0';
						menucnt++;
					}
					else
					{
						bnomore = true;
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
	prnt_title = false;
}
