#ifndef WIN_H
#define WIN_H


#include <global/global.h>
#include <index/index.h>
#include <ircproto/ircproto.h>

#include <ncursesw/curses.h> //WINDOW
#include <panel.h> //PANEL
#include <stdio.h> //FILE
#include <wchar.h> //wchar_t


#define HISTORYBUF 100   //must be greater than window height, otherwise loss


typedef struct Win
{
	FILE *fd;
	WINDOW *win;
	PANEL *pan;
	Index *index;
	size_t read_bytes;
	size_t display_bytes;
	IrcProto *ircproto;
} Win;


int
win_init(const char filename[PATHLEN + 1]);

void
win_del(void);

void
win_draw(void);

static void
win_index_file(void);

static void
win_read_file(char buf[CBUFLEN + 1], const size_t start, const size_t end);

static void
win_draw_line(const char buf[CBUFLEN + 1]);

static void
win_draw_line_privmsg(void);

static void
win_draw_line_myprivmsg(void);

static void
win_draw_line_error(void);

static void
win_draw_line_myunspecified(void);

void
win_matrix_reloaded(void);


#endif /*WIN_H*/
