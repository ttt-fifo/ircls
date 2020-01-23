/* Rotating style used to assign to different chat conversations
 */
#ifndef STYLEROT_H
#define STYLEROT_H


#include <global/global.h>                       //CHANLEN

#include <wchar.h>                               //wchar_t


#define STYLEROT_ENTRIES 50                      //maximum conversations, after
                                                 //this number - only white


/*
 * Enumerate the rotational styles index
 * starts from 25. The other user defined styles should be less than 25
 */
enum StyleRotEnum {STYLEROT_MAGENTA = 25, STYLEROT_CYAN, STYLEROT_BLUE,
	           STYLEROT_RED, STYLEROT_GREEN, STYLEROT_WHITE};


/* 
 * One entry consist of the "to nick", or channel/nick name
 * and the ncurses style.
 */
typedef struct StyleRotEntry
{
	wchar_t tonick[CHANLEN + 1];
	int style;
} StyleRotEntry;


/*
 * Initialize ncurses styles
 */
void
stylerot_init(void);

/*
 * Returns the style of the given "to nick"
 * tonick: the given "to nick" we need to get the style for
 * Returns: int - the ncurses color style integer
 */
int
stylerot_get(const wchar_t tonick[CHANLEN + 1]);


#endif /*STYLEROT_H*/
