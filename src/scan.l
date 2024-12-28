/* scan.l - scanner for flex input -*-C-*- */

%top{
/* flexdef.h includes config.h, which may contain macros that alter the API */
/* of libc functions. Must include first before any libc header. */
#include "flexdef.h"
}

%{
/*  Copyright (c) 1990 The Regents of the University of California. */
/*  All rights reserved. */

/*  This code is derived from software contributed to Berkeley by */
/*  Vern Paxson. */

/*  The United States Government has rights in this work pursuant */
/*  to contract no. DE-AC03-76SF00098 between the United States */
/*  Department of Energy and the University of California. */

/*  This file is part of flex. */

/*  Redistribution and use in source and binary forms, with or without */
/*  modification, are permitted provided that the following conditions */
/*  are met: */

/*  1. Redistributions of source code must retain the above copyright */
/*     notice, this list of conditions and the following disclaimer. */
/*  2. Redistributions in binary form must reproduce the above copyright */
/*     notice, this list of conditions and the following disclaimer in the */
/*     documentation and/or other materials provided with the distribution. */

/*  Neither the name of the University nor the names of its contributors */
/*  may be used to endorse or promote products derived from this software */
/*  without specific prior written permission. */

/*  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR */
/*  IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED */
/*  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR */
/*  PURPOSE. */

#include "parse.h"
//extern bool tablesverify, tablesext;
//extern bool trlcontxt; /* Set in parse.y for each rule. */
//extern const char *escaped_qstart, *escaped_qend;

#define M4QSTART "[""["
#define M4QEND "]""]"

#define ESCAPED_QSTART "[" M4QEND M4QSTART "[" M4QEND M4QSTART
#define ESCAPED_QEND M4QEND "]" M4QSTART M4QEND "]" M4QSTART

#define ACTION_ECHO add_action( gv, yytext )
#define ACTION_ECHO_QSTART add_action (gv, ESCAPED_QSTART)
#define ACTION_ECHO_QEND   add_action (gv, ESCAPED_QEND)

#define MARK_END_OF_PROLOG mark_prolog();

#define YY_DECL \
	int flexscan(YYSTYPE * yylval_param, yyscan_t yyscanner)

#define RETURNCHAR \
	*yylval = (unsigned char) yytext[0]; \
	return CHAR;

#define RETURNNAME \
	if(yyleng < MAXLINE) \
         { \
	strncpy( gv->nmstr, yytext, sizeof(gv->nmstr) ); \
	return NAME; \
	 } \
	else \
	 do { \
	   synerr(gv, _("Input line too long\n")); \
	   FLEX_EXIT(EXIT_FAILURE);  \
	 } while (0)

#define PUT_BACK_STRING(str, start) \
	{ size_t i = strlen( str );	\
	  while ( i > start )		\
	    unput((str)[--i]);		\
	}

#define CHECK_REJECT(str) \
	if ( all_upper( str ) ) \
		gv->reject = true;

#define CHECK_YYREJECT(str) \
	if ( all_lower( str ) ) \
		gv->reject = true;

#define YY_USER_INIT \
	if ( getenv("POSIXLY_CORRECT") ) {\
                FlexState* gv = yyget_extra ( yyscanner ); \
		gv->ctrl.posix_compat = true; \
                }

#define START_CODEBLOCK(x) do { \
    /* Emit the needed line directive... */\
    if (gv->indented_code == false) { \
        if (!x) gv->linenum++; \
        line_directive_out(gv, NULL, gv->infilename, gv->linenum);	\
    } \
    add_action(gv, M4QSTART); \
    yy_push_state(CODEBLOCK, yyscanner); \
    if ((gv->indented_code = x)) ACTION_ECHO; \
} while(0)

#define END_CODEBLOCK do { \
    yy_pop_state(yyscanner);\
    add_action(gv, M4QEND); \
    if (!gv->indented_code) line_directive_out(gv, NULL, NULL, gv->linenum);	\
} while (0)

#define YY_EXTRA_TYPE FlexState*
%}

%option caseless nodefault noreject stack noyy_top_state
%option nostdinit reentrant bison-bridge

%x SECT2 SECT2PROLOG SECT3 CODEBLOCK PICKUPDEF SC CARETISBOL NUM QUOTE
%x FIRSTCCL CCL ACTION RECOVER COMMENT ACTION_STRING PERCENT_BRACE_ACTION
%x OPTION LINEDIR CODEBLOCK_MATCH_BRACE
%x GROUP_WITH_PARAMS
%x GROUP_MINUS_PARAMS
%x EXTENDED_COMMENT
%x COMMENT_DISCARD CODE_COMMENT
%x SECT3_NOESCAPE
%x CHARACTER_CONSTANT

WS		[[:blank:]]+
OPTWS		[[:blank:]]*
NOT_WS		[^[:blank:]\r\n]

NL		\r?\n

NAME		([[:alpha:]_][[:alnum:]_-]*)
NOT_NAME	[^[:alpha:]_*\n]+

SCNAME		{NAME}

ESCSEQ		(\\([^\n]|[0-7]{1,3}|x[[:xdigit:]]{1,2}))

FIRST_CCL_CHAR	([^\\\n]|{ESCSEQ})
CCL_CHAR	([^\\\n\]]|{ESCSEQ})
CCL_EXPR	("[:"^?[[:alpha:]]+":]")

