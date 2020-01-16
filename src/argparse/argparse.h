/* Parsing arguments for ircls
 *     -fromirc /path/to/file: this is the irc output file from irclsd
 *     -toirc /path/to/fifo: this is the named pipe (fifo) to irclsd
 *     -server <servername>: ircls needs to know it for status line
 *     -nick <yournick>: ircls needs to know it to highlight messages
 */ 
#ifndef ARGPARSE_H
#define ARGPARSE_H


#include <global/global.h>                       //PATHLEN, NICKLEN, etc.

#include <wchar.h>                               //wide characters


/*enum all argparse keys*/
enum ArgIndex {ARG_UNKNOWN = 0, ARG_FROMIRC, ARG_TOIRC, ARG_NICK, ARG_SERVER};


/*this is the structure where argparse object is parsed to*/
typedef struct ArgParse
{
	char fromirc[PATHLEN];                   //file with output from irclsd
	char toirc[PATHLEN];                     //fifo to input in irclsd
	wchar_t nick[NICKLEN + 1];               //nick (wide char)
	wchar_t server[SERVERLEN + 1];           //server (wide char)
} ArgParse;


/* Creates an argparse object
 * argc: argument count coming from main()
 * argv: arguments coming from main()
 * Returns: ArgParse pointer filled with data
 */
ArgParse *
argparse_new(int argc, char *argv[]);

/* 
 * Frees the memory of an argparse object
 */
void
argparse_del(ArgParse *argparse);


#endif /*ARGPARSE_H*/
