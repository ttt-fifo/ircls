/* Simple and incomplete implementation of IRC protocol parsing
 * We implement only the parts we need from the full implementation given here:
 * ----------------------------------------------------------------------------
 * https://tools.ietf.org/html/rfc1459  -- Original specification
 * https://tools.ietf.org/html/rfc2810  -- Architecture specfication
 * https://tools.ietf.org/html/rfc2811  -- Channel specification
 * https://tools.ietf.org/html/rfc2812  -- Client specification
 * https://tools.ietf.org/html/rfc2813  -- Server specification
 * ----------------------------------------------------------------------------
 * <message>  ::= [':' <prefix> <SPACE> ] <command> <params> <crlf>
 * <prefix>   ::= <servername> | <nick> [ '!' <user> ] [ '@' <host> ]
 * <command>  ::= <letter> { <letter> } | <number> <number> <number>
 * <SPACE>    ::= ' ' { ' ' }
 * <params>   ::= <SPACE> [ ':' <trailing> | <middle> <params> ]
 * <middle>   ::= <Any *non-empty* sequence of octets not including SPACE
               or NUL or CR or LF, the first of which may not be ':'>
 * <trailing> ::= <Any, possibly *empty*, sequence of octets not including
 *                  NUL or CR or LF>
 * <crlf>     ::= CR LF
 */
#include <ircproto/ircproto.h>
#include <global/global.h>                       //mynick[]

#include <stdlib.h>                              //mbstowcs()
#include <wchar.h>                               //wcsncpy()


//coming from global.{h, c}
extern wchar_t mynick[NICKLEN + 1];


/*
 * Constructor
 * Returns: IrcProto datatype pointer
 */
IrcProto *
ircproto_new(void)
{
	IrcProto *ircproto = (IrcProto *)malloc(sizeof(IrcProto));
	return ircproto;
} /*ircproto_new()*/


/*
 * Destructor
 * Frees memory
 * ircproto: IrcProto pointer to free
 */
void
ircproto_del(IrcProto *ircproto)
{
	if(ircproto != NULL) free(ircproto);
} /*ircproto_del()*/


/*
 * Parsing one IRC protocol line into IrcProto datatype variable
 * ircproto: the data to parse into
 * buf: character buffer with IRC line
 * Returns: one of the IrcResEnum values
 */
int
ircproto_parse(IrcProto *ircproto, const char buf[CBUFLEN + 1])
{
	wchar_t wbuf[WBUFLEN + 1];               //wide character IRC line buf
	wchar_t *pwbuf;                          //helper pointer to wbuf

	ircproto_reinit(ircproto);               //zero values

	mbstowcs(wbuf, buf, WBUFLEN);            //convert to wide char
	wbuf[WBUFLEN] = L'\0';

        ircproto_parse_mark(ircproto, wbuf);     //get irclsd mark <>!

	if(ircproto->mark == L'!')               //error is parsed as unknown
	{
		ircproto_parse_unknown(ircproto, wbuf);
		return IRC_RES_ERROR;
	}

	if(ircproto->mark == L'>')               //sent from me
	{
		pwbuf = wbuf + IRC_MSG_POSITION;
		if(wcsncmp(pwbuf, L"PRIVMSG", 7) == 0) //this is my privmsg
		{
			ircproto_parse_myprivmsg(ircproto, wbuf);
			return IRC_RES_MYPRIVMSG;
		}
	}
	/*from now on it does not matter if sent from me or to me*/

	if(wbuf[IRC_MSG_POSITION] != L':')       //if IRC msg starts with :
	{                                        //parsing as unknown
		ircproto_parse_unknown(ircproto, wbuf);
		return IRC_RES_UNKNOWN;
	}

	if(!ircproto_parse_cmd(ircproto, wbuf))  //parse as cmd
	{                                        //on err parse as uknown
		ircproto_parse_unknown(ircproto, wbuf);
		return IRC_RES_UNKNOWN;
	}

	if(wcsncmp(ircproto->cmd, L"PRIVMSG", 7) != 0) //if cmd is not privmsg
	{
		ircproto_parse_unknown(ircproto, wbuf);
		return IRC_RES_UNKNOWN;
	}

	/*if reached here it is privmsg to me, parse and return*/
	ircproto_parse_privmsg(ircproto, wbuf);
	return IRC_RES_PRIVMSG;
} /*ircproto_parse()*/


/* 
 * Re-initializes IrcProto variable
 * ircproto: variable to reinit
 */