LEXOPT		[aceknopr]

M4QSTART    "[""["
M4QEND      "]""]"

FUNARGS     [^)]*

%{
void context_call(FlexState* gv, char *);
void context_member(FlexState* gv, char *, const char *);
#undef yyreject
%}
%%
	//static int bracelevel, didadef, indented_code;
	//static bool doing_rule_action = false;
	//static bool option_sense;

	//static bool doing_codeblock = false;
	//int brace_depth=0, brace_start_line=0;
	//char nmdef[MAXLINE];
        FlexState* gv = yyget_extra ( yyscanner );


<INITIAL>{
	^{WS}		START_CODEBLOCK(true);
	^"/*"		add_action(gv, "/*[""["); yy_push_state( COMMENT, yyscanner );
	^#{OPTWS}line{WS}	yy_push_state( LINEDIR, yyscanner );
	^"%s"{NAME}?	return SCDECL;
	^"%x"{NAME}?	return XSCDECL;
	^"%{".*{NL}	START_CODEBLOCK(false);
    ^"%top"[[:blank:]]*"{"[[:blank:]]*{NL}    {
		char trampoline[512];
                gv->brace_start_line = gv->linenum;
                ++gv->linenum;
		snprintf(trampoline, sizeof(trampoline),
			 "M4_HOOK_TRACE_LINE_FORMAT(%d, [[%s]])",
			 gv->linenum, gv->infilename?gv->infilename:"<stdin>");
                buf_strappend(gv, &gv->top_buf, trampoline);
                gv->brace_depth = 1;
                yy_push_state(CODEBLOCK_MATCH_BRACE, yyscanner);
            }

    ^"%top".*   synerr(gv, _("malformed '%top' directive") );

	{WS}		/* discard */

	^"%%".*		{
			gv->sectnum = 2;
			gv->bracelevel = 0;
			mark_defs1(gv);
			line_directive_out(gv, NULL, gv->infilename, gv->linenum);
			BEGIN(SECT2PROLOG);
			return SECTEND;
			}

	^"%pointer".*{NL}	gv->ctrl.yytext_is_array = false; ++gv->linenum;
	^"%array".*{NL}		gv->ctrl.yytext_is_array = true; ++gv->linenum;

	^"%option"	BEGIN(OPTION); return TOK_OPTION;

	^"%"{LEXOPT}{OPTWS}[[:digit:]]*{OPTWS}{NL}	++gv->linenum; /* ignore */
	^"%"{LEXOPT}{WS}.*{NL}	++gv->linenum;	/* ignore */

	/* xgettext: no-c-format */
	^"%"[^sxaceknopr{}].*	synerr(gv, _( "unrecognized '%' directive" ) );

	^{NAME}		{
			if(yyleng < MAXLINE)
        		 {
			strncpy( gv->nmstr, yytext, sizeof(gv->nmstr) );
			 }
			else
			 {
			   synerr(gv, _("Definition name too long\n"));
			   FLEX_EXIT(EXIT_FAILURE);
			 }

			gv->didadef = false;
			BEGIN(PICKUPDEF);
			}

	{SCNAME}	RETURNNAME;
	^{OPTWS}{NL}	++gv->linenum; /* allows blank lines in section 1 */
	{OPTWS}{NL}	ACTION_ECHO; ++gv->linenum; /* maybe end of comment line */
}


<COMMENT,CODE_COMMENT>{ /* */
        [^\[\]\*\n]*  ACTION_ECHO;
        .           ACTION_ECHO;

	{NL}	    ++gv->linenum; ACTION_ECHO;
}
<COMMENT>{
	"*/"	    add_action(gv, "*/]""]"); yy_pop_state(yyscanner);
}
<CODE_COMMENT>{
        "*/"        ACTION_ECHO; yy_pop_state(yyscanner);
}

<COMMENT_DISCARD>{
        /* This is the same as COMMENT, but is discarded rather than output. */
	"*/"		yy_pop_state(yyscanner);
    "*"         ;
	[^*\n]      ;
	{NL}	    ++gv->linenum;
}

<EXTENDED_COMMENT>{
    ")"         yy_pop_state(yyscanner);
    [^\n\)]+      ;
    {NL}        ++gv->linenum;
}

<LINEDIR>{
	\n		yy_pop_state(yyscanner);
	[[:digit:]]+	gv->linenum = myctoi( yytext );

	"\""[^""\n]*"\""	{
			free(gv->infilename);
			gv->infilename = xstrdup(gv, yytext + 1);
			gv->infilename[strlen( gv->infilename ) - 1] = '\0';
			}
	.		/* ignore spurious characters */
}
<ACTION,CODEBLOCK,ACTION_STRING,PERCENT_BRACE_ACTION,CHARACTER_CONSTANT,COMMENT,CODE_COMMENT>{
   {M4QSTART}   ACTION_ECHO_QSTART;
   {M4QEND}     ACTION_ECHO_QEND;
}

<CODEBLOCK>{
	^"%}".*{NL}	++gv->linenum; END_CODEBLOCK;
	[^\n%\[\]]*         ACTION_ECHO;
        .		ACTION_ECHO;
	{NL}		{
			++gv->linenum;
			ACTION_ECHO;
			if ( gv->indented_code ) END_CODEBLOCK;
			}
}

