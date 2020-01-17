#include <in/in.h>                               //prototypes
#include <global/global.h>                       //CHANLEN, etc

#include <ncursesw/curses.h>
#include <wchar.h>                               //wchar_t
#include <string.h>                              //strlen()
#include <stdlib.h>                              //mbstowcs()
#include <stdio.h>                               //fopen()


//input data    fd    win   pan   buf  buf_offset
static In in = {NULL, NULL, NULL, L"", 0};

/*these are coming from globals.c*/
extern Style style;                              //ncurses styles
extern wchar_t tosend[CHANLEN + 1];              //default channel to send
extern int count_tome;                           //count of my private msg


/*
 * Initializes input
 * filename: the name of named pipe (fifo) to write user input to
 * Returns: OK-1, ERR-0
 */
int
in_init(const char filename[PATHLEN + 1])
{
	int h;                                   //height
	int w;                                   //width

	/* open the fifo, r+ means for read/write, but do not create
	 * write is needed not to hang waiting for writer on the other end
	 */
	in.fd = fopen(filename, "r+");
	if(in.fd == NULL) return 0;              //ERR

	getmaxyx(stdscr, h, w);

	in.win = newwin(1, w, h - 1, 0);         //window with desired geometry
	wtimeout(in.win, INTIMEOUT);             //non blocking input
	keypad(in.win, TRUE);                    //keypad on

	in.pan = new_panel(in.win);              //pannel for z axis overlap

	return 1;                                //OK

} /*in_init()*/


/*
 * Free memory and file descriptors
 */
void
in_del(void)
{
	if(in.win != NULL) delwin(in.win);
	if(in.pan != NULL) del_panel(in.pan);
	if(in.fd != NULL) fclose(in.fd);

	in.buf_offset = 0;
} /*in_del()*/


/*
 * Draws input field on virtual screen
 */
void
in_draw(void)
{
	int h;                                   //height
	int w;                                   //width
	int i;                                   //index for the buffer

	getmaxyx(in.win, h, w);

	/* start drawing the buffer eighter at 0 character
	 * or skip front characters if buffer larger than width
	 */
	i = in.buf_offset - w + 1;
	i = MAX(0, i);

	wmove(in.win, 0, 0);
	wattron(in.win, style.input);
	for(i=i; i < in.buf_offset; i++)         //using loop because string
	{                                        //is not null terminated
		wprintw(in.win, "%lc", in.buf[i]);
	}
	wattroff(in.win, style.input);
	wclrtoeol(in.win);                       //needed for backspace

	wrefresh(in.win);                        //draw on virtual screen
} /*in_draw()*/

/*
 * Gets the input from the user
 */
void
in_input(void)
{
	wchar_t ch;                              //wide char to get
	int h;                                   //height
	int w;                                   //width

	/*arrange cursor possition at the end of string
	 * or at the end of width*/
	getmaxyx(in.win, h, w);
	wmove(in.win, 0, MIN(in.buf_offset, w - 1));

	wget_wch(in.win, &ch);

	switch(ch)                               //act according to input
	{
		case ERR:                        //no input
		case 0: break;
		case KEY_ENTER:                  //line termination
		case L'\n':
			in_input_process();
			count_tome = 0;
			break;
		case KEY_BACKSPACE:              //backspace dels one char
		case 127:
		case 8:
			in_key_backspace();
			break;
		case KEY_LEFT:                   //left arrow same as backspace
			in_key_backspace();
			break;
		case KEY_DOWN: break;            //other arrows ignored
		case KEY_UP: break;
		case KEY_RIGHT: break;
		default:                         //other keys go in buffer
			if(!(ch & KEY_CODE_YES)) in_input_buffer(ch);
			break;
	}
} /*in_input()*/


/*
 * Inputs a wide character into buffer
 * ch: the wide char to input
 */
static void
in_input_buffer(const wchar_t ch)
{
	/*buffer overflow protected*/
	if(in.buf_offset < INBUFLEN - 1) in.buf[in.buf_offset++] = ch;
} /*in_input_buffer()*/


/*
 * Processes a command line from the user
 */
static void
in_input_process(void)
{
	in.buf[in.buf_offset] = L'\0';           //terminate buf string

	/*react according to the command*/
	if(wcsncmp(in.buf, L"/to", 3) == 0) in_cmd_to();
	else if(wcsncmp(in.buf, L"/raw", 4) == 0) in_cmd_raw();
	else if(wcsncmp(in.buf, L"/msg", 4) == 0) in_cmd_msg();
	else if(wcsncmp(in.buf, L"/join", 5) == 0) in_cmd_join();
	else if(wcsncmp(in.buf, L"/part", 5) == 0) in_cmd_part();
	else if(wcsncmp(in.buf, L"/quit", 5) == 0) in_cmd_quit();
	else if(wcsncmp(in.buf, L"/exit", 5) == 0) in_cmd_exit();
	else if(in.buf[0] == L'/') /*do nothing*/;
	else in_cmd_msg_default();

	/*clear the buffer for the next user input*/
	in.buf[0] = L'\0';
	in.buf_offset = 0;
} /*in_input_process()*/


/* 
 * Processes user command '/to'
 * Copies the given channel to the global variable
 */
static void
in_cmd_to(void)
{
	wchar_t *p;

	p = in.buf + 3;                          //skip to the channel
	if(*p == L'\0')
	{
		tosend[0] = L'\0';
		return;
	}

	p++;
	wcsncpy(tosend, p, CHANLEN);             //copy to the global variable
	tosend[CHANLEN] = L'\0';                 //'extern wchar_t tosend'
} /*in_cmd_to()*/


/*
 * Processes user command '/raw'
 * Gets IRC protocol raw input from the user
 */
static void
in_cmd_raw(void)
{
	wchar_t *p;
	char cbuf[CBUFLEN];                      //char buffer to send
	char cinbuf[CBUFLEN];                    //multibyte in.buf

	p = in.buf + 4;                          //move to data after command
	if(*p == L'\0') return;
	p++;
	if(*p == L'\0') return;

	wcstombs(cinbuf, p, CBUFLEN - 1);        //put in multibyte cbuf
	cinbuf[CBUFLEN - 1] = '\0';

	snprintf(cbuf, CBUFLEN - 1, "%s\n", cinbuf); //arrange buffer to send
	cbuf[CBUFLEN - 1] = '\0';

	in_send_pipe(cbuf);                      //send to irclsd pipe
} /*in_cmd_raw()*/


/* 
 * Processes user command '/msg'
 * Sends message to channel or user
 */
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
