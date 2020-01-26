/*
 * The ncurses chat window object
 * Reads its data from the irclsd output log file and displays it
 * on the screen in 'tail -f' fashion.
 * There is no buffer history, the 'part', 'join', 'quit' messages are not
 * displayed. Only privmsg are displayed.
 * Feature: when 'To:' is sent by user, all other messages which do not
 * corespond to 'To:' are filtered
 */
#ifndef WIN_H
#define WIN_H


#include <global/global.h>                       //PATHLEN
#include <index/index.h>                         //index object
#include <ircproto/ircproto.h>                   //ircproto object

#include <ncursesw/curses.h>                     //WINDOW
#include <panel.h>                               //PANEL
#include <stdio.h>                               //FILE
#include <wchar.h>                               //wchar_t


/* how many of the trailing lines of irclsd output log file to keep indexed?
 * this needs to be more than the terminal LINES */
#define HISTORYBUF 100


/*data structure for the chat window object*/
typedef struct Win
{
	FILE *fd;                                //file descriptor irclsd log
	WINDOW *win;                             //ncurses window
	PANEL *pan;                              //ncurses panel
	Index *index;                            //indexing object
	size_t read_bytes;                       //how many bytes read from log
	size_t display_bytes;                    //how many bytes displayed
	IrcProto *ircproto;                      //IRC protocol parser
} Win;


/*
 * Initializes the chat window object
 * filename: the filename of the irclsd output log file
 * Returns: 0 OK, negative ERR
 */
int
win_init(const char filename[PATHLEN + 1]);

/*
 * Frees memory and pointers
 */
void
win_del(void);

/*
 * Draws the window on the screen
 */
void
win_draw(void);

/*
 * Indexes the irclsd output log file into the win.index object
 */
static void
win_index_file(void);

/*
 * Reads irclsd output log file between start, end bytes
 * buf: buffer to read into
 */
static void
win_read_file(char buf[CBUFLEN + 1], const size_t start, const size_t end);

/*
 * Draws one line from the given char buffer
 * buf: character buffer to draw on screen
 */
static void
win_draw_line(const char buf[CBUFLEN + 1]);

/*
 * Draws a privmsg line on screen
 */
static void
win_draw_line_privmsg(void);

/* 
 * Draws my privmsg line (privmsg sent from me) on screen
 */
static void
win_draw_line_myprivmsg(void);

/*
 * Draws an error line on screen
 */
static void
win_draw_line_error(void);

/*
 * Draws one line on screen of unspecified type sent from me
 */
static void
win_draw_line_myunspecified(void);

/*
 * Clears screen in funny way
 */
void
win_matrix_reloaded(void);


#endif /*WIN_H*/
