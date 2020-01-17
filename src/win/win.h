#ifndef WIN_H
#define WIN_H


#include <global/global.h>
#include <index/index.h>

#include <ncursesw/curses.h> //WINDOW
#include <panel.h> //PANEL
#include <stdio.h> //FILE
#include <wchar.h> //wchar_t


#define MARKPOSITION 20
#define TIMELEN 19
#define TIMEHRSTART 11
#define TIMEMINEND 15
#define CMDLEN 7
#define HISTORYBUF 100   //must be greater than window height


typedef struct Win
{
	FILE *fd;
	WINDOW *win;
	PANEL *pan;
	Index *index;
	size_t read_bytes;
	size_t display_bytes;
	int current_row_style;
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
win_read(char buf[CBUFLEN], const size_t start, const size_t end);

static int
win_parse(const char buf[CBUFLEN], wchar_t wbuf[WBUFLEN], int *styl);

static int
win_row_style_flip(void);

static int
win_parse_fromirc(const char buf[CBUFLEN], wchar_t wbuf[WBUFLEN], int *styl);

static int
win_parse_toirc(const char buf[CBUFLEN], wchar_t wbuf[WBUFLEN], int *styl);

static void
win_draw_entry(const wchar_t wbuf[WBUFLEN], const int styl);


#endif /*WIN_H*/
