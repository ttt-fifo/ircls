/* 
 * Global definitions used all over the program
 */
#ifndef GLOBAL_H
#define GLOBAL_H


#define PATHLEN 1023                             //length of the paths
#define WBUFLEN 1023                             //wchar buffers length
#define CBUFLEN 4096                             //char buffers length
#define NICKLEN 16                               //nickname length
#define CHANLEN 32                               //channels length
#define SERVERLEN 128                            //server name length
#define COUNTLEN 4                               //count of priv msgs string


#define MIN(a,b) (((a)<(b))?(a):(b))             //minimum macro
#define MAX(a,b) (((a)>(b))?(a):(b))             //maximum macro


/*
 * ncurses styles are exported here by style_init()
 * to be used by all objects in the program
 */
typedef struct Style
{
	int row_a;                               //row even representation
	int row_b;                               //row odd representation
	int row_a_hl;                            //row even highlighted
	int row_b_hl;                            //row odd representation
	int row_tome;                            //messages to me
	int error;                               //messages error
	int bar;                                 //status bar main style
	int bar_txtbox;                          //textbox in the status bar
	int count;                               //msg count in status bar
	int count_highlight;                     //msg count highlighted
	int border;                              //borders style
	int input;                               //style of user input
} Style;


#endif /*GLOBAL_H*/
