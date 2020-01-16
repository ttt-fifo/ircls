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


#define INTIMEOUT 800                            //timeout waiting for one ch
#define INBUFLEN 256                             //length of input buffer


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
in_init(const char filename[PATHLEN]);

/*
 * Free memory and file descriptors
 */
void
in_del(void);

void
in_draw(void);

void
in_input(void);

static void
in_input_buffer(const wchar_t ch);

static void
in_input_process(void);

static void
in_cmd_to(void);

static void
in_cmd_raw(void);

static void
in_cmd_msg(void);

static void
in_cmd_join(void);

static void
in_cmd_part(void);

static void
in_cmd_quit(void);

static void
in_cmd_exit(void);

static void
in_cmd_msg_default(void);

static int
in_send_pipe(const char buf[CBUFLEN]);

static void
in_key_backspace(void);


#endif /*IN_H*/
