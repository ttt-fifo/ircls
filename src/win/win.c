#include <win/win.h>
#include <global/global.h>
#include <index/index.h>
#include <ircproto/ircproto.h>
#include <stylerot/stylerot.h>

#include <ncursesw/curses.h>
#include <stdio.h> //fopen()
#include <wchar.h> //wchar_t


/*win data        fd    win   pan   read_bytes display_bytes current_row_style*/
static Win win; //TODO: initialization here

extern Style style;
extern int count_tome;
extern wchar_t mynick[NICKLEN + 1];
extern wchar_t tosend[CHANLEN + 1];


int
win_init(const char filename[PATHLEN + 1])
{
	int h;
	int w;

	getmaxyx(stdscr, h, w);

	win.win = newwin(h - 1, w, 0, 0);
	idlok(win.win, TRUE);                    //needed with scrollok (man)
	scrollok(win.win, TRUE);
	win.pan = new_panel(win.win);

	win.ircproto = ircproto_new();
	if(win.ircproto == NULL)
	{
		win_del();
		return -3;
	}

	win.fd = fopen(filename, "r");
	if(win.fd == NULL)
	{
		win_del();
		return -1;
	}

	win.index = index_new(HISTORYBUF);
	if(win.index == NULL)
	{
		win_del();
		return -2;
	}
	win_index_file();

	index_get(win.index, &(win.display_bytes), -1);

	return 0;
} /*win_init()*/


void
win_del(void)
{
	if(win.win != NULL) delwin(win.win);
	if(win.pan != NULL) del_panel(win.pan);
	if(win.fd != NULL) fclose(win.fd);
	index_del(win.index);
	ircproto_del(win.ircproto);
} /*win_del()*/


void
win_draw(void)
{
	int i;
	char buf[CBUFLEN + 1];
	wchar_t wbuf[WBUFLEN + 1];
	size_t start;
	size_t end;
	int styl;

	win_index_file();

	i = -1;
	index_get(win.index, &start, i);
	if(start <= win.display_bytes) return;

	i = 0;
	do
	{
		if(!index_get(win.index, &start, --i)) break;
	}
	while(start > win.display_bytes);
	

	while(i < -1)
	{
		if(!index_get(win.index, &start, i++)) break;
		if(!index_get(win.index, &end, i)) break;

		win_read_file(buf, start, end);
		buf[strcspn(buf, "\r")] = '\0';
		win_draw_line(buf);
	}

	index_get(win.index, &(win.display_bytes), -1);
	fseek(win.fd, win.read_bytes, SEEK_SET);
	wrefresh(win.win); //???
} /*win_draw()*/


static void
win_index_file(void)
{
	int ch;

	while((ch = fgetc(win.fd)) != EOF)
	{
		win.read_bytes++;
		if(ch == '\n')	index_append(win.index, win.read_bytes);
	}
} /*win_index_file()*/


static void
win_read_file(char buf[CBUFLEN + 1], const size_t start, const size_t end)
{
	size_t read_size = end - start;
	read_size = MIN(read_size, CBUFLEN + 1);

	if(fseek(win.fd, start, SEEK_SET) != 0) return;

	fread(buf, sizeof(char), read_size, win.fd); /*how to check success?*/
	buf[read_size - 1] = '\0';
} /*win_read_file()*/


static void
win_draw_line(const char buf[CBUFLEN + 1])
{
	int res = ircproto_parse(win.ircproto, buf);

	if(res == IRC_RES_PRIVMSG) win_draw_line_privmsg();
	else if((res == IRC_RES_UNKNOWN) &&
		(win.ircproto->mark == L'>') &&
		(wcsncmp(win.ircproto->params, L"PONG", 4) != 0))
		win_draw_line_myunspecified();
	else if(res == IRC_RES_MYPRIVMSG) win_draw_line_myprivmsg();
	else if(res == IRC_RES_ERROR) win_draw_line_error();
} /*win_draw_line()*/


static void
win_draw_line_privmsg(void)
{
	int h;
	int w;
	int flag_tome = 0;

	if((tosend[0] != '\0') &&
	   (wcsncmp(win.ircproto->tonick, tosend, CHANLEN) != 0) &&
	   (wcsncmp(win.ircproto->tonick, mynick, NICKLEN != 0)))
		return;

	getmaxyx(win.win, h, w);
	wmove(win.win, h - 1, 0);

	wattron(win.win, style.time);
	wprintw(win.win, "%ls", win.ircproto->tshort);
	wattroff(win.win, style.time);

	if(wcsncmp(win.ircproto->tonick, mynick, NICKLEN) == 0)
	{
		flag_tome = 1;
		count_tome++;
	}

	if(flag_tome) wattron(win.win, style.priv);
	else wattron(win.win, stylerot_get(win.ircproto->tonick));

	wprintw(win.win, " [%ls] ", win.ircproto->tonick);
	wattron(win.win, style.mod_nick);
	wprintw(win.win, "<%ls> ", win.ircproto->nick);
	wattroff(win.win, style.mod_nick);

	if(!flag_tome) wattroff(win.win, stylerot_get(win.ircproto->tonick));

	wprintw(win.win, "%ls\n", win.ircproto->params);

	if(flag_tome) wattroff(win.win, style.priv);

} /*win_draw_line_privmsg()*/


static void
win_draw_line_myprivmsg(void)
{
	int h;
	int w;

	getmaxyx(win.win, h, w);
	wmove(win.win, h - 1, 0);

	wattron(win.win, style.time);
	wprintw(win.win, "%ls", win.ircproto->tshort);
	wattroff(win.win, style.time);

	wattron(win.win, style.priv);
	wprintw(win.win, " [%ls] ", win.ircproto->tonick);

	wattron(win.win, style.mod_nick);
	wprintw(win.win, "<%ls> ", win.ircproto->nick);
	wattroff(win.win, style.mod_nick);

	wprintw(win.win, "%ls\n", win.ircproto->params);
	wattron(win.win, style.priv);
} /*win_draw_line_myprivmsg()*/


static void
win_draw_line_error(void)
{
	int h;
	int w;

	getmaxyx(win.win, h, w);
	wmove(win.win, h - 1, 0);

	wattron(win.win, style.error);
	wprintw(win.win, "%ls %lc %ls\n",
		win.ircproto->t,
		win.ircproto->mark,
		win.ircproto->params);
	wattroff(win.win, style.error);
} /*win_draw_line_error()*/


static void
win_draw_line_myunspecified(void)
{
	int h;
	int w;

	getmaxyx(win.win, h, w);
	wmove(win.win, h - 1, 0);

	wattron(win.win, style.time);
	wprintw(win.win, "%ls", win.ircproto->tshort);
	wattroff(win.win, style.time);

	wattron(win.win, style.priv);
	wattron(win.win, style.mod_nick);
	wprintw(win.win, " <%ls> ", mynick);
	wattroff(win.win, style.mod_nick);

	wprintw(win.win, "%ls\n", win.ircproto->params);
	wattron(win.win, style.priv);
} /*win_draw_line_myunspecified()*/
