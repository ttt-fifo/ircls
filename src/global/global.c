/* 
 * Global definitions used all over the program
 * .c file is mandatory to be able to compile global.{h,c} object file
 */
#include <global/global.h>

#include <wchar.h>                               //wchar_t
#include <sys/time.h>                            //timeval
#include <stdlib.h>



/*
 * These are initialized here and used all over the program
 * via 'extern type var;'
 */
Style style; /*todo:init here*/
/*FIXME: do we need count_tome here? It seams only bar uses it*/
int count_tome = 0;                              //private msg received
wchar_t mynick[NICKLEN + 1] = L"";               //nick name
wchar_t server[SERVERLEN + 1] = L"";             //server name
wchar_t tosend[CHANLEN + 1] = L"";               //channel to send all msg


//NOTE: srand(time(0)) should be invoked before this function
int
range_random(int lower, int upper)
{
	int num = (rand() % (upper - lower + 1)) + lower;
	return num;
} /*range_random()*/
