#include <stylerot/stylerot.h>                   //prototypes
#include <global/global.h>                       //CHANLEN

#include <ncursesw/curses.h>
#include <wchar.h>                               //wchar_t


/*
 * The array with rotational style entries
 */
static StyleRotEntry stylerot[STYLEROT_ENTRIES];


/*
 * Initialize ncurses styles
 */
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


/*
 * Returns the style of the given "to nick"
 * tonick: the given "to nick" we need to get the style for
 * Returns: int - the ncurses color style integer
 */
int
stylerot_get(const wchar_t tonick[CHANLEN + 1])
{
	/*
	 * NOTE: these i, j are static. This means their value does not change
	 * between invocations of the function
	 */
	static int i = 0;                        //index of tonick in array
	static int j = STYLEROT_MAGENTA;         //number of color pair

	/* try to find the saved style for tonick and return it*/
	for(int k = 0; k < i; k++)
	{
		if(wcsncmp(tonick, stylerot[k].tonick, CHANLEN) == 0)
			return COLOR_PAIR(stylerot[k].style);
	}

	/*if reached max rotational style entries - always return white*/
	if(i >= STYLEROT_ENTRIES) return COLOR_PAIR(STYLEROT_WHITE);

	/* pick the next style and assign to the current tonick*/
	wcsncpy(stylerot[i].tonick, tonick, CHANLEN);
	stylerot[i].tonick[CHANLEN] = L'\0';
	stylerot[i].style = j;

	/*increment j and wrap it over if needed*/
	j++;
	if(j > STYLEROT_WHITE) j = STYLEROT_MAGENTA;

	/* return the currently assigned style and increment i
	 * for the next run*/
	return COLOR_PAIR(stylerot[i++].style);
} /*stylerot_get()*/


//NOTE srand(time(0)) should be invoked before this function
int
stylerot_random(void)
{
	int styl = range_random(STYLEROT_MAGENTA, STYLEROT_WHITE);
	int modifier = range_random(0, 1);

	if(modifier)
		return COLOR_PAIR(styl) | A_BOLD;

	return COLOR_PAIR(styl);
} /*stylerot_random()*/
