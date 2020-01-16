/* Diagram to understand the code
 *
 *   -------------------------
 *   | chat window           | <---- data coming from irclsd
 *   |                       |       output file
 *   |                       |       (argv -fromirc /path/to/file)
 *   |                       |       (object win)
 *   |                       |
 *   -------------------------
 *   | status bar            | ..... (argv -server ... -nick ...) (obj bar)
 *   -------------------------
 *   | user input            | ----> data going to irclsd input
 *   -------------------------       named pipe (fifo)
 *                                   (argv -toirc /path/to/fifo)
 *                                   (object in)
 */
#include <control/control.h>                     //some prototypes
#include <global/global.h>                       //some globals
#include <style/style.h>                         //ncurses styles
#include <bar/bar.h>                             //status bar object
#include <win/win.h>                             //chat window object
#include <in/in.h>                               //user input object
#include <argparse/argparse.h>                   //arguments parsing object

#include <ncursesw/curses.h>
/* ncurses panels are used only because chat window has it's bottom line always
 * empty (for scrolling purposes). We place the status bar on top of this line.
 * Whenever overlaping windows on z-axis, you need to always use panels.
 */
#include <panel.h>
#include <locale.h>                              //setlocale()
#include <wchar.h>                               //wide characters
#include <stdlib.h>                              //exit()
#include <stdio.h>                               //printf()


extern wchar_t mynick[NICKLEN + 1];              //these coming from globals.c
extern wchar_t server[SERVERLEN + 1];
extern wchar_t tosend[CHANLEN + 1];

/*
 * Initializes the control
 * Gets arguments as input from main
 */
void
control_init(int argc, char *argv[])
{
	setlocale(LC_ALL, "");                   //will get unicode arguments
	                                         //and display wide chars

	ArgParse *argparse = argparse_new(argc, argv); //parse arguments
	                                               //argparse exits on err

	initscr();                               //configure ncurses screen
	noecho();

	/* Initialize all styles. The styles are written to a global variable
	 * 'extern style' in order to be used by all objects*/
	style_init();

	if(win_init(argparse->fromirc) != 0)     //init chat window
	{
		/*TODO*/
	}

	bar_init();                              //init status bar

	if(!in_init(argparse->toirc))            //init input field
	{
		control_del();
		printf("*ERROR 206* cannot open toirc fifo\n");
		exit(6);
	}

	update_panels();                         //order panels at z axis

	wcsncpy(mynick, argparse->nick, NICKLEN); //copy given nick&server
	mynick[NICKLEN] = L'\0';
	wcsncpy(server, argparse->server, SERVERLEN);
	server[SERVERLEN] = L'\0';

	argparse_del(argparse);                  //not needed anymore-free mem
} /*control_init()*/


/*
 * Frees the memory, closes all file handles
 */
void
control_del(void)
{
	bar_del();
	win_del();
	in_del();
	endwin();                                //end stdsrc
} /*control_del()*/


/*
 * Control event loop
 */
void
control_loop(void)
{
	while(1)
	{
		win_draw();                      //draws chat window
		bar_draw();                      //draws status bar
		in_input();                      //gets user input, here we
		                                 //have delay aka sleep
		in_draw();                       //draws user input on screeen
	}
} /*control_loop()*/
