#include <in/in.h>
#include <global/global.h>

#include <ncursesw/curses.h>
#include <wchar.h> //wchar_t
#include <string.h> //strlen()
#include <stdlib.h> //mbstowcs()
#include <stdio.h> //fopen()

/*todo
 * -NOK not handling, get better mlterm -hline is not displayed in mlterm-fb
 * +OK send pointer in out.c is not good, garbling, needs buffer
 * +OK send in out.c need : separator
 * +OK in.c when sending empty message it should not send
 * +OK basic editing of input line, minimum to backspace things
 * +OK -with proper locale() - cyr input not working bulgarian 'r' etc.
 * +NOK - too much extras - red input row on error command
 * +OK - how to write in green in input
 * +OK (forgot to implement) sometimes PONG comes in window 
 * +OK in.curx and line overflow correction, because wen out of scope
 * resize needed :) vertical OK, horizontal buggy
 * +OK due to overlaping windows -
 *    maybe panels needed? (sometimes wrong init window)
 * /exit implementation
 * /cls implementation (with the matrix)
 * logo implementation (asccii art don't panic Douglas Adams)
 * argv sane implementation
 * go to github
 */
//input data    fd    win   pan   buf  buf_offset
static In in = {NULL, NULL, NULL, L"", 0};

extern Style style;
extern wchar_t tosend[CHANLEN + 1];
extern int count_tome;


int
in_init(const char filename[PATHLEN])
{
	int h;
	int w;

	in.fd = fopen(filename, "r+");
	if(in.fd == NULL)
	{
		/*handle*/
	}

	getmaxyx(stdscr, h, w);
	in.win = newwin(1, w, h - 1, 0);
	wtimeout(in.win, INTIMEOUT);
	keypad(in.win, TRUE);
	in.pan = new_panel(in.win);

	return 0;

} /*in_init()*/


void
in_del(void)
{
	if(in.win != NULL) delwin(in.win);
	if(in.pan != NULL) del_panel(in.pan);
	if(in.fd != NULL) fclose(in.fd);
	in.buf_offset = 0;
} /*in_del()*/


void
in_draw(void)
{
	int h;
	int w;
	int i;

	getmaxyx(in.win, h, w);

	i = in.buf_offset - w + 1;
	i = MAX(0, i);
	/*
	in.buf_offset - x = w - 1;
	in.buf_offset - w + 1 = x;
	*/


	wmove(in.win, 0, 0);
	wattron(in.win, style.input);
	for(i=i; i < in.buf_offset; i++)
	{
		wprintw(in.win, "%lc", in.buf[i]);
	}
	wattroff(in.win, style.input);
	wclrtoeol(in.win);

	wrefresh(in.win); //???
} /*in_draw()*/


void
in_input(void)
{
	wchar_t ch;
	int h;
	int w;

	getmaxyx(in.win, h, w);
	wmove(in.win, 0, MIN(in.buf_offset, w - 1));

	wget_wch(in.win, &ch);

	switch(ch)
	{
		case ERR:
		case 0: break;
		case KEY_ENTER:
		case L'\n':
			in_input_process();
			count_tome = 0;
			break;
		case KEY_BACKSPACE:
		case 127:
		case 8:
			in_key_backspace();
			break;
		case KEY_LEFT:
			in_key_backspace();
			break;
		case KEY_DOWN: break;
		case KEY_UP: break;
		case KEY_RIGHT: break;
		default:
			if(!(ch & KEY_CODE_YES)) in_input_buffer(ch);
			break;
	}
} /*in_input()*/


static void
in_input_buffer(const wchar_t ch)
{
	if(in.buf_offset < INBUFLEN - 1) in.buf[in.buf_offset++] = ch;
} /*in_input_buffer()*/


static void
in_input_process(void)
{
	in.buf[in.buf_offset] = L'\0';

	if(wcsncmp(in.buf, L"/to", 3) == 0) in_cmd_to();
	else if(wcsncmp(in.buf, L"/raw", 4) == 0) in_cmd_raw();
	else if(wcsncmp(in.buf, L"/msg", 4) == 0) in_cmd_msg();
	else if(wcsncmp(in.buf, L"/join", 5) == 0) in_cmd_join();
	else if(wcsncmp(in.buf, L"/part", 5) == 0) in_cmd_part();
	else if(wcsncmp(in.buf, L"/quit", 5) == 0) in_cmd_quit();
	else if(wcsncmp(in.buf, L"/exit", 5) == 0) in_cmd_exit();
	else if(in.buf[0] == L'/') /*do nothing*/;
	else in_cmd_msg_default();

	in.buf[0] = L'\0';
	in.buf_offset = 0;
} /*in_input_process()*/