<CODEBLOCK_MATCH_BRACE>{
    "}"     {
                if( --gv->brace_depth == 0){
                    /* TODO: Matched. */
                    yy_pop_state(yyscanner);
                }else
                    buf_strnappend(gv, &gv->top_buf, yytext, yyleng);
            }

    "{"     {
                gv->brace_depth++;
                buf_strnappend(gv, &gv->top_buf, yytext, yyleng);
            }

    {NL}    {
                ++gv->linenum;
                buf_strnappend(gv, &gv->top_buf, yytext, yyleng);
            }

    {M4QSTART}  buf_strnappend(gv, &gv->top_buf, gv->escaped_qstart, (int) strlen(gv->escaped_qstart));
    {M4QEND}    buf_strnappend(gv, &gv->top_buf, gv->escaped_qend, (int) strlen(gv->escaped_qend));
    ([^{}\r\n\[\]]+)|[^{}\r\n]  {
       buf_strnappend(gv, &gv->top_buf, yytext, yyleng);
    }

    <<EOF>>     {
                gv->linenum = gv->brace_start_line;
                synerr(gv, _("Unmatched '{'"));
                return YY_NULL;
                }
}


<PICKUPDEF>{
	{WS}		/* separates name and definition */

	{NOT_WS}[^\r\n]*	{
 		        if(yyleng < MAXLINE)
 		         {
			strncpy( gv->nmdef, yytext, sizeof(gv->nmdef) );
 		         }
 		        else
 		         {
 		           format_synerr(gv, _("Definition value for {%s} too long\n"), gv->nmstr);
 		           FLEX_EXIT(EXIT_FAILURE);
			 }
			/* Skip trailing whitespace. */
			{
			    size_t i = strlen( gv->nmdef );
			    while (i > 0 && (gv->nmdef[i-1] == ' ' || gv->nmdef[i-1] == '\t'))
			       --i;
			    gv->nmdef[i] = '\0';
			}

			ndinstal( gv, gv->nmstr, gv->nmdef );
			gv->didadef = true;
			}

	{NL}		{
			if ( ! gv->didadef )
				synerr(gv, _( "incomplete name definition" ) );
			BEGIN(INITIAL);
			++gv->linenum;
			}
}


