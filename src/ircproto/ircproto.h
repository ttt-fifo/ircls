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
#ifndef IRCPROTO_H
#define IRCPROTO_H


#include <global/global.h>                       //WBUFLEN, etc.

#include <wchar.h>                               //wchar_t


#define TIMELEN 19                               //length of the time string
#define CMDLEN 15                                //length of the command string
#define MARKPOSITION 20                          //the position of irclsd mark
                                                 //this is not IRC, irclsd
                                                 //marks traffic as > < or !
#define IRC_MSG_POSITION 22                      //where IRC message starts


/* results to return from functions:
 * UNKNOWN: some unparsed irc message
 * PRIVMSG: parsed privmsg
 * MYPRIVMSG: parsed privmsg sent from me
 * ERROR: parsed error (non - IRC protocol: disconnected, memory err, etc.)
 */
enum IrcResEnum {IRC_RES_UNKNOWN = 0, IRC_RES_PRIVMSG, IRC_RES_MYPRIVMSG,
                 IRC_RES_ERROR};


/* data structure for parsing IRC protocol into*/
typedef struct IrcProto
{
	wchar_t t[TIMELEN + 1];                  //time of event
	wchar_t tshort[TIMELEN + 1];             //time - short presentation
	wchar_t mark;                            //irclsd makr <>!, etc
	wchar_t nick[NICKLEN + 1];               //sending nick
	wchar_t cmd[CMDLEN + 1];                 //IRC command
	wchar_t tonick[CHANLEN + 1];             //receiving nick or channel
	wchar_t params[WBUFLEN + 1];             //IRC params (see above)
} IrcProto;


/*
 * Constructor
 * Returns: IrcProto datatype pointer
 */
IrcProto *
ircproto_new(void);

/*
 * Destructor
 * Frees memory
 * ircproto: IrcProto pointer to free
 */
void
ircproto_del(IrcProto *ircproto);

/*
 * Parsing one IRC protocol line into IrcProto datatype variable
 * ircproto: the data to parse into
 * buf: character buffer with IRC line
 * Returns: one of the IrcResEnum values
 */
int
ircproto_parse(IrcProto *ircproto, const char buf[CBUFLEN + 1]);

/* 
 * Re-initializes IrcProto variable
 * ircproto: variable to reinit
 */
static void
ircproto_reinit(IrcProto *ircproto);

/*
 * Parses the IRC command and saves it in ircproto.cmd
 * ircproto: data structure to save to
 * wbuf: wide char IRC line
 * Returns: 0 Err, 1 OK
 */
static int
ircproto_parse_cmd(IrcProto *ircproto, wchar_t wbuf[WBUFLEN + 1]);

/*
 * Parses an unknown IRC message
 * (because we do not have full IRC implementation, we consider some commands
 *  as 'unknown type')
 *  Saves the whole IRC message to ircproto.param
 *  ircproto: where to save data to
 *  wbuf: wide character IRC line
 */
static void
ircproto_parse_unknown(IrcProto *ircproto, wchar_t wbuf[WBUFLEN + 1]);

/*
 * Parses the privmsg IRC message
 * Saves ircproto.t, ircproto.tshort, ircproto.nick, ircproto.tonick,
 * ircproto.cmd, ircproto.params.
 * ircproto: where to parse to
 * wbuf: wide character buffer for the IRC line
 */
static void
ircproto_parse_privmsg(IrcProto *ircproto, wchar_t wbuf[WBUFLEN + 1]);

/*
 * Parses the time into ircproto.t and ircproto.tshort
 * ircproto: where to save data to
 * wbuf: wide char IRC line buffer
 */
static void
ircproto_parse_time(IrcProto *ircproto, wchar_t wbuf[WBUFLEN + 1]);

/* Parses the mark <>! comming from irclsd
 * Saves it into ircproto.mark
 * ircproto: where to save data to
 * wbuf: wide char IRC line buffer
 */
static void
ircproto_parse_mark(IrcProto *ircproto, wchar_t wbuf[WBUFLEN + 1]);

/*
 * Parses nick into ircproto.nick
 * ircproto: where to save data to
 * wbuf: wide char IRC line buffer
 * Returns: 0 Err, 1 OK
 */
static int
ircproto_parse_nick(IrcProto *ircproto, wchar_t wbuf[WBUFLEN + 1]);

/*
 * Parses tonick into ircproto.tonick
 * ircproto: where to save data to
 * wbuf: wide char IRC line buffer
 * Returns: 0 Err, 1 OK
 */
static int
ircproto_parse_tonick(IrcProto *ircproto, wchar_t wbuf[WBUFLEN + 1]);

/*
 * Parses params into ircproto.params
 * ircproto: where to save data to
 * wbuf: wide char IRC line buffer
 * Returns: 0 Err, 1 OK
 */
static int
ircproto_parse_params(IrcProto *ircproto, wchar_t wbuf[WBUFLEN + 1]);

/*
 * Parses IRC privmsg sent from me
 * ircproto: data to save to
 * wbuf: wide char IRC line buffer
 * Returns: 0 Err, 1 OK
 */
static int
ircproto_parse_myprivmsg(IrcProto *ircproto, wchar_t wbuf[WBUFLEN + 1]);


#endif /*IRCPROTO_H*/