static void
in_cmd_to(void)
{
	wchar_t *p;

	p = in.buf + 3;
	if(*p == L'\0')
	{
		tosend[0] = L'\0';
		return;
	}

	p++;
	wcsncpy(tosend, p, CHANLEN);
	tosend[CHANLEN] = L'\0';
} /*in_cmd_to()*/


static void
in_cmd_raw(void)
{
	wchar_t *p;
	char cbuf[CBUFLEN];
	char cinbuf[CBUFLEN];

	p = in.buf + 4;
	if(*p == L'\0') return;
	p++;
	if(*p == L'\0') return;

	wcstombs(cinbuf, p, CBUFLEN - 1);
	cinbuf[CBUFLEN - 1] = '\0';

	snprintf(cbuf, CBUFLEN - 1, "%s\n", cinbuf);
	cbuf[CBUFLEN - 1] = '\0';

	in_send_pipe(cbuf);
} /*in_cmd_raw()*/


static void
in_cmd_msg(void)
{
	wchar_t *p;
	char cbuf[CBUFLEN];
	char cinbuf[CBUFLEN];

	p = in.buf + 4;

	wcstombs(cinbuf, p, CBUFLEN - 1);
	cinbuf[CBUFLEN - 1] = '\0';

	snprintf(cbuf, CBUFLEN - 1, "PRIVMSG%s\n", cinbuf);
	cbuf[CBUFLEN - 1] = '\0';

	in_send_pipe(cbuf);
} /*in_cmd_msg()*/


static void
in_cmd_join(void)
{
	wchar_t *p;
	char cbuf[CBUFLEN];
	char cinbuf[CBUFLEN];

	p = in.buf + 5;
	if(*p == L'\0') return;

	wcstombs(cinbuf, p, CBUFLEN - 1);
	cinbuf[CBUFLEN - 1] = '\0';

	snprintf(cbuf, CBUFLEN - 1, "JOIN%s\n", cinbuf);
	cbuf[CBUFLEN - 1] = '\0';

	in_send_pipe(cbuf);
} /*in_cmd_join()*/


static void
in_cmd_part(void)
{
	wchar_t *p;
	char cbuf[CBUFLEN];
	char cinbuf[CBUFLEN];

	p = in.buf + 5;
	if(*p == L'\0') return;

	wcstombs(cinbuf, p, CBUFLEN - 1);
	cinbuf[CBUFLEN - 1] = '\0';

	snprintf(cbuf, CBUFLEN - 1, "PART%s\n", cinbuf);
	cbuf[CBUFLEN - 1] = '\0';

	in_send_pipe(cbuf);
} /*in_cmd_part()*/


static void
in_cmd_quit(void)
{
	wchar_t *p;
	char cbuf[CBUFLEN];

	p = in.buf + 5;

	snprintf(cbuf, CBUFLEN - 1, "QUIT\n");
	cbuf[CBUFLEN - 1] = '\0';

	in_send_pipe(cbuf);
} /*in_cmd_quit()*/


static void
in_cmd_exit(void)
{
} /*in_cmd_exit()*/


static void
in_cmd_msg_default(void)
{
	char cbuf[CBUFLEN];
	char ctosend[4*CHANLEN + 1];
	char cinbuf[CBUFLEN];
	wchar_t *p;

	if(tosend[0] == L'\0')
	{
		/*todo:handle error color*/
		return;
	}

	p = in.buf;

	while(*p == L' ') p++;
	if(*p == L'\0')
	{
		/*todo*/
		return;
	}

	wcstombs(ctosend, tosend, 4*CHANLEN);
	ctosend[4*CHANLEN] = '\0';
	wcstombs(cinbuf, in.buf, CBUFLEN - 1);
	cinbuf[CBUFLEN - 1] = '\0';

	snprintf(cbuf, CBUFLEN - 1, "PRIVMSG %s %s\n", ctosend, cinbuf);
	cbuf[CBUFLEN - 1] = '\0';

	in_send_pipe(cbuf);
} /*in_cmd_msg_default()*/


static int
in_send_pipe(const char buf[CBUFLEN])
{
	fwrite(buf, sizeof(char), strlen(buf), in.fd);
	fflush(in.fd);
} /*in_send_pipe()*/


static void
in_key_backspace(void)
{
	if(in.buf_offset > 0) in.buf_offset--;
} /*in_key_backspace()*/
