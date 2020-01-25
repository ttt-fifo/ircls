/* 
 * User input object
 * Gets the input from user and sends it to irclsd named pipe (fifo)
 */
#ifndef IN_H
#define IN_H


#include <global/global.h>                       //PATHLEN

#include <ncursesw/curses.h>
#include <panel.h>                               //PANEL
#include <stdio.h>                               //FILE


#define INTIMEOUT 250                            //timeout waiting for one ch
                                                 //(milliseconds)
#define INBUFLEN 256                             //length of input buffer


/*
 * Return values from user input
 * functions: in_input() and in_input_process()
 */
enum InputRvEnum {INPUTRV_OK=0, INPUTRV_EXIT, INPUTRV_CLS};


/*in data type struct*/
typedef struct In
{
	FILE *fd;                                //file descriptor for fifo
	WINDOW *win;                             //ncurses window
	PANEL *pan;                              //ncurses pane
	wchar_t buf[INBUFLEN];                   //input buffer
	int buf_offset;                          //index next char to write
} In;


/*
 * Initializes input
 * filename: the name of named pipe (fifo) to write user input to
 * Returns: OK-1, ERR-0
 */
int
in_init(const char filename[PATHLEN + 1]);

/*
 * Free memory and file descriptors
 */
void
in_del(void);

/*
 * Draws input field on virtual screen
 */
void
in_draw(void);

/*
 * Gets the input from the user
 */
int
in_input(void);

/*
 * Inputs a wide character into buffer
 * ch: the wide char to input
 */
static void
in_input_buffer(const wchar_t ch);

/*
 * Processes a command line from the user
 */
static int
in_input_process(void);

/* 
 * Processes user command '/to'
 * Copies the given channel to the global variable
 */
static void
in_cmd_to(void);

/*
 * Processes user command '/raw'
 * Gets IRC protocol raw input from the user
 */
static void
in_cmd_raw(void);

/* 
 * Processes user command '/msg'
 * Sends message to channel or user
 */
static void
in_cmd_msg(void);

/*
 * Processes user command /join
 * Joins to a channel
 */
static void
in_cmd_join(void);

/*
 * Processes user command /part
 * Parts from a channel
 */
static void
in_cmd_part(void);

/*
 * Processes user command /quit
 * Quits the connection to server.
 * Does not exit the client process (there is command /exit for this)
 */
static void
in_cmd_quit(void);

/*
 * Sends a message to the user commanded "To:" channel (the "To:" field in the 
 * status bar). When "To:" field in status bar is configured, then everything
 * user inputs is send directly as a message to the "To:" channel.
 */
static void
in_cmd_msg_default(void);

/* 
 * Sends string to ircls pipe
 * buf: character string to send
 */
static int
in_send_pipe(const char buf[CBUFLEN + 1]);

/*
 * Processes backspace key pressed
 */
static void
in_key_backspace(void);


#endif /*IN_H*/
