#ifndef STYLEROT_H
#define STYLEROT_H


#include <global/global.h>

#include <wchar.h>


#define STYLEROT_ENTRIES 50


enum StyleRotEnum {STYLEROT_MAGENTA = 25, STYLEROT_CYAN, STYLEROT_BLUE,
	           STYLEROT_RED, STYLEROT_GREEN, STYLEROT_WHITE};


typedef struct StyleRotEntry
{
	wchar_t tonick[CHANLEN + 1];
	int style;
} StyleRotEntry;


void
stylerot_init(void);

void
stylerot_del(void);

int
stylerot_get(const wchar_t tonick[CHANLEN + 1]);


#endif /*STYLEROT_H*/
