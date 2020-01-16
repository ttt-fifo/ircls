/* 
 * This is the status bar object for the ncurses interface
 */
#ifndef BAR_H
#define BAR_H


#include <ncursesw/curses.h>                     //WINDOW
#include <panel.h>                               //PANEL


/*the status bar data*/
typedef struct Bar
{
	WINDOW *win;                             //here data is drawn
	PANEL *pan;                              //panel needed for overlaping
	                                         //over the chat window
} Bar;


/*
 * Initializes status bar object
 */
void
bar_init(void);

/*
 * Frees the memory
 */
void
bar_del(void);

/*
 * Draws the status bar
 */
void
bar_draw(void);


#endif /*BAR_H*/
