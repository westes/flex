/* flexscan.l - scanner for flex input */

/*
 * Copyright (c) University of California, 1987
 */

%{
#include "flexdef.h"
#include "strings.h"
#include "y.tab.h"

#undef YYDECL
#define YYDECL \
	int lexscan()

#undef yywrap
#define yywrap(result) \
	{ \
	if ( ! did_second_skelout ) \
		skelout(); \
	result = 1; \
	}

#define RETURNCHAR \
	yylval = yytext[0]; \
	return ( CHAR );

#define RETURNNAME \
	(void) strcpy( nmstr, yytext ); \
	return ( NAME );

#define PUT_BACK_STRING(str, start) \
	for ( i = strlen( str ) - 1; i >= start; --i ) \
	    unput(str[i])
%}

%x SECT2 SECT2PROLOG SECT3 CODEBLOCK PICKUPDEF SC CARETISBOL NUM QUOTE
%x FIRSTCCL CCL ACTION RECOVER BRACEERROR C_COMMENT ACTION_COMMENT
%x ACTION_STRING PERCENT_BRACE_ACTION

WS		[ \t]+

OPTWS		[ \t]*

NAME		[a-z_][a-z_0-9]*

SCNAME		{NAME}

ESCSEQ		\\([^^\n]|"^".|0[0-9]{1,3})

%%
    static int bracelevel;
    int i, cclval;
    char nmdef[MAXLINE], myesc();
    static int didadef, did_second_skelout = false;

^{WS}.*\n		++linenum; ECHO; /* indented code */
^#.*\n			++linenum; ECHO; /* either a Ratfor comment or a CPP directive */
^"/*"			ECHO; BEGIN(C_COMMENT);
^"%s"(tart)?		return ( SCDECL );
^"%x"			return ( XSCDECL );
^"%{".*\n		++linenum; line_directive_out(); BEGIN(CODEBLOCK);
{WS}			return ( WHITESPACE );

^"%%".*			{
			sectnum = 2;
			skelout();
			line_directive_out();
			BEGIN(SECT2PROLOG);
			return ( SECTEND );
			}

^"%"[^sx{%].*\n		{
			fprintf( stderr,
			     "old-style lex command at line %d ignored:\n\t%s",
				 linenum, yytext );
			++linenum;
			}

^{NAME}			{
			(void) strcpy( nmstr, yytext );
			didadef = false;
			BEGIN(PICKUPDEF);
			}

{SCNAME}		RETURNNAME;
^{OPTWS}\n		++linenum; /* allows blank lines in section 1 */
\n			++linenum; return ( '\n' );
.			synerr( "illegal character" ); BEGIN(RECOVER);


<C_COMMENT>"*/"		ECHO; BEGIN(0);
<C_COMMENT>"*/".*\n	++linenum; ECHO; BEGIN(0);
<C_COMMENT>[^*\n]+	ECHO;
<C_COMMENT>"*"		ECHO;
<C_COMMENT>\n		++linenum; ECHO;

<CODEBLOCK>^"%}".*\n	++linenum; BEGIN(0);
<CODEBLOCK>.*\n		++linenum; ECHO;

<PICKUPDEF>{WS}		/* separates name and definition */

<PICKUPDEF>[^ \t\n].*	{
			(void) strcpy( nmdef, yytext );

			for ( i = strlen( nmdef ) - 1;
			      i >= 0 &&
			      nmdef[i] == ' ' || nmdef[i] == '\t';
			      --i )
			    ;

			nmdef[i + 1] = '\0';

                        ndinstal( nmstr, nmdef );
			didadef = true;
			}

<PICKUPDEF>\n		{
			if ( ! didadef )
			    synerr( "incomplete name definition" );
			BEGIN(0);
			++linenum;
			}

<RECOVER>.*\n		++linenum; RETURNNAME;


<SECT2PROLOG>.*\n/[^ \t\n]	{
			++linenum;
			ECHO;
			skelout();
			did_second_skelout = true;
			BEGIN(SECT2);
			}

<SECT2PROLOG>.*\n	++linenum; ECHO;

<SECT2>^{OPTWS}\n	++linenum; /* allow blank lines in section 2 */
<SECT2>^{WS}.*\n	{
			synerr( "indented code found outside of action" );
			++linenum;
			}
<SECT2>"<"		BEGIN(SC); return ( '<' );
<SECT2>^"^"		return ( '^' );
<SECT2>\"		BEGIN(QUOTE); return ( '"' );
<SECT2>"{"/[0-9]		BEGIN(NUM); return ( '{' );
<SECT2>"{"[^0-9\n][^}\n]*	BEGIN(BRACEERROR);
<SECT2>"$"/[ \t\n]	return ( '$' );

