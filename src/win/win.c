#include <win/win.h>
#include <global/global.h>
#include <index/index.h>

#include <ncursesw/curses.h>
#include <stdio.h> //fopen()
#include <string.h> //strncmp()
#include <time.h> //localtime_n()
#include <stdlib.h> //mbstowcs()
#include <wchar.h> //wchar_t


/*win data        fd    win   pan   read_bytes display_bytes current_row_style*/
static Win win = {NULL, NULL, NULL, 0,         0,            0};

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

	win.fd = fopen(filename, "r");
	if(win.fd == NULL)
	{
		win_del();
		return -1;
	}

	win.index = index_new(100); /*todo:100*/
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
	if(win.index != NULL) index_del(win.index);
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
	if(start < win.display_bytes) return;

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

		win_read(buf, start, end);
		buf[strcspn(buf, "\r")] = '\0';
		if(!win_parse(buf, wbuf, &styl)) continue;
		win_draw_entry(wbuf, styl);
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
win_read(char buf[CBUFLEN + 1], const size_t start, const size_t end)
{
	size_t read_size = end - start;
	read_size = MIN(read_size, CBUFLEN + 1);

	if(fseek(win.fd, start, SEEK_SET) != 0) return;

	fread(buf, sizeof(char), read_size, win.fd); /*how to check success?*/
	buf[read_size - 1] = '\0';
} /*win_read()*/


static int
win_parse(const char buf[CBUFLEN + 1], wchar_t wbuf[WBUFLEN + 1], int *styl)
{
	if(buf[MARKPOSITION] == '<')
	{
		return win_parse_fromirc(buf, wbuf, styl);
	}
	else if(buf[MARKPOSITION] == '>')
	{
		return win_parse_toirc(buf, wbuf, styl);
	}
	else if(buf[MARKPOSITION] == '!')
	{
		mbstowcs(wbuf, buf, WBUFLEN);
		wbuf[WBUFLEN] = '\0';
		*styl = style.error;
		return 1;
	}
	else
		return 0;
} /*win_parse()*/


static int
win_row_style_flip(void)
{
	if(win.current_row_style == style.row_a)
		win.current_row_style = style.row_b;
	else
		win.current_row_style = style.row_a;

	return win.current_row_style;
} /*win_style_flip()*/


static int
win_parse_fromirc(const char buf[CBUFLEN + 1], wchar_t wbuf[WBUFLEN + 1],
		  int *styl)
{
	wchar_t t[TIMELEN + 1];
	wchar_t nick[NICKLEN + 1];
	wchar_t cmd[CMDLEN + 1];
	wchar_t chan[CHANLEN + 1];
	wchar_t msg[WBUFLEN + 1]; //can be avoided?
	wchar_t *p;
	wchar_t *pbuf;
	int i;

	mbstowcs(wbuf, buf, WBUFLEN);
	wbuf[WBUFLEN] = L'\0';

	pbuf = wbuf;

	p = t;
	for(i = 0; i < TIMELEN; i++) *p++ = *pbuf++;
	*p = L'\0';

	pbuf += 3;
	if(*pbuf != L':') return 0;
	pbuf++;

	p = nick;
	i = 0;
	while(*pbuf != L'!')
	{
		if(*pbuf == L'\0') return 0;
		if(i >= NICKLEN) return 0;
		*p++ = *pbuf++;
		i++;
	}
	*p = L'\0';

	while(*pbuf != L' ')
	{
		if(*pbuf == L'\0') return 0;
		pbuf++;
	}

	pbuf++;
	p = cmd;
	i = 0;
	while(*pbuf != L' ')
	{
		if(*pbuf == L'\0') return 0;
		if(i >= CMDLEN) return 0;
		*p++ = *pbuf++;
		i++;
	}
	*p = L'\0';

	if(wcscmp(cmd, L"PRIVMSG") != 0) return 0;

	pbuf++;
	p = chan;
	i = 0;
	while(*pbuf != L' ')
	{
		if(*pbuf == L'\0') return 0;
		if(i >= CHANLEN) return 0;
		*p++ = *pbuf++;
		i++;
	}
	*p = L'\0';

	while(*pbuf != L':')
	{
		if(*pbuf == L'\0') return 0;
		pbuf++;
	}
	pbuf++;

	swprintf(msg, WBUFLEN, L"%ls", pbuf);
	msg[WBUFLEN] = L'\0';

	t[TIMEMINEND + 1] = L'\0';
	p = t + TIMEHRSTART;

	swprintf(wbuf,
		 WBUFLEN,
		 L"%ls [%ls] %ls: %ls",
		 p, chan, nick, msg);
	wbuf[WBUFLEN] = L'\0';

	if(wcsncmp(mynick, chan, NICKLEN) == 0)
	{
		*styl = style.row_tome;
		count_tome++;
	}
	else
	{
	        *styl = win_row_style_flip();
		if(wcsncmp(chan, tosend, CHANLEN) == 0)
		{
			if(*styl == style.row_a) *styl = style.row_a_hl;
			else *styl = style.row_b_hl;
		}
	}

	return 1;
} /*win_parse_fromirc()*/


static int
win_parse_toirc(const char buf[CBUFLEN + 1], wchar_t wbuf[WBUFLEN + 1],
		int *styl)
{
	wchar_t t[TIMELEN + 1];
	wchar_t chan[CHANLEN + 1];
	wchar_t msg[WBUFLEN + 1]; //can be avoided?
	wchar_t *p;
	wchar_t *pbuf;
	int i;

	*styl = style.input;

	mbstowcs(wbuf, buf, WBUFLEN);
	wbuf[WBUFLEN] = L'\0';

	pbuf = wbuf;

	p = t;
	for(i = 0; i < TIMELEN; i++) *p++ = *pbuf++;
	*p = L'\0';

	pbuf += 3;
	if(wcsncmp(pbuf, L"PONG", 4) == 0)
	{
		return 0;
	}
	else if(wcsncmp(pbuf, L"PRIVMSG", 7) != 0) //this is NOT (!=) privmsg
	{
		/*the whole wbuf will be printed, 1 means green light for
		 * printing */
		return 1;
	}

	/*if reached here - parsing PRIVMSG*/

	pbuf += 8;
	if(*pbuf == L'\0') return 0;

	p = chan;
	i = 0;
	while(*pbuf != L' ')
	{
		if(*pbuf == L'\0') return 0;
		if(i >= CHANLEN) return 0;
		*p++ = *pbuf++;
		i++;
	}
	*p = L'\0';

	pbuf++;
	swprintf(msg, WBUFLEN, L"%ls", pbuf);
	msg[WBUFLEN] = L'\0';

        t[TIMEMINEND + 1] = L'\0';
        p = t + TIMEHRSTART;

        swprintf(wbuf,
		 WBUFLEN - 1,
		 L"%ls [%ls] %ls: %ls",
		 p, chan, mynick, msg);
	wbuf[WBUFLEN] = '\0';
	
	return 1;
} /*win_parse_toirc()*/


static void
win_draw_entry(const wchar_t wbuf[WBUFLEN + 1], const int styl)
{
	int h;
	int w;

	getmaxyx(win.win, h, w);

	wmove(win.win, h - 1, 0);
	wattron(win.win, styl);
	wprintw(win.win, "%ls\n", wbuf);
	wattroff(win.win, styl);
} /*win_draw_entry()*/