<OPTION>{
	{NL}		++gv->linenum; BEGIN(INITIAL);
	{WS}		gv->option_sense = true;

	"="		return '=';
	[[:digit:]]+	{gv->nmval = atoi(yytext); return TOK_NUMERIC;}

	no		gv->option_sense = ! gv->option_sense;

	7bit		gv->ctrl.csize = gv->option_sense ? 128 : 256;
	8bit		gv->ctrl.csize = gv->option_sense ? 256 : 128;

	align		gv->ctrl.long_align = gv->option_sense;
	always-interactive	{
			gv->ctrl.always_interactive = gv->option_sense;
			gv->ctrl.interactive = (trit)gv->option_sense;
			}
	array		gv->ctrl.yytext_is_array = gv->option_sense;
	backup		gv->env.backing_up_report = gv->option_sense;
	batch		gv->ctrl.interactive = (trit)!gv->option_sense;
	bison-bridge     gv->ctrl.bison_bridge_lval = gv->option_sense;
	bison-locations  { if((gv->ctrl.bison_bridge_lloc = gv->option_sense))
                            gv->ctrl.bison_bridge_lval = true;
			}
	"c++"		gv->ctrl.C_plus_plus = gv->option_sense;
	caseful|case-sensitive		sf_set_case_ins(!gv->option_sense);
	caseless|case-insensitive	sf_set_case_ins(gv->option_sense);
	debug		gv->ctrl.ddebug = gv->option_sense;
	default		gv->ctrl.spprdflt = ! gv->option_sense;
	ecs		gv->ctrl.useecs = gv->option_sense;
	fast		{
			gv->ctrl.useecs = gv->ctrl.usemecs = false;
			gv->ctrl.use_read = gv->ctrl.fullspd = true;
			}
	full		{
			gv->ctrl.useecs = gv->ctrl.usemecs = false;
			gv->ctrl.use_read = gv->ctrl.fulltbl = true;
			}
	input		gv->ctrl.no_yyinput = ! gv->option_sense;
	yyinput		gv->ctrl.no_yyinput = ! gv->option_sense;
	interactive	gv->ctrl.interactive = (trit)gv->option_sense;
	lex-compat	gv->ctrl.lex_compat = gv->option_sense;
	posix-compat	gv->ctrl.posix_compat = gv->option_sense;
	line		gv->ctrl.gen_line_dirs = gv->option_sense;
	main		{
			gv->ctrl.do_main = gv->option_sense;
			/* Override yywrap */
			if (gv->option_sense)
                		gv->ctrl.do_yywrap = false;
			}
	meta-ecs	gv->ctrl.usemecs = gv->option_sense;
	never-interactive	{
			gv->ctrl.never_interactive = gv->option_sense;
            		gv->ctrl.interactive = (trit)!gv->option_sense;
			}
	perf-report	gv->env.performance_hint += gv->option_sense ? 1 : -1;
	pointer		gv->ctrl.yytext_is_array = ! gv->option_sense;
	read		gv->ctrl.use_read = gv->option_sense;
	reentrant	gv->ctrl.reentrant = gv->option_sense;
	reject		gv->ctrl.reject_really_used = gv->option_sense;
	rewrite		gv->ctrl.rewrite = gv->option_sense;
	stack		gv->ctrl.stack_used = gv->option_sense;
	stdinit		gv->ctrl.do_stdinit = gv->option_sense;
	stdout		gv->env.use_stdout = gv->option_sense;
	unistd		gv->ctrl.no_unistd = ! gv->option_sense;
	unput		gv->ctrl.no_yyunput = ! gv->option_sense;
	yyunput		gv->ctrl.no_yyunput = ! gv->option_sense;
	verbose		gv->env.printstats = gv->option_sense;
	warn		gv->env.nowarn = ! gv->option_sense;
	yylineno	gv->ctrl.do_yylineno = gv->option_sense;
	yymore		gv->ctrl.yymore_really_used = gv->option_sense;
	yywrap		gv->ctrl.do_yywrap = gv->option_sense;
	yyread		gv->ctrl.noyyread = !gv->option_sense;

	yypanic		gv->ctrl.no_yypanic = !gv->option_sense;

	yy_push_state	gv->ctrl.no_yy_push_state = ! gv->option_sense;
	yy_pop_state	gv->ctrl.no_yy_pop_state = ! gv->option_sense;
	yy_top_state	gv->ctrl.no_yy_top_state = ! gv->option_sense;

	yy_scan_buffer	gv->ctrl.no_yy_scan_buffer = ! gv->option_sense;
	yy_scan_bytes	gv->ctrl.no_yy_scan_bytes = ! gv->option_sense;
	yy_scan_string	gv->ctrl.no_yy_scan_string = ! gv->option_sense;

	yyalloc         gv->ctrl.no_flex_alloc = ! gv->option_sense;
	yyrealloc       gv->ctrl.no_flex_realloc = ! gv->option_sense;
	yyfree          gv->ctrl.no_flex_free = ! gv->option_sense;

	yyget_debug     gv->ctrl.no_get_debug = ! gv->option_sense;
	yyset_debug     gv->ctrl.no_set_debug = ! gv->option_sense;
	yyget_extra     gv->ctrl.no_yyget_extra = ! gv->option_sense;
	yyset_extra     gv->ctrl.no_yyset_extra = ! gv->option_sense;
	yyget_leng      gv->ctrl.no_yyget_leng = ! gv->option_sense;
	yyget_text      gv->ctrl.no_yyget_text = ! gv->option_sense;
	yyget_column    gv->ctrl.no_yyget_column = ! gv->option_sense;
	yyset_column    gv->ctrl.no_yyset_column = ! gv->option_sense;
	yyget_lineno    gv->ctrl.no_yyget_lineno = ! gv->option_sense;
	yyset_lineno    gv->ctrl.no_yyset_lineno = ! gv->option_sense;
	yyget_in        gv->ctrl.no_yyget_in = ! gv->option_sense;
	yyset_in        gv->ctrl.no_yyset_in = ! gv->option_sense;
	yyget_out       gv->ctrl.no_yyget_out = ! gv->option_sense;
	yyset_out       gv->ctrl.no_yyset_out = ! gv->option_sense;
	yyget_lval      gv->ctrl.no_yyget_lval = ! gv->option_sense;
	yyset_lval      gv->ctrl.no_yyset_lval = ! gv->option_sense;
	yyget_lloc      gv->ctrl.no_yyget_lloc = ! gv->option_sense;
	yyset_lloc      gv->ctrl.no_yyset_lloc = ! gv->option_sense;

	bufsize		return TOK_BUFSIZE;
	emit		return TOK_EMIT;
	extra-type	return TOK_EXTRA_TYPE;
	outfile		return TOK_OUTFILE;
	prefix		return TOK_PREFIX;
	yyclass		return TOK_YYCLASS;
	yylmax		return TOK_YYLMAX;
	yydecl		return TOK_YYDECL;
	yyterminate	return TOK_YYTERMINATE;
	pre-action	return TOK_PREACTION;
	post-action	return TOK_POSTACTION;
	user-init	return TOK_USERINIT;
	header(-file)?      return TOK_HEADER_FILE;
	tables-file         return TOK_TABLES_FILE;
	tables-verify   {
                    gv->tablesverify = gv->option_sense;
                    if (!gv->tablesext && gv->option_sense)
                        gv->tablesext = true;
                    }


	"\""[^""\n]*"\""	{
			if(yyleng-1 < MAXLINE)
        		 {
			strncpy( gv->nmstr, yytext + 1, sizeof(gv->nmstr) );
			 }
			else
			 {
			   synerr(gv, _("Option line too long\n"));
			   FLEX_EXIT(EXIT_FAILURE);
			 }
			gv->nmstr[strlen( gv->nmstr ) - 1] = '\0';
			return NAME;
			}

	(([a-mo-z]|n[a-np-z])[[:alpha:]\-+]*)|.	{
			format_synerr(gv, _( "unrecognized %%option: %s" ),
				yytext );
			BEGIN(RECOVER);
			}
}

<RECOVER>.*{NL}		++gv->linenum; BEGIN(INITIAL);


