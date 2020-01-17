/* 
 * Parsing arguments for ircls - the code
 */      
#include <argparse/argparse.h>                   //some prototypes
#include <string.h>                              //strcmp()
#include <stdlib.h>                              //free()
#include <stdio.h>                               //printf()


/* Creates an argparse object
 * argc: argument count coming from main()
 * argv: arguments coming from main()
 * Returns: ArgParse pointer filled with data
 */
ArgParse *
argparse_new(int argc, char *argv[])
{
	int key = ARG_UNKNOWN;                   //the key to distinguish args
	char cserver[SERVERLEN*4 + 1];           //server, nick char values 
	char cnick[NICKLEN*4 + 1];               //they will become wchar_t

	/*reserve memory for the object*/
	ArgParse *argparse = (ArgParse *)malloc(sizeof(ArgParse));
	if(argparse == NULL)
	{
		printf("*ERROR 201* cannot parse arguments");
		exit(1);
	}

	/*initialize to empty strings*/
	argparse->fromirc[0] = '\0';
	argparse->toirc[0] = '\0';
	argparse->server[0] = L'\0';             //NOTE: server, nick are wide
	argparse->nick[0] = L'\0';               //character strings

	/*parsing the argv into argparse struct*/
	for(int i=0; i < argc; i++)
	{
		/*identify if current argv is a key*/
		if(strcmp(argv[i], "-fromirc") == 0) key = ARG_FROMIRC;
		else if(strcmp(argv[i], "-toirc") == 0) key = ARG_TOIRC;
		else if(strcmp(argv[i], "-server") == 0) key = ARG_SERVER;
		else if(strcmp(argv[i], "-nick") == 0) key = ARG_NICK;
		/*else it is a value*/
		else
		{
			switch(key)
			{
				case ARG_FROMIRC:
					strncpy(argparse->fromirc,
						argv[i], PATHLEN);
					argparse->fromirc[PATHLEN] = '\0';
					break;
				case ARG_TOIRC:
					strncpy(argparse->toirc,
						argv[i], PATHLEN);
					argparse->toirc[PATHLEN] = '\0';
					break;
				case ARG_SERVER: //multibyte char to wchar_t
					strncpy(cserver,
						argv[i], SERVERLEN*4);
					cserver[SERVERLEN*4] = '\0';
					mbstowcs(argparse->server,
						 cserver,
						 SERVERLEN);
					argparse->server[SERVERLEN] = L'\0';
					break;
				case ARG_NICK:   //multibyte to wchar
					strncpy(cnick,
						argv[i], NICKLEN*4);
					cnick[NICKLEN*4] = '\0';
					mbstowcs(argparse->nick,
						 cnick,
						 NICKLEN);
					argparse->nick[NICKLEN] = L'\0';
					break;
			} /*switch(key)*/

			key = ARG_UNKNOWN;       //this is not key, not value
		} /*else*/
	} /*for(i....)*/

	/*check if all keys are filled with values*/
	if(argparse->fromirc[0] == '\0')
	{
		printf("*ERROR 202* file with data from irc not given\n");
		exit(2);
	}
	else if(argparse->toirc[0] == '\0')
	{
		printf("*ERROR 203* fifo for data to irc not given\n");
		exit(3);
	}
	else if(argparse->server[0] == L'\0')
	{
		printf("*ERROR 204* server name not given\n");
		exit(4);

	}
	else if(argparse->nick[0] == L'\0')
	{
		printf("*ERROR 205* nick name not given\n");
		exit(5);

	}

	/*and return object (never forget this, it will be a mess :)*/
	return argparse;
} /*argparse_new()*/


/* 
 * Frees the memory of an argparse object
 */
void
argparse_del(ArgParse *argparse)
{
	if(argparse != NULL) free(argparse);
} /*argparse_del()*/
