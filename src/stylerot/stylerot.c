#include <stylerot/stylerot.h>
#include <global/global.h>

#include <ncursesw/curses.h>
#include <wchar.h>


static StyleRotEntry stylerot[STYLEROT_ENTRIES];


void
stylerot_init(void)
{
	init_pair(STYLEROT_MAGENTA, COLOR_MAGENTA, -1);
	init_pair(STYLEROT_CYAN, COLOR_CYAN, -1);
	init_pair(STYLEROT_BLUE, COLOR_BLUE, -1);
	init_pair(STYLEROT_RED, COLOR_RED, -1);
	init_pair(STYLEROT_GREEN, COLOR_GREEN, -1);
	init_pair(STYLEROT_WHITE, COLOR_WHITE, -1);
} /*stylerot_init()*/


void
stylerot_del(void)
{
} /*stylerot_del()*/


int
stylerot_get(const wchar_t tonick[CHANLEN + 1])
{
	static int i = 0;
	static int j = STYLEROT_MAGENTA;

	for(int k = 0; k < i; k++)
	{
		if(wcsncmp(tonick, stylerot[k].tonick, CHANLEN) == 0)
			return COLOR_PAIR(stylerot[k].style);
	}

	if(i >= STYLEROT_ENTRIES) return COLOR_PAIR(STYLEROT_WHITE);

	wcsncpy(stylerot[i].tonick, tonick, CHANLEN);
	stylerot[i].tonick[CHANLEN] = L'\0';
	stylerot[i].style = j;

	j++;
	if(j > STYLEROT_WHITE) j = STYLEROT_MAGENTA;

	return COLOR_PAIR(stylerot[i++].style);
} /*stylerot_get()*/
