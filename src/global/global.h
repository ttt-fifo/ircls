/* 
 * Global definitions used all over the program
 */
#ifndef GLOBAL_H
#define GLOBAL_H

/*logo file to display on startup*/
#define LOGO "data/logo.txt"

#define PATHLEN 1023                             //length of the paths
#define WBUFLEN 1023                             //wchar buffers length
#define CBUFLEN 4095                             //char buffers length
#define NICKLEN 16                               //nickname length
#define CHANLEN 32                               //channels length
#define SERVERLEN 128                            //server name length
#define COUNTLEN 4                               //count of priv msgs string
#define CHMULTIPLY 4                             //multiplier between ch -> wch


#define MIN(a,b) (((a)<(b))?(a):(b))             //minimum macro
#define MAX(a,b) (((a)>(b))?(a):(b))             //maximum macro


/*
 * ncurses styles are exported here by style_init()
 * to be used by all objects in the program
 */
typedef struct Style
{
	int time;                                //action time
	int priv;                                //private messages to me
	int error;                               //messages error
	int bar;                                 //status bar main style
	int bar_txtbox;                          //textbox in the status bar
	int count;                               //msg count in status bar
	int count_highlight;                     //msg count highlighted
	int border;                              //borders style
	int input;                               //style of user input
	int mod_nick;                            //modifier of sending nick
} Style;


/*
 * Helper to give random integer from specified range
 * lower: lowest possible integer to give
 * upper: upper possible integer to give
 * Returns: a random integer in range lower - upper
 * NOTE: (!) before using this function a random seed should be set manually
 *       e.g: srand(time(0));
 */
int
range_random(int lower, int upper);


#endif /*GLOBAL_H*/