<SECT2PROLOG>{
	^"%{".*	++gv->bracelevel; yyless( 2 );	/* eat only %{ */
	^"%}".*	--gv->bracelevel; yyless( 2 );	/* eat only %} */

	^{WS} START_CODEBLOCK(true); /* indented code in prolog */

	^{NOT_WS}.*	{
        /* non-indented code */
		if ( gv->bracelevel <= 0 ) {
            /* not in %{ ... %} */
            yyless( 0 );	/* put it all back */
            yy_set_bol( 1 );
            mark_prolog(gv);
            BEGIN(SECT2);
        } else {
            START_CODEBLOCK(true);
        }
    }

	.		ACTION_ECHO;
	{NL}	++gv->linenum; ACTION_ECHO;

	<<EOF>>		{
			mark_prolog(gv);
			gv->sectnum = 0;
			return YY_NULL; /* to stop the parser */
			}
}

<SECT2>{
	^{OPTWS}{NL}	++gv->linenum; /* allow blank lines in section 2 */

	^{OPTWS}"%{"	{
			gv->indented_code = false;
			gv->doing_codeblock = true;
			gv->bracelevel = 1;
			BEGIN(PERCENT_BRACE_ACTION);
			}

	^{OPTWS}"<"	    {
                        /* Allow "<" to appear in (?x) patterns. */
                        if (!sf_skip_ws())
                            BEGIN(SC);
                        return '<';
                    }
	^{OPTWS}"^"	return '^';
	"\""		BEGIN(QUOTE); return '"';
	"{"/[[:digit:]]	{
			BEGIN(NUM);
			if ( gv->ctrl.lex_compat || gv->ctrl.posix_compat )
				return BEGIN_REPEAT_POSIX;
			else
				return BEGIN_REPEAT_FLEX;
			}
	"$"/([[:blank:]]|{NL})	return '$';

	{WS}"%{"		{
			gv->bracelevel = 1;
			BEGIN(PERCENT_BRACE_ACTION);

			if ( gv->in_rule )
				{
				gv->doing_rule_action = true;
				gv->in_rule = false;
				return '\n';
				}
			}
	{WS}"|".*{NL}	{
                        if (sf_skip_ws()){
                            /* We're in the middle of a (?x: ) pattern. */
                            /* Push back everything starting at the "|" */
                            int amt = (int) (strchr (yytext, '|') - yytext);
                            yyless(amt);
                        }
                        else {
                            add_action(gv, "]""]");
                            gv->continued_action = true;
                            ++gv->linenum;
                            return '\n';
                        }
                    }

	^{WS}"/*"	{

                if (sf_skip_ws()){
                    /* We're in the middle of a (?x: ) pattern. */
                    yy_push_state(COMMENT_DISCARD, yyscanner);
                }
                else{
                    yyless( yyleng - 2 );	/* put back '/', '*' */
                    gv->bracelevel = 0;
                    gv->continued_action = false;
                    BEGIN(ACTION);
                }
			}

	^{WS}		/* allow indented rules */ ;

	{WS}		{
            if (sf_skip_ws()){
                /* We're in the middle of a (?x: ) pattern. */
            }
            else{
                /* This rule is separate from the one below because
                 * otherwise we get variable trailing context, so
                 * we can't build the scanner using -{f,F}.
                 */
                gv->bracelevel = 0;
                gv->continued_action = false;
                BEGIN(ACTION);

                if ( gv->in_rule )
                    {
                    gv->doing_rule_action = true;
                    gv->in_rule = false;
                    return '\n';
                    }
            }
			}

	{OPTWS}{NL}	{
            if (sf_skip_ws()){
                /* We're in the middle of a (?x: ) pattern. */
                ++gv->linenum;
            }
            else{
                gv->bracelevel = 0;
                gv->continued_action = false;
                BEGIN(ACTION);
                unput( '\n' );	/* so <ACTION> sees it */

                if ( gv->in_rule )
                    {
                    gv->doing_rule_action = true;
                    gv->in_rule = false;
                    return '\n';
                    }
            }
			}

	^{OPTWS}"<<EOF>>"	|
	"<<EOF>>"	return EOF_OP;

	^"%%".*		{
			gv->sectnum = 3;
			BEGIN(gv->ctrl.no_section3_escape ? SECT3_NOESCAPE : SECT3);
			yyterminate(); /* to stop the parser */

			}

	"["({FIRST_CCL_CHAR}|{CCL_EXPR})({CCL_CHAR}|{CCL_EXPR})*	{
			int cclval;

			if(yyleng < MAXLINE)
        		 {
			strncpy( gv->nmstr, yytext, sizeof(gv->nmstr) );
			 }
			else
			 {
			   synerr(gv, _("Input line too long\n"));
			   FLEX_EXIT(EXIT_FAILURE);
			 }

			/* Check to see if we've already encountered this
			 * ccl.
			 */
			if (0 /* <--- This "0" effectively disables the reuse of a
                   * character class (purely based on its source text).
                   * The reason it was disabled is so yacc/bison can parse
                   * ccl operations, such as ccl difference and union.
                   */
                &&  (cclval = ccllookup( gv, gv->nmstr )) != 0 )
				{
     				 /* Dead code removed */
				}
			else
				{
				/* We fudge a bit.  We know that this ccl will
				 * soon be numbered as lastccl + 1 by cclinit.
				 */
				cclinstal( gv, gv->nmstr, gv->lastccl + 1 );

				/* Push back everything but the leading bracket
				 * so the ccl can be rescanned.
				 */
				yyless( 1 );

				BEGIN(FIRSTCCL);
				return '[';
				}
			}
    "{-}"       return CCL_OP_DIFF;
    "{+}"       return CCL_OP_UNION;


    /* Check for :space: at the end of the rule so we don't
     * wrap the expanded regex in '(' ')' -- breaking trailing
     * context.
     */
	"{"{NAME}"}"[[:space:]]?	 {
			char *nmdefptr;
            int end_is_ws, end_ch;

            end_ch = yytext[yyleng-1];
            end_is_ws = end_ch != '}' ? 1 : 0;

 			if(yyleng-1 < MAXLINE)
         		 {
			strncpy( gv->nmstr, yytext + 1, sizeof(gv->nmstr) );
 			 }
 			else
 			 {
 			   synerr(gv, _("Input line too long\n"));
 			   FLEX_EXIT(EXIT_FAILURE);
 			 }
gv->nmstr[yyleng - 2 - end_is_ws] = '\0';  /* chop trailing brace */

			if ( (nmdefptr = ndlookup( gv, gv->nmstr )) == 0 )
				format_synerr(gv,
					_( "undefined definition {%s}" ),
						gv->nmstr );

			else
				{ /* push back name surrounded by ()'s */
				size_t len = strlen( nmdefptr );
                if (end_is_ws)
                    unput(end_ch);

				if ( gv->ctrl.lex_compat || nmdefptr[0] == '^' ||
				     (len > 0 && nmdefptr[len - 1] == '$')
                     || (end_is_ws && gv->trlcontxt && !sf_skip_ws()))
					{ /* don't use ()'s after all */
					PUT_BACK_STRING(nmdefptr, 0);

					if ( nmdefptr[0] == '^' )
						BEGIN(CARETISBOL);
					}

				else
					{
					unput(')');
					PUT_BACK_STRING(nmdefptr, 0);
					unput('(');
					}
				}
			}

    "/*"        {
                    if (sf_skip_ws())
                        yy_push_state(COMMENT_DISCARD, yyscanner);
                    else{
                        /* Push back the "*" and return "/" as usual. */
                        yyless(1);
                        return '/';
                    }
                }

    "(?#"       {
                    if (gv->ctrl.lex_compat || gv->ctrl.posix_compat){
                        /* Push back the "?#" and treat it like a normal parens. */
                        yyless(1);
                        sf_push(gv);
                        return '(';
                    }
                    else
                        yy_push_state(EXTENDED_COMMENT, yyscanner);
                }
    "(?"        {
                    sf_push(gv);
                    if (gv->ctrl.lex_compat || gv->ctrl.posix_compat)
                        /* Push back the "?" and treat it like a normal parens. */
                        yyless(1);
                    else
                        BEGIN(GROUP_WITH_PARAMS);
                    return '(';
                }
    "("         sf_push(gv); return '(';
    ")"         {
                    if (gv->_sf_top_ix > 0) {
                        sf_pop(gv);
                        return ')';
                    } else
                        synerr(gv, _("unbalanced parenthesis"));
                }

	[/|*+?.(){}]	return (unsigned char) yytext[0];
	.		RETURNCHAR;
}


