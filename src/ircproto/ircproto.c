#include <ircproto/ircproto.h>

#include <stdlib.h> //mbstowcs()
#include <wchar.h> //wcsncpy()


extern wchar_t mynick[NICKLEN + 1];


IrcProto *
ircproto_new(void)
{
	IrcProto *ircproto = (IrcProto *)malloc(sizeof(IrcProto));
	return ircproto;
} /*ircproto_new()*/


void
ircproto_del(IrcProto *ircproto)
{
	if(ircproto != NULL) free(ircproto);
} /*ircproto_del()*/


int
ircproto_parse(IrcProto *ircproto, const char buf[CBUFLEN + 1])
{
	wchar_t wbuf[WBUFLEN + 1];
	wchar_t *pwbuf;

	ircproto_reinit(ircproto);

	mbstowcs(wbuf, buf, WBUFLEN);
	wbuf[WBUFLEN] = L'\0';

        ircproto_parse_mark(ircproto, wbuf);

	if(ircproto->mark == L'!')
	{
		ircproto_parse_unknown(ircproto, wbuf);
		return IRC_RES_ERROR;
	}

	if(ircproto->mark == L'>')
	{
		pwbuf = wbuf + IRC_MSG_POSITION;
		if(wcsncmp(pwbuf, L"PRIVMSG", 7) == 0)
		{
			ircproto_parse_myprivmsg(ircproto, wbuf);
			return IRC_RES_MYPRIVMSG;
		}
	}

	if(wbuf[IRC_MSG_POSITION] != L':')
	{
		ircproto_parse_unknown(ircproto, wbuf);
		return IRC_RES_UNKNOWN;
	}

	if(!ircproto_parse_cmd(ircproto, wbuf))
	{
		ircproto_parse_unknown(ircproto, wbuf);
		return IRC_RES_UNKNOWN;
	}

	if(wcsncmp(ircproto->cmd, L"PRIVMSG", 7) != 0)
	{
		ircproto_parse_unknown(ircproto, wbuf);
		return IRC_RES_UNKNOWN;
	}

	ircproto_parse_privmsg(ircproto, wbuf);
	return IRC_RES_PRIVMSG;
} /*ircproto_parse()*/


static void
ircproto_reinit(IrcProto *ircproto)
{
	ircproto->t[0] = L'\0';
	ircproto->tshort[0] = L'\0';
	ircproto->mark = L'\0';
	ircproto->nick[0] = L'\0';
	ircproto->cmd[0] = L'\0';
	ircproto->tonick[0] = L'\0';
	ircproto->params[0] = L'\0';
} /*ircproto_reinit()*/


static int
ircproto_parse_cmd(IrcProto *ircproto, wchar_t wbuf[WBUFLEN + 1])
{
	wchar_t *pwbuf;
	wchar_t *pcmd;
	int i;

	pwbuf = wbuf + IRC_MSG_POSITION;

	while(*pwbuf != L' ') if(*pwbuf++ == L'\0') return 0;
	while(*pwbuf == L' ') if(*pwbuf++ == L'\0') return 0;

	pcmd = ircproto->cmd;
	i = 0;
	while(i < CMDLEN)
	{
		if(*pwbuf == L'\0') return 0;
		if(*pwbuf == L' ') break;
		*pcmd++ = *pwbuf++;
		i++;
	}
	*pcmd = L'\0';

	return 1; //OK
} /*ircproto_parse_cmd()*/


static void
ircproto_parse_unknown(IrcProto *ircproto, wchar_t wbuf[WBUFLEN + 1])
{
	wchar_t *pwbuf;

	ircproto_parse_time(ircproto, wbuf);

	pwbuf = wbuf + IRC_MSG_POSITION;
	wcsncpy(ircproto->params, pwbuf, WBUFLEN);
	ircproto->params[WBUFLEN] = L'\0';
} /*ircproto_parse_unknown()*/


static void
ircproto_parse_privmsg(IrcProto *ircproto, wchar_t wbuf[WBUFLEN + 1])
{
	ircproto_parse_time(ircproto, wbuf);
	ircproto_parse_nick(ircproto, wbuf);
	/*ircproto_parse_cmd(...) - unneeded, already parsed */
	ircproto_parse_tonick(ircproto, wbuf);
	ircproto_parse_params(ircproto, wbuf);
} /*ircproto_parse_privmsg()*/


