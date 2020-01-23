// https://tools.ietf.org/html/rfc1459  -- Original specification
// https://tools.ietf.org/html/rfc2810  -- Architecture specfication
// https://tools.ietf.org/html/rfc2811  -- Channel specification
// https://tools.ietf.org/html/rfc2812  -- Client specification
// https://tools.ietf.org/html/rfc2813  -- Server specification
//
// <message>  ::= [':' <prefix> <SPACE> ] <command> <params> <crlf>
// <prefix>   ::= <servername> | <nick> [ '!' <user> ] [ '@' <host> ]
// <command>  ::= <letter> { <letter> } | <number> <number> <number>
// <SPACE>    ::= ' ' { ' ' }
// <params>   ::= <SPACE> [ ':' <trailing> | <middle> <params> ]
//
// <middle>   ::= <Any *non-empty* sequence of octets not including SPACE
//                or NUL or CR or LF, the first of which may not be ':'>
// <trailing> ::= <Any, possibly *empty*, sequence of octets not including
//                  NUL or CR or LF>
//
// <crlf>     ::= CR LF
#ifndef IRCPROTO_H
#define IRCPROTO_H


#include <global/global.h> //WBUFLEN, etc.

#include <wchar.h> //wchar_t


#define TIMELEN 19
#define CMDLEN 15
#define MARKPOSITION 20
#define IRC_MSG_POSITION 22


enum IrcResEnum {IRC_RES_UNKNOWN = 0, IRC_RES_PRIVMSG, IRC_RES_MYPRIVMSG,
                 IRC_RES_ERROR};


typedef struct IrcProto
{
	wchar_t t[TIMELEN + 1];
	wchar_t tshort[TIMELEN + 1];
	wchar_t mark;
	wchar_t nick[NICKLEN + 1];
	wchar_t cmd[CMDLEN + 1];
	wchar_t tonick[CHANLEN + 1];
	wchar_t params[WBUFLEN + 1];
} IrcProto;


IrcProto *
ircproto_new(void);

void
ircproto_del(IrcProto *ircproto);


int
ircproto_parse(IrcProto *ircproto, const char buf[CBUFLEN + 1]);

static void
ircproto_reinit(IrcProto *ircproto);

static int
ircproto_parse_cmd(IrcProto *ircproto, wchar_t wbuf[WBUFLEN + 1]);

static void
ircproto_parse_unknown(IrcProto *ircproto, wchar_t wbuf[WBUFLEN + 1]);

static void
ircproto_parse_privmsg(IrcProto *ircproto, wchar_t wbuf[WBUFLEN + 1]);

static void
ircproto_parse_time(IrcProto *ircproto, wchar_t wbuf[WBUFLEN + 1]);

static void
ircproto_parse_mark(IrcProto *ircproto, wchar_t wbuf[WBUFLEN + 1]);

static int
ircproto_parse_nick(IrcProto *ircproto, wchar_t wbuf[WBUFLEN + 1]);

static int
ircproto_parse_tonick(IrcProto *ircproto, wchar_t wbuf[WBUFLEN + 1]);

static int
ircproto_parse_params(IrcProto *ircproto, wchar_t wbuf[WBUFLEN + 1]);

static int
ircproto_parse_myprivmsg(IrcProto *ircproto, wchar_t wbuf[WBUFLEN + 1]);


#endif /*IRCPROTO_H*/