static void
ircproto_reinit(IrcProto *ircproto)
{
	/*zero all*/
	ircproto->t[0] = L'\0';
	ircproto->tshort[0] = L'\0';
	ircproto->mark = L'\0';
	ircproto->nick[0] = L'\0';
	ircproto->cmd[0] = L'\0';
	ircproto->tonick[0] = L'\0';
	ircproto->params[0] = L'\0';
} /*ircproto_reinit()*/


/*
 * Parses the IRC command and saves it in ircproto.cmd
 * ircproto: data structure to save to
 * wbuf: wide char IRC line
 * Returns: 0 Err, 1 OK
 */
static int
ircproto_parse_cmd(IrcProto *ircproto, wchar_t wbuf[WBUFLEN + 1])
{
	wchar_t *pwbuf;                          //pointer to wbuf
	wchar_t *pcmd;                           //pt to ircproto.cmd
	int i;

	pwbuf = wbuf + IRC_MSG_POSITION;

	while(*pwbuf != L' ') if(*pwbuf++ == L'\0') return 0; //rewind to
	while(*pwbuf == L' ') if(*pwbuf++ == L'\0') return 0; //cmd

	/*parse the cmd into ircproto->cmd*/
	pcmd = ircproto->cmd;
	i = 0;
	while(i < CMDLEN)
	{
		if(*pwbuf == L'\0') return 0;
		if(*pwbuf == L' ') break;        //until next space
		*pcmd++ = *pwbuf++;
		i++;
	}
	*pcmd = L'\0';

	return 1; //OK
} /*ircproto_parse_cmd()*/


/*
 * Parses an unknown IRC message
 * (because we do not have full IRC implementation, we consider some commands
 *  as 'unknown type')
 *  Saves the whole IRC message to ircproto.param
 *  ircproto: where to save data to
 *  wbuf: wide character IRC line
 */
static void
ircproto_parse_unknown(IrcProto *ircproto, wchar_t wbuf[WBUFLEN + 1])
{
	wchar_t *pwbuf;

	ircproto_parse_time(ircproto, wbuf);

	pwbuf = wbuf + IRC_MSG_POSITION;
	wcsncpy(ircproto->params, pwbuf, WBUFLEN);
	ircproto->params[WBUFLEN] = L'\0';
} /*ircproto_parse_unknown()*/


/*
 * Parses the privmsg IRC message
 * Saves ircproto.t, ircproto.tshort, ircproto.nick, ircproto.tonick,
 * ircproto.cmd, ircproto.params.
 * ircproto: where to parse to
 * wbuf: wide character buffer for the IRC line
 */
static void
ircproto_parse_privmsg(IrcProto *ircproto, wchar_t wbuf[WBUFLEN + 1])
{
	ircproto_parse_time(ircproto, wbuf);
	ircproto_parse_nick(ircproto, wbuf);
	/*ircproto_parse_cmd(...) - unneeded, already parsed */
	ircproto_parse_tonick(ircproto, wbuf);
	ircproto_parse_params(ircproto, wbuf);
} /*ircproto_parse_privmsg()*/


/*
 * Parses the time into ircproto.t and ircproto.tshort
 * ircproto: where to save data to
 * wbuf: wide char IRC line buffer
 */
static void
ircproto_parse_time(IrcProto *ircproto, wchar_t wbuf[WBUFLEN + 1])
{
	int i;
	int j;

	/*the time is the first TIMELEN simbols, copy them*/
	wcsncpy(ircproto->t, wbuf, TIMELEN);
	ircproto->t[TIMELEN] = L'\0';

	/* short time is similar to 11:00*/
	for(i = 11, j = 0; i <= 15; i++, j++)
	{
		ircproto->tshort[j] = ircproto->t[i];
	}
	ircproto->tshort[j] = L'\0';
} /*ircproto_parse_time()*/


/* Parses the mark <>! comming from irclsd
 * Saves it into ircproto.mark
 * ircproto: where to save data to
 * wbuf: wide char IRC line buffer
 */
static void
ircproto_parse_mark(IrcProto *ircproto, wchar_t wbuf[WBUFLEN + 1])
{
	ircproto->mark = wbuf[MARKPOSITION];
} /*ircproto_parse_mark()*/


/*
 * Parses nick into ircproto.nick
 * ircproto: where to save data to
 * wbuf: wide char IRC line buffer
 * Returns: 0 Err, 1 OK
 */
static int
ircproto_parse_nick(IrcProto *ircproto, wchar_t wbuf[WBUFLEN + 1])
{
	wchar_t *pwbuf;                          //ptr to wbuf
	wchar_t *pnick;                          //ptr to ircproto.nick
	int i;

	pwbuf = wbuf + IRC_MSG_POSITION;         //rewind to nick
	pwbuf++;

	pnick = ircproto->nick;
	i = 0;
	while(i < NICKLEN)
	{
		if(*pwbuf == L'\0') return 0;
		if(*pwbuf == L'!') break;        //parse nick until ! char
		*pnick++ = *pwbuf++;
		i++;
	}
	*pnick = L'\0';

	return 1; //OK
} /*ircproto_parse_nick()*/


