/* 
 * User input object
 * Gets the input from user and sends it to irclsd named pipe (fifo)
 */
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

	/* open the fifo, r+ means for read/write, but do not create.
	 * write is needed not to hang waiting for writer on the other end
	 */
	in.fd = fopen(filename, "r+");
	if(in.fd == NULL) return 0;

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
int
in_input(void)
{
	wchar_t ch;                              //wide char to get
	int h;                                   //height
	int w;                                   //width

	/*arrange cursor possition at the end of string
	 * or at the end of width*/
	getmaxyx(in.win, h, w);
	wmove(in.win, 0, MIN(in.buf_offset, w - 1));

	wget_wch(in.win, &ch);                   //get one wide char

	switch(ch)                               //act according to input
	{
		case ERR:                        //no input
		case 0: break;
		case KEY_ENTER:                  //line termination
		case L'\n':
			count_tome = 0;
			return in_input_process(); //return value to control
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

	return INPUTRV_OK;                       //default return val is OK
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
static int
in_input_process(void)
{
	int rv = INPUTRV_OK;                     //return val (default OK)

	in.buf[in.buf_offset] = L'\0';           //terminate buf string

	/*react according to the command*/
	if(wcsncmp(in.buf, L"/to", 3) == 0) in_cmd_to();
	else if(wcsncmp(in.buf, L"/raw", 4) == 0) in_cmd_raw();
	else if(wcsncmp(in.buf, L"/msg", 4) == 0) in_cmd_msg();
	else if(wcsncmp(in.buf, L"/join", 5) == 0) in_cmd_join();
	else if(wcsncmp(in.buf, L"/part", 5) == 0) in_cmd_part();
	else if(wcsncmp(in.buf, L"/quit", 5) == 0) in_cmd_quit();
	else if(wcsncmp(in.buf, L"/exit", 5) == 0) rv = INPUTRV_EXIT;
	else if(wcsncmp(in.buf, L"/cls", 4) == 0) rv = INPUTRV_CLS;
	else if(in.buf[0] == L'/') /*do nothing*/;
	else in_cmd_msg_default();

	/*clear the buffer for the next user input*/
	in.buf[0] = L'\0';
	in.buf_offset = 0;

	return rv;
} /*in_input_process()*/


/* 
 * Processes user command '/to'
 * Copies the given channel to the global variable
 * Sets "To:" in status bar. The effect of setting it is that all messages
 * typed by the user will be going to this channel, also other channels are
 * filtered and their messages are not visible anymore (until unsetting "To:")
 * Unsetting is don by typing an empty '/to' in command line.
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
	char cbuf[CBUFLEN + 1];                  //char buffer to send
	char cinbuf[CBUFLEN + 1];                //multibyte in.buf

	p = in.buf + 4;                          //move to data after command
	if(*p == L'\0') return;
	p++;
	if(*p == L'\0') return;

	wcstombs(cinbuf, p, CBUFLEN);            //put in multibyte cbuf
	cinbuf[CBUFLEN] = '\0';

	snprintf(cbuf, CBUFLEN, "%s\n", cinbuf); //arrange buffer to send
	cbuf[CBUFLEN] = '\0';

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
	char cbuf[CBUFLEN + 1];
	char cinbuf[CBUFLEN + 1];

	p = in.buf + 4;

	wcstombs(cinbuf, p, CBUFLEN);
	cinbuf[CBUFLEN] = '\0';

	/*TODO: this is not working, should have : before message*/
	snprintf(cbuf, CBUFLEN, "PRIVMSG%s\n", cinbuf);
	cbuf[CBUFLEN] = '\0';

	in_send_pipe(cbuf);
} /*in_cmd_msg()*/


/*
 * Processes user command /join
 * Joins to a channel
 */
static void
in_cmd_join(void)
{
	wchar_t *p;
	char cbuf[CBUFLEN + 1];
	char cinbuf[CBUFLEN + 1];

	p = in.buf + 5;                          //move after user command
	if(*p == L'\0') return;

	wcstombs(cinbuf, p, CBUFLEN);            //convert to multibyte
	cinbuf[CBUFLEN] = '\0';

	snprintf(cbuf, CBUFLEN, "JOIN%s\n", cinbuf); //arrange IRC proto cmd
	cbuf[CBUFLEN] = '\0';

	in_send_pipe(cbuf);                      //send to IRC socket
} /*in_cmd_join()*/


/*
 * Processes user command /part
 * Parts from a channel
 */
static void
in_cmd_part(void)
{
	wchar_t *p;
	char cbuf[CBUFLEN + 1];
	char cinbuf[CBUFLEN + 1];

	p = in.buf + 5;                          //move to end of user command
	if(*p == L'\0') return;

	wcstombs(cinbuf, p, CBUFLEN);            //convert to multibyte
	cinbuf[CBUFLEN] = '\0';

	snprintf(cbuf, CBUFLEN, "PART%s\n", cinbuf); //IRC protocol command
	cbuf[CBUFLEN] = '\0';

	in_send_pipe(cbuf);                      //send to IRC socket
} /*in_cmd_part()*/


/*
 * Processes user command /quit
 * Quits the connection to server.
 * Does not exit the client process (there is command /exit for this)
 */
static void
in_cmd_quit(void)
{
	in_send_pipe("QUIT\n");
} /*in_cmd_quit()*/


/*
 * Sends a message to the user commanded "To:" channel (the "To:" field in the 
 * status bar). When "To:" field in status bar is configured, then everything
 * user inputs is send directly as a message to the "To:" channel.
 */
static void
in_cmd_msg_default(void)
{
	char cbuf[CBUFLEN + 1];                  //character string buffer
	char ctosend[CHMULTIPLY*CHANLEN + 1];    //char string tosend
	char cinbuf[CBUFLEN + 1];                //char string for in.buf
	wchar_t *p;                              //pointer for manipulations

	if(tosend[0] == L'\0')                   //tosend not configured
	{                                        //so do nothing
		return;
	}

	p = in.buf;

	while(*p == L' ') p++;                   //rewind empty spaces
	if(*p == L'\0')
	{
		return;
	}

	/*convert these to multibyte strings*/
	wcstombs(ctosend, tosend, CHMULTIPLY*CHANLEN);
	ctosend[CHMULTIPLY*CHANLEN] = '\0';
	wcstombs(cinbuf, in.buf, CBUFLEN);
	cinbuf[CBUFLEN] = '\0';

	/*construct buffer to send*/
	snprintf(cbuf, CBUFLEN, "PRIVMSG %s :%s\n", ctosend, cinbuf);
	cbuf[CBUFLEN] = '\0';

	in_send_pipe(cbuf);                      //send to irc 
} /*in_cmd_msg_default()*/


/* 
 * Sends string to ircls pipe
 * buf: character string to send
 */
static int
in_send_pipe(const char buf[CBUFLEN + 1])
{
	fwrite(buf, sizeof(char), strlen(buf), in.fd);
	fflush(in.fd);
	return 0; /*TODO: some error checking?*/
} /*in_send_pipe()*/


/*
 * Processes backspace key pressed
 */
static void
in_key_backspace(void)
{
	/*returns one char back in buffer, discards last char*/
	if(in.buf_offset > 0) in.buf_offset--;
} /*in_key_backspace()*/
