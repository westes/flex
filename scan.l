/* scan.l - scanner for flex input */

/*
 * Copyright (c) 1987, the University of California
 * 
 * The United States Government has rights in this work pursuant to
 * contract no. DE-AC03-76SF00098 between the United States Department of
 * Energy and the University of California.
 * 
 * This program may be redistributed.  Enhancements and derivative works
 * may be created provided the new works, if made available to the general
 * public, are made available for use by anyone.
 */

%{
#include "flexdef.h"
#include "parse.h"

#define ACTION_ECHO fprintf( temp_action_file, "%s", yytext )
#define MARK_END_OF_PROLOG fprintf( temp_action_file, "%%%% end of prolog\n" );

#undef YY_DECL
#define YY_DECL \
	int flexscan()

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
%x FIRSTCCL CCL ACTION RECOVER BRACEERROR C_COMMENT C_COMMENT_2 ACTION_COMMENT
%x ACTION_STRING PERCENT_BRACE_ACTION

WS		[ \t]+

OPTWS		[ \t]*

NAME		[a-z_][a-z_0-9]*

SCNAME		{NAME}

ESCSEQ		\\([^^\n]|"^".|0[0-9]{1,3})

%%
    static int bracelevel, didadef;
    int i, cclval;
    char nmdef[MAXLINE], myesc();

^{WS}.*\n		++linenum; ECHO; /* indented code */
^#.*\n			++linenum; ECHO; /* treat as a comment */
^"/*"			ECHO; BEGIN(C_COMMENT);
^"%s"(tart)?		return ( SCDECL );
^"%x"			return ( XSCDECL );
^"%{".*\n		++linenum; line_directive_out( stdout ); BEGIN(CODEBLOCK);
{WS}			return ( WHITESPACE );

^"%%".*			{
			sectnum = 2;
			line_directive_out( stdout );
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

<RECOVER>.*\n		++linenum; BEGIN(0); RETURNNAME;


<SECT2PROLOG>.*\n/[^ \t\n]	{
			++linenum;
			ACTION_ECHO;
			MARK_END_OF_PROLOG;
			BEGIN(SECT2);
			}

<SECT2PROLOG>.*\n	++linenum; ACTION_ECHO;

<SECT2>^{OPTWS}\n	++linenum; /* allow blank lines in section 2 */

	/* this horrible mess of a rule matches indented lines which
	 * do not contain "/*".  We need to make the distinction because
	 * otherwise this rule will be taken instead of the rule which
	 * matches the beginning of comments like this one
	 */
<SECT2>^{WS}([^/\n]|"/"[^*\n])*("/"?)\n	{
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

<SECT2>^{OPTWS}"/*"	ACTION_ECHO; BEGIN(C_COMMENT_2);

<SECT2>{WS}		{ /* needs to be separate from following rule due to
			   * bug with trailing context
			   */
			bracelevel = 0;
			BEGIN(ACTION);
			return ( '\n' );
			}

<SECT2>{OPTWS}/\n	{
			bracelevel = 0;
			BEGIN(ACTION);
			return ( '\n' );
			}

<SECT2>^{OPTWS}\n	++linenum; return ( '\n' );

<SECT2>^"%%".*		{
			/* guarantee that the SECT3 rule will have something
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
<PERCENT_BRACE_ACTION>.*		ACTION_ECHO;
<PERCENT_BRACE_ACTION>\n		{
			++linenum;
			ACTION_ECHO;
			if ( bracelevel == 0 )
			    {
			    fputs( "\tYY_BREAK\n", temp_action_file );
			    BEGIN(SECT2);
			    }
			}

<ACTION>"{"		ACTION_ECHO; ++bracelevel;
<ACTION>"}"		ACTION_ECHO; --bracelevel;
<ACTION>[^{}"'/\n]+	ACTION_ECHO;
<ACTION>"/*"		ACTION_ECHO; BEGIN(ACTION_COMMENT);
<ACTION>"'"([^'\\\n]|\\.)*"'"	ACTION_ECHO; /* character constant */
<ACTION>\"		ACTION_ECHO; BEGIN(ACTION_STRING);
<ACTION>\n		{
			++linenum;
			ACTION_ECHO;
			if ( bracelevel == 0 )
			    {
			    fputs( "\tYY_BREAK\n", temp_action_file );
			    BEGIN(SECT2);
			    }
			}
<ACTION>.		ACTION_ECHO;

<ACTION_COMMENT>"*/"	ACTION_ECHO; BEGIN(ACTION);
<ACTION_COMMENT>[^*\n]+	ACTION_ECHO;
<ACTION_COMMENT>"*"	ACTION_ECHO;
<ACTION_COMMENT>\n	++linenum; ACTION_ECHO;
<ACTION_COMMENT>.	ACTION_ECHO;

<C_COMMENT_2>"*/"	ACTION_ECHO; BEGIN(SECT2);
<C_COMMENT_2>"*/".*\n	++linenum; ACTION_ECHO; BEGIN(SECT2);
<C_COMMENT_2>[^*\n]+	ACTION_ECHO;
<C_COMMENT_2>"*"	ACTION_ECHO;
<C_COMMENT_2>\n		++linenum; ACTION_ECHO;

<ACTION_STRING>[^"\\\n]+	ACTION_ECHO;
<ACTION_STRING>\\.	ACTION_ECHO;
<ACTION_STRING>\n	++linenum; ACTION_ECHO;
<ACTION_STRING>\"	ACTION_ECHO; BEGIN(ACTION);
<ACTION_STRING>.	ACTION_ECHO;


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

			/* black magic - we know the names of a flex scanner's
			 * internal variables.  We cap the input buffer with
			 * an end-of-string and dump it to the output.
			 */
			YY_DO_BEFORE_SCAN; /* recover from setting up yytext */

#ifdef FLEX_FAST_SKEL
			fputs( yy_c_buf_p + 1, stdout );
#else
			yy_ch_buf[yy_e_buf_p + 1] = '\0';

			/* ignore the first character; it's the second '%'
			 * put back by the yyless(1) above
			 */
			fputs( yy_ch_buf + yy_c_buf_p + 1, stdout );
#endif

			/* if we don't do this, the data written by write()
			 * can get overwritten when stdout is finally flushed
			 */
			(void) fflush( stdout );

			while ( (numchars = read( fileno(yyin), yy_ch_buf,
						  YY_BUF_MAX )) > 0 )
			    (void) write( fileno(stdout), yy_ch_buf, numchars );
	
			if ( numchars < 0 )
			    flexerror( "fatal read error in section 3" );

			return ( EOF );
			}
%%