static void
ircproto_parse_time(IrcProto *ircproto, wchar_t wbuf[WBUFLEN + 1])
{
	int i;
	int j;

	wcsncpy(ircproto->t, wbuf, TIMELEN);
	ircproto->t[TIMELEN] = L'\0';

	for(i = 11, j = 0; i <= 15; i++, j++)
	{
		ircproto->tshort[j] = ircproto->t[i];
	}
	ircproto->tshort[j] = L'\0';
} /*ircproto_parse_time()*/


static void
ircproto_parse_mark(IrcProto *ircproto, wchar_t wbuf[WBUFLEN + 1])
{
	ircproto->mark = wbuf[MARKPOSITION];
} /*ircproto_parse_mark()*/


static int
ircproto_parse_nick(IrcProto *ircproto, wchar_t wbuf[WBUFLEN + 1])
{
	wchar_t *pwbuf;
	wchar_t *pnick;
	int i;

	pwbuf = wbuf + IRC_MSG_POSITION;
	pwbuf++;

	pnick = ircproto->nick;
	i = 0;
	while(i < NICKLEN)
	{
		if(*pwbuf == L'!') break;
		if(*pwbuf == L'\0') return 0;
		*pnick++ = *pwbuf++;
		i++;
	}
	*pnick = L'\0';

	return 1; //OK
} /*ircproto_parse_nick()*/


static int
ircproto_parse_tonick(IrcProto *ircproto, wchar_t wbuf[WBUFLEN + 1])
{
	wchar_t *pwbuf;
	wchar_t *ptonick;
	int i;

	pwbuf = wbuf + IRC_MSG_POSITION;

	while(*pwbuf != L' ') if(*pwbuf++ == L'\0') return 0; //rewind nick
	while(*pwbuf == L' ') if(*pwbuf++ == L'\0') return 0;

	while(*pwbuf != L' ') if(*pwbuf++ == L'\0') return 0; //rewind cmd
	while(*pwbuf == L' ') if(*pwbuf++ == L'\0') return 0;

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
	return 1; //OK
} /*ircproto_parse_tonick()*/


static int
ircproto_parse_params(IrcProto *ircproto, wchar_t wbuf[WBUFLEN + 1])
{
	wchar_t *pwbuf;
	wchar_t *pparams;
	int i;

	pwbuf = wbuf + IRC_MSG_POSITION;

	while(*pwbuf != L' ') if(*pwbuf++ == L'\0') return 0; //rewind nick
	while(*pwbuf == L' ') if(*pwbuf++ == L'\0') return 0;

	while(*pwbuf != L' ') if(*pwbuf++ == L'\0') return 0; //rewind cmd
	while(*pwbuf == L' ') if(*pwbuf++ == L'\0') return 0;

	while(*pwbuf != L' ') if(*pwbuf++ == L'\0') return 0; //rewind tonick
	while(*pwbuf == L' ') if(*pwbuf++ == L'\0') return 0;
	/*todo rewind :*/

	/*todo: this to be refactored with wcsncpy()*/
        pparams = ircproto->params;
	i = 0;
	while(i < WBUFLEN)
	{
		if(*pwbuf == L'\0') break;
		*pparams++ = *pwbuf++;
		i++;
	}
	*pparams = L'\0';

	return 1; //OK
} /*ircproto_parse_params()*/


static int
ircproto_parse_myprivmsg(IrcProto *ircproto, wchar_t wbuf[WBUFLEN + 1])
{
	wchar_t *pwbuf;
	wchar_t *ptonick;
	int i;

	ircproto_parse_time(ircproto, wbuf);

	wcsncpy(ircproto->cmd, L"PRIVMSG", 8); //copies '\0' termination

	wcsncpy(ircproto->nick, mynick, NICKLEN);
	ircproto->nick[NICKLEN] = L'\0';

	pwbuf = wbuf + IRC_MSG_POSITION;

	while(*pwbuf != L' ') if(*pwbuf++ == L'\0') return 0; //rewind cmd
	while(*pwbuf == L' ') if(*pwbuf++ == L'\0') return 0;

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

	wcsncpy(ircproto->params, pwbuf, WBUFLEN);
	ircproto->params[WBUFLEN] = L'\0';

	return 1; //OK
} /*ircproto_parse_myprivmsg()*/