/*
 * Parses tonick into ircproto.tonick
 * ircproto: where to save data to
 * wbuf: wide char IRC line buffer
 * Returns: 0 Err, 1 OK
 */
static int
ircproto_parse_tonick(IrcProto *ircproto, wchar_t wbuf[WBUFLEN + 1])
{
	wchar_t *pwbuf;                          //ptr to wbuf
	wchar_t *ptonick;                        //ptr to ircproto.tonick
	int i;

	pwbuf = wbuf + IRC_MSG_POSITION;

	while(*pwbuf != L' ') if(*pwbuf++ == L'\0') return 0; //rewind nick
	while(*pwbuf == L' ') if(*pwbuf++ == L'\0') return 0;

	while(*pwbuf != L' ') if(*pwbuf++ == L'\0') return 0; //rewind cmd
	while(*pwbuf == L' ') if(*pwbuf++ == L'\0') return 0;

	/*get the to nick or to channel*/
	ptonick = ircproto->tonick;
	i = 0;
	while(i < CHANLEN)
	{
		if(*pwbuf == L'\0') return 0;
		if(*pwbuf == L' ') break;        //until next empty space
		*ptonick++ = *pwbuf++;
		i++;
	}
	*ptonick = L'\0';
	return 1; //OK
} /*ircproto_parse_tonick()*/


/*
 * Parses params into ircproto.params
 * ircproto: where to save data to
 * wbuf: wide char IRC line buffer
 * Returns: 0 Err, 1 OK
 */
static int
ircproto_parse_params(IrcProto *ircproto, wchar_t wbuf[WBUFLEN + 1])
{
	wchar_t *pwbuf;                          //ptr to wbuf

	pwbuf = wbuf + IRC_MSG_POSITION;

	while(*pwbuf != L' ') if(*pwbuf++ == L'\0') return 0; //rewind nick
	while(*pwbuf == L' ') if(*pwbuf++ == L'\0') return 0;

	while(*pwbuf != L' ') if(*pwbuf++ == L'\0') return 0; //rewind cmd
	while(*pwbuf == L' ') if(*pwbuf++ == L'\0') return 0;

	while(*pwbuf != L' ') if(*pwbuf++ == L'\0') return 0; //rewind tonick
	while(*pwbuf == L' ') if(*pwbuf++ == L'\0') return 0;

	/*copy all reminding to params*/
	wcsncpy(ircproto->params, pwbuf, WBUFLEN);
	ircproto->params[WBUFLEN] = L'\0';

	return 1; //OK
} /*ircproto_parse_params()*/


/*
 * Parses IRC privmsg sent from me
 * In irclsd log file it looks like this: 2020-01-01-00:01 > PRIVMSG :somemsg
 * ircproto: data to save to
 * wbuf: wide char IRC line buffer
 * Returns: 0 Err, 1 OK
 */
static int
ircproto_parse_myprivmsg(IrcProto *ircproto, wchar_t wbuf[WBUFLEN + 1])
{
	wchar_t *pwbuf;                          //ptr to wbuf
	wchar_t *ptonick;                        //ptr to ircproto.tonick
	int i;                                   //index

	ircproto_parse_time(ircproto, wbuf);

	wcsncpy(ircproto->cmd, L"PRIVMSG", 8); //copies '\0' termination

	/*copies my nick from globals*/
	wcsncpy(ircproto->nick, mynick, NICKLEN);
	ircproto->nick[NICKLEN] = L'\0';

	pwbuf = wbuf + IRC_MSG_POSITION;

	while(*pwbuf != L' ') if(*pwbuf++ == L'\0') return 0; //rewind cmd
	while(*pwbuf == L' ') if(*pwbuf++ == L'\0') return 0;

	/*get tonick*/
	ptonick = ircproto->tonick;
	i = 0;
	while(i < CHANLEN)
	{
		if(*pwbuf == L'\0') return 0;
		if(*pwbuf == L' ') break;
		*ptonick++ = *pwbuf++;
		i++;
	}
	*ptonick = L'\0';

	while(*pwbuf == L' ') if(*pwbuf++ == L'\0') return 0; //rewind empty

	/*copy the user message in params*/
	wcsncpy(ircproto->params, pwbuf, WBUFLEN);
	ircproto->params[WBUFLEN] = L'\0';

	return 1; //OK
} /*ircproto_parse_myprivmsg()*/
