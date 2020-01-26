/*
 * The ncurses chat window object
 * Reads its data from the irclsd output log file and displays it
 * on the screen in 'tail -f' fashion.
 * There is no buffer history, the 'part', 'join', 'quit' messages are not
 * displayed. Only privmsg are displayed.
 * Feature: when 'To:' is sent by user, all other messages which do not
 * corespond to 'To:' are filtered
 */
#include <win/win.h>
#include <global/global.h>                       //NICLEN, etc
#include <index/index.h>                         //log file indexing
#include <ircproto/ircproto.h>                   //irc protocol parsing
#include <stylerot/stylerot.h>                   //rotational styles

#include <ncursesw/curses.h>
#include <stdio.h>                               //fopen()
#include <wchar.h>                               //wchar_t
#include <string.h>                              //strcspn()
#include <stdlib.h>                              //srand()
#include <time.h>                                //time()
#include <sys/select.h>                          //select()


/*win data        fd    win   pan   index read_bytes display_bytes ircproto*/
static Win win = {NULL, NULL, NULL, NULL, 0        , 0           , NULL};

/*
 * These are coming from global.{h, c}
 */
extern Style style;
extern int count_tome;
extern wchar_t mynick[NICKLEN + 1];
extern wchar_t tosend[CHANLEN + 1];

/*
 * Initializes the chat window object
 * filename: the filename of the irclsd output log file
 * Returns: 0 OK, negative ERR
 */
int
win_init(const char filename[PATHLEN + 1])
{
	int h;                                   //height
	int w;                                   //width

	getmaxyx(stdscr, h, w);

	win.win = newwin(h - 1, w, 0, 0);        //create, position window
	idlok(win.win, TRUE);                    //needed with scrollok (man)
	scrollok(win.win, TRUE);                 //auto scrolling
	win.pan = new_panel(win.win);            //own panel for chat window

	win.ircproto = ircproto_new();           //irc protocol object
	if(win.ircproto == NULL)
	{
		win_del();
		return -3;
	}

	win.fd = fopen(filename, "r");           //open irclsd output log file
	if(win.fd == NULL)
	{
		win_del();
		return -1;
	}

	win.index = index_new(HISTORYBUF);       //file indexing object
	if(win.index == NULL)
	{
		win_del();
		return -2;
	}
	win_index_file();                        //index file for first time

	/*
	 * this emulates the last log file lines as already displayed
	 * e.g in the future we will display only the newly logged lines
	 * (behaviour similar to tail -f)
	 */
	index_get(win.index, &(win.display_bytes), -1);

	win_draw_logo();                         //draw the logo

	return 0;                                //OK
} /*win_init()*/


/*
 * Frees memory and pointers
 */
void
win_del(void)
{
	/*these are protected - if not initialized, do not delete*/
	if(win.win != NULL) delwin(win.win);
	if(win.pan != NULL) del_panel(win.pan);
	if(win.fd != NULL) fclose(win.fd);

	/*these are unprotected - they have their own protection inside*/
	index_del(win.index);
	ircproto_del(win.ircproto);
} /*win_del()*/


/*
 * Draws the window on the screen
 */
void
win_draw(void)
{
	int i;                                   //index
	char buf[CBUFLEN + 1];                   //character buffer for one ln
	wchar_t wbuf[WBUFLEN + 1];               //wide char buf 1 ln
	size_t start;                            //the current ln start
	size_t end;                              //the current ln end
	int styl;                                //ncurses style

	win_index_file();                        //index the newly received
	                                         //lines from irclsd

	/* get the previous newline bytes position in the file*/
	i = -1;
	index_get(win.index, &start, i);
	/* if already displayed - return and do not display anything*/
	if(start <= win.display_bytes) return;

	/* rewind the file backwards until the last line read*/
	i = 0;
	do
	{
		if(!index_get(win.index, &start, --i)) break;
	}
	while(start > win.display_bytes);
	/*now we have the first index to read in variable i*/
	

	/*display the newly received lines from irclsd*/
	while(i < -1)
	{
		/*get the start byte*/
		if(!index_get(win.index, &start, i++)) break;
		/*get the next byte as end byte*/
		if(!index_get(win.index, &end, i)) break;

		/*read the file between start, end*/
		win_read_file(buf, start, end);
		/*discard \r if exists*/
		buf[strcspn(buf, "\r")] = '\0';
		/*draw the newly received line buffer on screen*/
		win_draw_line(buf);
	} /*while(i < -1)*/

	/*save the last display bytes*/
	index_get(win.index, &(win.display_bytes), -1);
	/*position the file pointer at the last read byte*/
	fseek(win.fd, win.read_bytes, SEEK_SET);
	/*draw from virtual screen to user screen*/
	wrefresh(win.win);
} /*win_draw()*/