<SC>{
	{OPTWS}{NL}{OPTWS}	++gv->linenum;	/* Allow blank lines & continuations */
	[,*]		return (unsigned char) yytext[0];
	">"		BEGIN(SECT2); return '>';
	">"/^		BEGIN(CARETISBOL); return '>';
	{SCNAME}	RETURNNAME;
	.		{
			format_synerr(gv, _( "bad <start condition>: %s" ),
				yytext );
			}
}

<CARETISBOL>"^"		BEGIN(SECT2); return '^';


<QUOTE>{
	[^""\n]		RETURNCHAR;
	"\""		BEGIN(SECT2); return '"';

	{NL}		{
			synerr(gv, _( "missing quote" ) );
			BEGIN(SECT2);
			++gv->linenum;
			return '"';
			}
}

<GROUP_WITH_PARAMS>{
    ":"     BEGIN(SECT2);
    "-"     BEGIN(GROUP_MINUS_PARAMS);
    i       sf_set_case_ins(1);
    s       sf_set_dot_all(1);
    x       sf_set_skip_ws(1);
}
<GROUP_MINUS_PARAMS>{
    ":"     BEGIN(SECT2);
    i       sf_set_case_ins(0);
    s       sf_set_dot_all(0);
    x       sf_set_skip_ws(0);
}

<FIRSTCCL>{
	"^"/[^-\]\n]	BEGIN(CCL); return '^';
	"^"/("-"|"]")	return '^';
	.		BEGIN(CCL); RETURNCHAR;
}

<CCL>{
	-/[^\]\n]	return '-';
	[^\]\n]		RETURNCHAR;
	"]"		BEGIN(SECT2); return ']';
	.|{NL}		{
			synerr(gv, _( "bad character class" ) );
			BEGIN(SECT2);
			return ']';
			}
}

