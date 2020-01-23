/* 
 * This is the status bar object for the ncurses interface
 */
#include <bar/bar.h>                             //prototypes
#include <global/global.h>                       //NICKLEN, CHANLEN, etc.

#include <ncursesw/curses.h>
#include <wchar.h>                               //wchar_t
#include <string.h>                              //strlen()


//bar data        win   pan
static Bar bar = {NULL, NULL};

/*these are coming from globals.{h, c}*/
extern wchar_t mynick[NICKLEN + 1];
extern wchar_t server[SERVERLEN + 1];
extern wchar_t tosend[CHANLEN + 1];
extern int count_tome;
extern Style style;


/*
 * Initializes status bar object
 */
void
bar_init(void)
{
	int h;                                   //height
	int w;                                   //width

	/*create window at desired place, create panel for z overlapping*/
	getmaxyx(stdscr, h, w);
	bar.win = newwin(1 , w, h - 2, 0);
	bar.pan = new_panel(bar.win);
} /*bar_init()*/


/*
 * Frees the memory
 */
void
bar_del(void)
{
	/*these are protected - if uninitialized, do nothing*/
	if(bar.win != NULL) delwin(bar.win);
	if(bar.pan != NULL) del_panel(bar.pan);
} /*bar_del()*/


/*
 * Draws the status bar
 */
void
bar_draw(void)
{
	size_t len = 0;                          //cumulative line length
	int h;                                   //height
	int w;                                   //width
	int y;                                   //y position
	int x;                                   //x position
	char count[COUNTLEN + 1];                //private messages count str
	wchar_t to[CHANLEN + 1];                 //send to wchar str

	getmaxyx(bar.win, h, w);

	wmove(bar.win, 0, 0);

	/*draw To label*/
	len = wcslen(L"  To: ");
	if(len >= w) goto PADDING;
	wattron(bar.win, style.bar);
	wprintw(bar.win, "  To: ");
	wattroff(bar.win, style.bar);

	/*draw configured To receiver*/
	if(wcsncmp(tosend, L"", 1) == 0) 
		wcsncpy(to, L"..............", CHANLEN); //default for empty
	else
		wcsncpy(to, tosend, CHANLEN);    //get from global tosend var
	to[CHANLEN] = '\0';
	len += wcslen(L"[") + wcslen(to) + wcslen(L"]");
	if(len >= w) goto PADDING;
	wattron(bar.win, style.bar_txtbox);
	wprintw(bar.win, "[%ls]", to);
	wattroff(bar.win, style.bar_txtbox);

	/*draw private messages label*/
	len += wcslen(L" |Priv:");
	if(len >= w) goto PADDING;
	wattron(bar.win, style.bar);
	wprintw(bar.win, " |Priv:");
	wattroff(bar.win, style.bar);

	/*draw private messages count*/
	snprintf(count, COUNTLEN, "%d", count_tome); //get it from global var
	count[COUNTLEN] = '\0';
	len += strlen(" ") + strlen(count) + strlen(" ");
	if(len >= w) goto PADDING;
	if(count_tome == 0)                      //arrange ncurses style
		wattron(bar.win, style.count);
	else
		wattron(bar.win, style.count_highlight);
	wprintw(bar.win, " %s ", count);
	if(count_tome == 0)
		wattroff(bar.win, style.count);
	else
		wattroff(bar.win, style.count_highlight);

	/*draw server label and value*/
	len += wcslen(L"|Srv:") + wcslen(server);
	if(len >= w) goto PADDING;
	wattron(bar.win, style.bar);
	wprintw(bar.win, "|Srv:%ls", server);
	wattroff(bar.win, style.bar);

	/*draw nick label and value*/
	len += wcslen(L" |Nick:") + wcslen(mynick);
	if(len >= w) goto PADDING;
	wattron(bar.win, style.bar);
	wprintw(bar.win, " |Nick:%ls", mynick);
	wattroff(bar.win, style.bar);

	/*when everything is drawn, pad to screen width with spaces*/
        PADDING:wattron(bar.win, style.bar);
	getyx(bar.win, y, x);
	while(x <= w - 1)
	{
		waddch(bar.win, ' ');
		if(x == w - 1) break;
		getyx(bar.win, y, x);
	}
	wattroff(bar.win, style.bar);

	wrefresh(bar.win);                       //draw on virtual screen
} /*bar_draw()*/