/*
 * Draws textual logo on screen
 */
static void
win_draw_logo(void)
{
	char buf[CBUFLEN + 1];                   //a line buffer
	int h;                                   //height
	int w;                                   //width
	FILE *fp = fopen(LOGO, "r");             //open logo.txt file
	if(fp == NULL) return;

	getmaxyx(win.win, h, w);

	/*draw logo on screen */
	wmove(win.win, 0, 0);
	waddch(win.win, '\n');
	waddch(win.win, '\n');
	wattron(win.win, style.error);
	while(fgets(buf, CBUFLEN, fp) != NULL)
	{
		buf[strcspn(buf, "\n")] = 0;
		if(strlen(buf) > w - 1)          //logo line needs to be less
		{                                //than screen width
			wattroff(win.win, style.error);
			fclose(fp);
			wrefresh(win.win);
			return;
		}
		wprintw(win.win, "%s\n", buf);
	}
	wattroff(win.win, style.error);

	fclose(fp);
	wrefresh(win.win);                       //draw on user screen
} /*win_draw_logo()*/


/*
 * Indexes the irclsd output log file into the win.index object
 */
static void
win_index_file(void)
{
	int ch;                                  //one character

	while((ch = fgetc(win.fd)) != EOF)       //get ch until EOF
	{
		win.read_bytes++;                //increment the already read

		/*we are indexing the newlines in the file by adding them to
		 * the win.index object*/
		if(ch == '\n')	index_append(win.index, win.read_bytes);
	}
} /*win_index_file()*/


/*
 * Reads irclsd output log file between start, end bytes
 * buf: buffer to read into
 */
static void
win_read_file(char buf[CBUFLEN + 1], const size_t start, const size_t end)
{
	size_t read_size = end - start;          //how many bytes to read
	read_size = MIN(read_size, CBUFLEN + 1); //buffer overflow protection

	/*go to start*/
	if(fseek(win.fd, start, SEEK_SET) != 0) return;

	fread(buf, sizeof(char), read_size, win.fd);
	/*FIXME: here may have fread success check protection*/
	buf[read_size - 1] = '\0';
} /*win_read_file()*/


/*
 * Draws one line from the given char buffer
 * buf: character buffer to draw on screen
 */
static void
win_draw_line(const char buf[CBUFLEN + 1])
{
	int res = ircproto_parse(win.ircproto, buf); //parse irc protocol

	/* act according to the result from ircproto_parse()*/
	if(res == IRC_RES_PRIVMSG) win_draw_line_privmsg();
	else if((res == IRC_RES_UNKNOWN) &&                      //unknown line
		(win.ircproto->mark == L'>') &&                  //from me
		(wcsncmp(win.ircproto->params, L"PONG", 4) != 0))//not PONG
		win_draw_line_myunspecified();                   //is drawn
	else if(res == IRC_RES_MYPRIVMSG) win_draw_line_myprivmsg();
	else if(res == IRC_RES_ERROR) win_draw_line_error();
} /*win_draw_line()*/


/*
 * Draws a privmsg line on screen
 */
static void
win_draw_line_privmsg(void)
{
	int h;                                   //height
	int w;                                   //width
	int flag_tome = 0;                       //is privmsg send to me?

	/*if 'tosend' is not empty, filter all other messages except messages
	 * sent directly to me*/
	if((tosend[0] != '\0') &&                                   //nonempty
	   (wcsncmp(win.ircproto->tonick, tosend, CHANLEN) != 0) && //no tosend
	   (wcsncmp(win.ircproto->tonick, mynick, NICKLEN != 0)))   //not to me
		return;                                             //bye

	getmaxyx(win.win, h, w);
	wmove(win.win, h - 1, 0);                //move to the begining
	                                         //of the last row

	/*draw short version of time*/
	wattron(win.win, style.time);
	wprintw(win.win, "%ls ", win.ircproto->tshort);
	wattroff(win.win, style.time);

	/*bring up flag_tome if the message is sent to me*/
	if(wcsncmp(win.ircproto->tonick, mynick, NICKLEN) == 0)
	{
		flag_tome = 1;
		count_tome++;
	}

	/*arrange style*/
	if(flag_tome) wattron(win.win, style.priv);
	/*this below arranges rotational style for the given tonick*/
	else wattron(win.win, stylerot_get(win.ircproto->tonick));

	/*draw tonick and nick*/
	wprintw(win.win, "[%ls] ", win.ircproto->tonick);
	wattron(win.win, style.mod_nick);
	wprintw(win.win, "<%ls>", win.ircproto->nick);
	wattroff(win.win, style.mod_nick);

	/*off style*/
	if(!flag_tome) wattroff(win.win, stylerot_get(win.ircproto->tonick));

	/*draw the message*/
	wprintw(win.win, " %ls\n", win.ircproto->params);

	/*off style*/
	if(flag_tome) wattroff(win.win, style.priv);
} /*win_draw_line_privmsg()*/