<FIRSTCCL,CCL>{
	"[:alnum:]"	BEGIN(CCL); return CCE_ALNUM;
	"[:alpha:]"	BEGIN(CCL); return CCE_ALPHA;
	"[:blank:]"	BEGIN(CCL); return CCE_BLANK;
	"[:cntrl:]"	BEGIN(CCL); return CCE_CNTRL;
	"[:digit:]"	BEGIN(CCL); return CCE_DIGIT;
	"[:graph:]"	BEGIN(CCL); return CCE_GRAPH;
	"[:lower:]"	BEGIN(CCL); return CCE_LOWER;
	"[:print:]"	BEGIN(CCL); return CCE_PRINT;
	"[:punct:]"	BEGIN(CCL); return CCE_PUNCT;
	"[:space:]"	BEGIN(CCL); return CCE_SPACE;
	"[:upper:]"	BEGIN(CCL); return CCE_UPPER;
	"[:xdigit:]"	BEGIN(CCL); return CCE_XDIGIT;

	"[:^alnum:]"	BEGIN(CCL); return CCE_NEG_ALNUM;
	"[:^alpha:]"	BEGIN(CCL); return CCE_NEG_ALPHA;
	"[:^blank:]"	BEGIN(CCL); return CCE_NEG_BLANK;
	"[:^cntrl:]"	BEGIN(CCL); return CCE_NEG_CNTRL;
	"[:^digit:]"	BEGIN(CCL); return CCE_NEG_DIGIT;
	"[:^graph:]"	BEGIN(CCL); return CCE_NEG_GRAPH;
	"[:^lower:]"	BEGIN(CCL); return CCE_NEG_LOWER;
	"[:^print:]"	BEGIN(CCL); return CCE_NEG_PRINT;
	"[:^punct:]"	BEGIN(CCL); return CCE_NEG_PUNCT;
	"[:^space:]"	BEGIN(CCL); return CCE_NEG_SPACE;
	"[:^upper:]"	BEGIN(CCL); return CCE_NEG_UPPER;
	"[:^xdigit:]"	BEGIN(CCL); return CCE_NEG_XDIGIT;
	{CCL_EXPR}	{
			format_synerr(gv,
				_( "bad character class expression: %s" ),
					yytext );
			BEGIN(CCL); return CCE_ALNUM;
			}
}

<NUM>{
	[[:digit:]]+	{
			*yylval = myctoi( yytext );
			return NUMBER;
			}

	","		return ',';
	"}"		{
			BEGIN(SECT2);
			if ( gv->ctrl.lex_compat || gv->ctrl.posix_compat )
				return END_REPEAT_POSIX;
			else
				return END_REPEAT_FLEX;
			}

	.		{
			synerr(gv, _( "bad character inside {}'s" ) );
			BEGIN(SECT2);
			return '}';
			}

	{NL}		{
			synerr(gv, _( "missing }" ) );
			BEGIN(SECT2);
			++gv->linenum;
			return '}';
			}
}


<PERCENT_BRACE_ACTION>{
	{OPTWS}"%}".*		gv->bracelevel = 0;

	<ACTION>"/*"		ACTION_ECHO; yy_push_state( CODE_COMMENT, yyscanner );

	<CODEBLOCK,ACTION>{
		"reject" {
			ACTION_ECHO;
			CHECK_REJECT(yytext);
        }
		"yyreject"{OPTWS}"("{OPTWS}")" {
			add_action(gv, "]""]M4_HOOK_REJECT[""[");
			CHECK_YYREJECT(yytext);
        }
		"yymore"{OPTWS}"("{OPTWS}")" {
			gv->yymore_used = true;
			if (gv->ctrl.rewrite)
				context_call(gv, yytext);
			else
				ACTION_ECHO;
        }
	}

    .       ACTION_ECHO;
	{NL}	{
		++gv->linenum;
		ACTION_ECHO;
		if (gv->bracelevel <= 0 || (gv->doing_codeblock && gv->indented_code)) {
            if ( gv->doing_rule_action )
                add_action( gv, "\t]""]M4_HOOK_STATE_CASE_BREAK\n" );

            gv->doing_rule_action = gv->doing_codeblock = false;
            BEGIN(SECT2);
        }
    }
}


	/* yyreject and yymore() are checked for above, in PERCENT_BRACE_ACTION */
<ACTION>{
	"{"		ACTION_ECHO; ++gv->bracelevel;
	"}"		ACTION_ECHO; --gv->bracelevel;
	yyecho\(\)|yyinput\(\)|yystart\({FUNARGS}\)|yybegin\({FUNARGS}\)|yyunput\({FUNARGS}\)|yypanic\({FUNARGS}\)|yyatbol\(\)|yysetbol\({FUNARGS}\) {
			if (gv->ctrl.rewrite)
				context_call(gv, yytext);
			else
				ACTION_ECHO;
	}
	yyterminate\(\)|yyless\({FUNARGS}\) {
			add_action(gv, "]""]");
			add_action(gv, yytext);
			add_action(gv, "[""[");
	}
	(yyin|yyout|yyextra|yyleng|yytext|yyflexdebug)/[^[:alnum:]_] {
			if (gv->ctrl.rewrite)
				context_member(gv, yytext, "M4_PROPERTY_CONTEXT_FORMAT");
			else
				ACTION_ECHO;
	}
	(yylineno|yycolumn)/[^[:alnum:]_] {
			if (gv->ctrl.rewrite)
				context_member(gv, yytext, "M4_PROPERTY_BUFFERSTACK_CONTEXT_FORMAT");
			else
				ACTION_ECHO;
	}
	[^[:Alpha:]_{}""''/\n\[\]]+	ACTION_ECHO;
        {NAME}		ACTION_ECHO;
        "'"([^''\\\n]|\\.)"'" ACTION_ECHO; /* character constant */
        "'"             ACTION_ECHO; BEGIN(CHARACTER_CONSTANT);
	"\""		ACTION_ECHO; BEGIN(ACTION_STRING);
	{NL} {
                ++gv->linenum;
                ACTION_ECHO;
                if (gv->bracelevel <= 0) {
                   if ( gv->doing_rule_action )
                      add_action( gv, "\t]""]M4_HOOK_STATE_CASE_BREAK\n" );

                   gv->doing_rule_action = false;
                   BEGIN(SECT2);
                }
             }
        .      ACTION_ECHO;
}