<SECT2>{WS}"%{"		{
			bracelevel = 1;
			BEGIN(PERCENT_BRACE_ACTION);
			return ( '\n' );
			}
<SECT2>{WS}"|".*\n	++linenum; return ( '\n' );

<SECT2>{WS}	|
<SECT2>{OPTWS}/\n	{
			bracelevel = 0;
			BEGIN(ACTION);
			return ( '\n' );
			}

<SECT2>^{OPTWS}\n	++linenum; return ( '\n' );

<SECT2>^"%%".*		{
			/* guarentee that the SECT3 rule will have something
			 * to match
			 */
			yyless(1);
			sectnum = 3;
			BEGIN(SECT3);
			return ( EOF ); /* to stop the parser */
			}

<SECT2>"["([^\\\]\n]|{ESCSEQ})+"]"	{
			(void) strcpy( nmstr, yytext );

			/* check to see if we've already encountered this ccl */
			if ( (cclval = ccllookup( nmstr )) )
			    {
			    yylval = cclval;
			    ++cclreuse;
			    return ( PREVCCL );
			    }
			else
			    {
			    /* we fudge a bit.  We know that this ccl will
			     * soon be numbered as lastccl + 1 by cclinit
			     */
			    cclinstal( nmstr, lastccl + 1 );

			    /* push back everything but the leading bracket
			     * so the ccl can be rescanned
			     */
			    PUT_BACK_STRING(nmstr, 1);

			    BEGIN(FIRSTCCL);
			    return ( '[' );
			    }
			}

<SECT2>"{"{NAME}"}"	{
			register char *nmdefptr;
			char *ndlookup();

			(void) strcpy( nmstr, yytext );
			nmstr[yyleng - 1] = '\0';  /* chop trailing brace */

			/* lookup from "nmstr + 1" to chop leading brace */
			if ( ! (nmdefptr = ndlookup( nmstr + 1 )) )
			    synerr( "undefined {name}" );

			else
			    { /* push back name surrounded by ()'s */
			    unput(')');
			    PUT_BACK_STRING(nmdefptr, 0);
			    unput('(');
			    }
			}

<SECT2>[/|*+?.()]	return ( yytext[0] );
<SECT2>.		RETURNCHAR;
<SECT2>\n		++linenum; return ( '\n' );


<SC>","			return ( ',' );
<SC>">"			BEGIN(SECT2); return ( '>' );
<SC>">"/"^"		BEGIN(CARETISBOL); return ( '>' );
<SC>{SCNAME}		RETURNNAME;
<SC>.			synerr( "bad start condition name" );

<CARETISBOL>"^"		BEGIN(SECT2); return ( '^' );


<QUOTE>[^"\n]		RETURNCHAR;
<QUOTE>\"		BEGIN(SECT2); return ( '"' );

<QUOTE>\n		{
			synerr( "missing quote" );
			BEGIN(SECT2);
			++linenum;
			return ( '"' );
			}


<FIRSTCCL>"^"/[^-\n]	BEGIN(CCL); return ( '^' );
<FIRSTCCL>"^"/-		return ( '^' );
<FIRSTCCL>-		BEGIN(CCL); yylval = '-'; return ( CHAR );
<FIRSTCCL>.		BEGIN(CCL); RETURNCHAR;

<CCL>-/[^\]\n]		return ( '-' );
<CCL>[^\]\n]		RETURNCHAR;
<CCL>"]"			BEGIN(SECT2); return ( ']' );