/* 
 * Draws my privmsg line (privmsg sent from me) on screen
 */
static void
win_draw_line_myprivmsg(void)
{
	int h;                                   //height
	int w;                                   //width

	getmaxyx(win.win, h, w);
	wmove(win.win, h - 1, 0);                //beginning of last line

	/*draw time*/
	wattron(win.win, style.time);
	wprintw(win.win, "%ls", win.ircproto->tshort);
	wattroff(win.win, style.time);

	wattron(win.win, style.priv);            //style is private style

	/*draw tonick, usually this is a channel*/
	wprintw(win.win, " [%ls] ", win.ircproto->tonick);

	/*draw sending nick (this is me)*/
	wattron(win.win, style.mod_nick);
	wprintw(win.win, "<%ls> ", win.ircproto->nick);
	wattroff(win.win, style.mod_nick);

	/*draw the message*/
	wprintw(win.win, "%ls\n", win.ircproto->params);

	wattron(win.win, style.priv);            //end style
} /*win_draw_line_myprivmsg()*/


/*
 * Draws an error line on screen
 */
static void
win_draw_line_error(void)
{
	int h;                                   //height
	int w;                                   //width

	getmaxyx(win.win, h, w);
	wmove(win.win, h - 1, 0);                //beginning of last line

	/*draw*/
	wattron(win.win, style.error);
	wprintw(win.win, "%ls %lc %ls\n",
		win.ircproto->t,
		win.ircproto->mark,
		win.ircproto->params);
	wattroff(win.win, style.error);
} /*win_draw_line_error()*/


/*
 * Draws one line on screen of unspecified type sent from me
 */
static void
win_draw_line_myunspecified(void)
{
	int h;                                   //height
	int w;                                   //width

	getmaxyx(win.win, h, w);
	wmove(win.win, h - 1, 0);                //to beginning of last line

	wattron(win.win, style.time);            //draw time
	wprintw(win.win, "%ls", win.ircproto->tshort);
	wattroff(win.win, style.time);

	wattron(win.win, style.priv);            //draw nick (this is me)
	wattron(win.win, style.mod_nick);
	wprintw(win.win, " <%ls> ", mynick);
	wattroff(win.win, style.mod_nick);

	wprintw(win.win, "%ls\n", win.ircproto->params); //the raw IRC command
	wattron(win.win, style.priv);
} /*win_draw_line_myunspecified()*/


/*
 * Clears screen in funny way
 */
void 
win_matrix_reloaded(void)
{
	int h;                                   //height
	int w;                                   //width
	int x;                                   //screen position x
	int y;                                   //screen position y
	int chcount;                             //character count
	int styl;                                //ncurses style
	struct timeval tv;                       //needed for microsleep

	getmaxyx(win.win, h, w);

	chcount = h * w;                         //all characters on screen

	curs_set(0);                             //cursor off (more beautyful)

	wmove(win.win, 0, 0);

	srand(time(0));                          //needed for stylerot_random()
	                                         //and range_random()
	for(int i = 0; i < chcount; i++)
	{

		getyx(win.win, y, x);
		if(x % 2 == 0)                   //even column
		{
		        styl = stylerot_random();//random style
		        wattron(win.win, styl);
			waddch(win.win, range_random(32, 126)); //random char
		        wattroff(win.win, styl);
		}
		else                             //odd column
			waddch(win.win, ' ');    //space


	}
	wrefresh(win.win);                       //display all on screen now

	for(int i = 0; i < chcount; i++)
	{
		waddch(win.win, ' ');            //space and immediately
		wrefresh(win.win);               //display on scr

		/*sleep implementation*/
		tv.tv_sec = 0;
		tv.tv_usec = 150; //todo 150
		select(0, NULL, NULL, NULL, &tv);
	}

	curs_set(1);                             //cursor on (needed in input)
} /*win_matrix_reloaded()*/