<ACTION_STRING>{
	[^\[\]""\\\n]+	ACTION_ECHO;
	"\""		ACTION_ECHO; BEGIN(ACTION);
}
<CHARACTER_CONSTANT>{
	[^\[\]''\\\n]+  ACTION_ECHO;
        "'"             ACTION_ECHO; BEGIN(ACTION);
}
<ACTION_STRING,CHARACTER_CONSTANT>{
        (\\\n)*         ACTION_ECHO;
	\\(\\\n)*.	ACTION_ECHO;
	{NL}	++gv->linenum; ACTION_ECHO; if (gv->bracelevel <= 0) { BEGIN(SECT2); } else { BEGIN(ACTION); }
        .	ACTION_ECHO;
}

<COMMENT,CODE_COMMENT,COMMENT_DISCARD,ACTION,ACTION_STRING,CHARACTER_CONSTANT><<EOF>>	{
			synerr(gv, _( "EOF encountered inside an action" ) );
			yyterminate();
			}

<EXTENDED_COMMENT,GROUP_WITH_PARAMS,GROUP_MINUS_PARAMS><<EOF>>	{
			synerr(gv, _( "EOF encountered inside pattern" ) );
			yyterminate();
			}

<SECT2,QUOTE,FIRSTCCL,CCL>{ESCSEQ}	{
			*yylval = myesc( (unsigned char *) yytext );

			if ( YY_START == FIRSTCCL )
				BEGIN(CCL);

			return CHAR;
			}

<SECT3>{
    {M4QSTART}   fputs(gv->escaped_qstart, yyout);
    {M4QEND}     fputs(gv->escaped_qend, yyout);
    [^\[\]]*     ECHO;
    [][]         ECHO;
    <<EOF>>      {
        gv->sectnum = 0;
        yyterminate();
    }
}
<SECT3_NOESCAPE>{
    {M4QSTART}  fprintf(yyout, "[""[%s]""]", gv->escaped_qstart);
    {M4QEND}    fprintf(yyout, "[""[%s]""]", gv->escaped_qend);
    [^][]*      ECHO;
    [][]        ECHO;
    <<EOF>>		{
       gv->sectnum = 0;
       yyterminate();
    }
}
<*>.|\n			format_synerr(gv, _( "bad character: %s" ), yytext );

%%


int yywrap(yyscan_t yyscanner)
	{
        FlexState* gv = yyget_extra ( yyscanner );
	if ( --gv->num_input_files > 0 )
		{
		set_input_file( gv, *++gv->input_files );
		return 0;
		}

	else
		return 1;
	}


/* set_input_file - open the given file (if NULL, stdin) for scanning */

void set_input_file( yyscan_t yyscanner, char *file )
	{
        struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
        FlexState* gv = yyget_extra ( yyscanner );
	if ( file && strcmp( file, "-" ) )
		{
                free(gv->infilename);
		gv->infilename = xstrdup(gv, file);
                if(gv->fp_infilename && gv->fp_infilename != stdin) fclose(gv->fp_infilename);
		yyin = fopen( gv->infilename, "r" );

		if ( yyin == NULL )
			lerr(gv, _( "can't open %s: %s" ), file, strerror(errno));
                gv->fp_infilename = yyin;
		}

	else
		{
		yyin = stdin;
                free(gv->infilename);
		gv->infilename = xstrdup(gv, "<stdin>");
		}

	gv->linenum = 1;
	}

void context_call(FlexState* gv, char *txt) {
	const char *context_arg = skel_property(gv, "M4_PROPERTY_CONTEXT_ARG");

	/* if there's no such property, simply pass through */
	if (context_arg == NULL) {
		add_action(gv, txt);
	} else {
		char buf[BUFSIZ];

		/* otherwise, supply the context string as a final argument */
		assert(txt[strlen(txt)-1] == ')');
		strncpy(buf, txt, sizeof(buf));
		buf[strlen(buf)-1] = '\0';	/* remove trailing ) */
		add_action(gv, buf);
		if (txt[strlen(buf)-1] != '(') {
			add_action(gv, ", ");
		}
		add_action(gv, context_arg);
		add_action(gv, ")");
	}
}

void context_member(FlexState* gv, char *txt, const char *prop) {
	const char *context_format = skel_property(gv, prop);

	/* if there's no such property, simply pass through */
	if (context_format == NULL) {
		add_action(gv, txt);
	} else {
		char buf[128];
		snprintf(buf, sizeof(buf), context_format, txt);
		add_action(gv, buf);
	}
}