<NUM>[0-9]+		{
			yylval = myctoi( yytext );
			return ( NUMBER );
			}

<NUM>","			return ( ',' );
<NUM>"}"			BEGIN(SECT2); return ( '}' );

<NUM>.			{
			synerr( "bad character inside {}'s" );
			BEGIN(SECT2);
			return ( '}' );
			}

<NUM>\n			{
			synerr( "missing }" );
			BEGIN(SECT2);
			++linenum;
			return ( '}' );
			}


<BRACEERROR>"}"		synerr( "bad name in {}'s" ); BEGIN(SECT2);
<BRACEERROR>\n		synerr( "missing }" ); ++linenum; BEGIN(SECT2);


<PERCENT_BRACE_ACTION>{OPTWS}"%}".*	bracelevel = 0;
<PERCENT_BRACE_ACTION>.*		ECHO;
<PERCENT_BRACE_ACTION>\n		{
			++linenum;
			ECHO;
			if ( bracelevel == 0 )
			    {
			    if ( genftl )
				puts( "\tbreak;" );
			    BEGIN(SECT2);
			    }
			}

<ACTION>"{"		ECHO; ++bracelevel;
<ACTION>"}"		ECHO; --bracelevel;
<ACTION>[^{}"'/\n]+	ECHO;
<ACTION>"/*"		ECHO; BEGIN(ACTION_COMMENT);
<ACTION>"'"([^'\\\n]|\\.)*"'"	ECHO; /* character constant */
<ACTION>\"		ECHO; BEGIN(ACTION_STRING);
<ACTION>\n		{
			++linenum;
			ECHO;
			if ( bracelevel == 0 )
			    {
			    if ( genftl )
				puts( "\tbreak;" );
			    BEGIN(SECT2);
			    }
			}
<ACTION>.		ECHO;

<ACTION_COMMENT>"*/"	ECHO; BEGIN(ACTION);
<ACTION_COMMENT>[^*\n]+	ECHO;
<ACTION_COMMENT>"*"	ECHO;
<ACTION_COMMENT>\n	++linenum; ECHO;
<ACTION_COMMENT>.	ECHO;

<ACTION_STRING>[^"\\\n]+	ECHO;
<ACTION_STRING>\\.	ECHO;
<ACTION_STRING>\n	++linenum; ECHO;
<ACTION_STRING>\"	ECHO; BEGIN(ACTION);
<ACTION_STRING>.	ECHO;


<SECT2,QUOTE,CCL>{ESCSEQ}	{
			yylval = myesc( yytext );
			return ( CHAR );
			}

<FIRSTCCL>{ESCSEQ}	{
			yylval = myesc( yytext );
			BEGIN(CCL);
			return ( CHAR );
			}


<SECT3>.|\n		{
			register int numchars;

			/* black magic - we know the names of a lex scanner's
			 * internal variables.  We cap the input buffer with
			 * an end-of-string and dump it to the output.
			 */
			YYDOBEFORESCAN;	/* recover from setting up yytext */

			yychbuf[yyebufp + 1] = '\0';

			/* ignore the first character; it's the second '%'
			 * put back by the yyless(1) above
			 */
			fputs( yychbuf + yycbufp + 1, stdout );

			/* if we don't do this, the data written by write()
			 * can get overwritten when stdout is finally flushed
			 */
			(void) fflush( stdout );

			while ( (numchars = read( fileno(yyin), yychbuf,
						  YYBUFMAX )) > 0 )
			    (void) write( fileno(stdout), yychbuf, numchars );
	
			if ( numchars < 0 )
			    lexerror( "fatal read error in section 3" );

			return ( EOF );
			}

%%
