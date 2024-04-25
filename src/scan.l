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
extern bool tablesverify, tablesext;
extern bool trlcontxt; /* Set in parse.y for each rule. */
extern const char *escaped_qstart, *escaped_qend;

#define M4QSTART "[""["
#define M4QEND "]""]"

#define ESCAPED_QSTART "[" M4QEND M4QSTART "[" M4QEND M4QSTART
#define ESCAPED_QEND M4QEND "]" M4QSTART M4QEND "]" M4QSTART

#define ACTION_ECHO add_action( yytext )
#define ACTION_ECHO_QSTART add_action (ESCAPED_QSTART)
#define ACTION_ECHO_QEND   add_action (ESCAPED_QEND)

#define MARK_END_OF_PROLOG mark_prolog();

#define YY_DECL \
	int flexscan(void)

#define RETURNCHAR \
	yylval = (unsigned char) yytext[0]; \
	return CHAR;

#define RETURNNAME \
	if(yyleng < MAXLINE) \
         { \
	strncpy( nmstr, yytext, sizeof(nmstr) ); \
	return NAME; \
	 } \
	else \
	 do { \
	   synerr(_("Input line too long\n")); \
	   FLEX_EXIT(EXIT_FAILURE);  \
	 } while (0)

#define PUT_BACK_STRING(str, start) \
	{ size_t i = strlen( str );	\
	  while ( i > start )		\
	    unput((str)[--i]);		\
	}

#define CHECK_REJECT(str) \
	if ( all_upper( str ) ) \
		reject = true;

#define CHECK_YYREJECT(str) \
	if ( all_lower( str ) ) \
		reject = true;

#define YY_USER_INIT \
	if ( getenv("POSIXLY_CORRECT") ) \
		ctrl.posix_compat = true;

#define START_CODEBLOCK(x) do { \
    /* Emit the needed line directive... */\
    if (indented_code == false) { \
        if (!x) linenum++; \
        line_directive_out(NULL, infilename, linenum);	\
    } \
    add_action(M4QSTART); \
    yy_push_state(CODEBLOCK); \
    if ((indented_code = x)) ACTION_ECHO; \
} while(0)

#define END_CODEBLOCK do { \
    yy_pop_state();\
    add_action(M4QEND); \
    if (!indented_code) line_directive_out(NULL, NULL, linenum);	\
} while (0)

%}

%option caseless nodefault noreject stack noyy_top_state
%option nostdinit

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
void context_call(char *);
void context_member(char *, const char *);
#undef yyreject
%}
%%
	static int bracelevel, didadef, indented_code;
	static bool doing_rule_action = false;
	static bool option_sense;

	static bool doing_codeblock = false;
	int brace_depth=0, brace_start_line=0;
	char nmdef[MAXLINE];


<INITIAL>{
	^{WS}		START_CODEBLOCK(true);
	^"/*"		add_action("/*[""["); yy_push_state( COMMENT );
	^#{OPTWS}line{WS}	yy_push_state( LINEDIR );
	^"%s"{NAME}?	return SCDECL;
	^"%x"{NAME}?	return XSCDECL;
	^"%{".*{NL}	START_CODEBLOCK(false);
    ^"%top"[[:blank:]]*"{"[[:blank:]]*{NL}    {
		char trampoline[512];
                brace_start_line = linenum;
                ++linenum;
		snprintf(trampoline, sizeof(trampoline),
			 "M4_HOOK_TRACE_LINE_FORMAT(%d, [[%s]])",
			 linenum, infilename?infilename:"<stdin>");
                buf_strappend(&top_buf, trampoline);
                brace_depth = 1;
                yy_push_state(CODEBLOCK_MATCH_BRACE);
            }

    ^"%top".*   synerr( _("malformed '%top' directive") );

	{WS}		/* discard */

	^"%%".*		{
			sectnum = 2;
			bracelevel = 0;
			mark_defs1();
			line_directive_out(NULL, infilename, linenum);
			BEGIN(SECT2PROLOG);
			return SECTEND;
			}

	^"%pointer".*{NL}	ctrl.yytext_is_array = false; ++linenum;
	^"%array".*{NL}		ctrl.yytext_is_array = true; ++linenum;

	^"%option"	BEGIN(OPTION); return TOK_OPTION;

	^"%"{LEXOPT}{OPTWS}[[:digit:]]*{OPTWS}{NL}	++linenum; /* ignore */
	^"%"{LEXOPT}{WS}.*{NL}	++linenum;	/* ignore */

	/* xgettext: no-c-format */
	^"%"[^sxaceknopr{}].*	synerr( _( "unrecognized '%' directive" ) );

	^{NAME}		{
			if(yyleng < MAXLINE)
        		 {
			strncpy( nmstr, yytext, sizeof(nmstr) );
			 }
			else
			 {
			   synerr( _("Definition name too long\n"));
			   FLEX_EXIT(EXIT_FAILURE);
			 }

			didadef = false;
			BEGIN(PICKUPDEF);
			}

	{SCNAME}	RETURNNAME;
	^{OPTWS}{NL}	++linenum; /* allows blank lines in section 1 */
	{OPTWS}{NL}	ACTION_ECHO; ++linenum; /* maybe end of comment line */
}


<COMMENT,CODE_COMMENT>{ /* */
        [^\[\]\*\n]*  ACTION_ECHO;
        .           ACTION_ECHO;

	{NL}	    ++linenum; ACTION_ECHO;
}
<COMMENT>{
	"*/"	    add_action("*/]""]"); yy_pop_state();
}
<CODE_COMMENT>{
        "*/"        ACTION_ECHO; yy_pop_state();
}

<COMMENT_DISCARD>{
        /* This is the same as COMMENT, but is discarded rather than output. */
	"*/"		yy_pop_state();
    "*"         ;
	[^*\n]      ;
	{NL}	    ++linenum;
}

<EXTENDED_COMMENT>{
    ")"         yy_pop_state();
    [^\n\)]+      ;
    {NL}        ++linenum;
}

<LINEDIR>{
	\n		yy_pop_state();
	[[:digit:]]+	linenum = myctoi( yytext );

	"\""[^""\n]*"\""	{
			free(infilename);
			infilename = xstrdup(yytext + 1);
			infilename[strlen( infilename ) - 1] = '\0';
			}
	.		/* ignore spurious characters */
}
<ACTION,CODEBLOCK,ACTION_STRING,PERCENT_BRACE_ACTION,CHARACTER_CONSTANT,COMMENT,CODE_COMMENT>{
   {M4QSTART}   ACTION_ECHO_QSTART;
   {M4QEND}     ACTION_ECHO_QEND;
}

<CODEBLOCK>{
	^"%}".*{NL}	++linenum; END_CODEBLOCK;
	[^\n%\[\]]*         ACTION_ECHO;
        .		ACTION_ECHO;
	{NL}		{
			++linenum;
			ACTION_ECHO;
			if ( indented_code ) END_CODEBLOCK;
			}
}

<CODEBLOCK_MATCH_BRACE>{
    "}"     {
                if( --brace_depth == 0){
                    /* TODO: Matched. */
                    yy_pop_state();
                }else
                    buf_strnappend(&top_buf, yytext, yyleng);
            }

    "{"     {
                brace_depth++;
                buf_strnappend(&top_buf, yytext, yyleng);
            }

    {NL}    {
                ++linenum;
                buf_strnappend(&top_buf, yytext, yyleng);
            }

    {M4QSTART}  buf_strnappend(&top_buf, escaped_qstart, (int) strlen(escaped_qstart));
    {M4QEND}    buf_strnappend(&top_buf, escaped_qend, (int) strlen(escaped_qend));
    ([^{}\r\n\[\]]+)|[^{}\r\n]  {
       buf_strnappend(&top_buf, yytext, yyleng);
    }

    <<EOF>>     {
                linenum = brace_start_line;
                synerr(_("Unmatched '{'"));
                return YY_NULL;
                }
}


<PICKUPDEF>{
	{WS}		/* separates name and definition */

	{NOT_WS}[^\r\n]*	{
 		        if(yyleng < MAXLINE)
 		         {
			strncpy( nmdef, yytext, sizeof(nmdef) );
 		         }
 		        else
 		         {
 		           format_synerr( _("Definition value for {%s} too long\n"), nmstr);
 		           FLEX_EXIT(EXIT_FAILURE);
			 }
			/* Skip trailing whitespace. */
			{
			    size_t i = strlen( nmdef );
			    while (i > 0 && (nmdef[i-1] == ' ' || nmdef[i-1] == '\t'))
			       --i;
			    nmdef[i] = '\0';
			}

			ndinstal( nmstr, nmdef );
			didadef = true;
			}

	{NL}		{
			if ( ! didadef )
				synerr( _( "incomplete name definition" ) );
			BEGIN(INITIAL);
			++linenum;
			}
}


<OPTION>{
	{NL}		++linenum; BEGIN(INITIAL);
	{WS}		option_sense = true;

	"="		return '=';
	[[:digit:]]+	{nmval = atoi(yytext); return TOK_NUMERIC;}

	no		option_sense = ! option_sense;

	7bit		ctrl.csize = option_sense ? 128 : 256;
	8bit		ctrl.csize = option_sense ? 256 : 128;

	align		ctrl.long_align = option_sense;
	always-interactive	{
			ctrl.always_interactive = option_sense;
			ctrl.interactive = (trit)option_sense;
			}
	array		ctrl.yytext_is_array = option_sense;
	backup		env.backing_up_report = option_sense;
	batch		ctrl.interactive = (trit)!option_sense;
	bison-bridge     ctrl.bison_bridge_lval = option_sense;
	bison-locations  { if((ctrl.bison_bridge_lloc = option_sense))
                            ctrl.bison_bridge_lval = true;
			}
	"c++"		ctrl.C_plus_plus = option_sense;
	caseful|case-sensitive		sf_set_case_ins(!option_sense);
	caseless|case-insensitive	sf_set_case_ins(option_sense);
	debug		ctrl.ddebug = option_sense;
	default		ctrl.spprdflt = ! option_sense;
	ecs		ctrl.useecs = option_sense;
	fast		{
			ctrl.useecs = ctrl.usemecs = false;
			ctrl.use_read = ctrl.fullspd = true;
			}
	full		{
			ctrl.useecs = ctrl.usemecs = false;
			ctrl.use_read = ctrl.fulltbl = true;
			}
	input		ctrl.no_yyinput = ! option_sense;
	yyinput		ctrl.no_yyinput = ! option_sense;
	interactive	ctrl.interactive = (trit)option_sense;
	lex-compat	ctrl.lex_compat = option_sense;
	posix-compat	ctrl.posix_compat = option_sense;
	line		ctrl.gen_line_dirs = option_sense;
	main		{
			ctrl.do_main = option_sense;
			/* Override yywrap */
			if (option_sense)
                		ctrl.do_yywrap = false;
			}
	meta-ecs	ctrl.usemecs = option_sense;
	never-interactive	{
			ctrl.never_interactive = option_sense;
            		ctrl.interactive = (trit)!option_sense;
			}
	perf-report	env.performance_hint += option_sense ? 1 : -1;
	pointer		ctrl.yytext_is_array = ! option_sense;
	read		ctrl.use_read = option_sense;
	reentrant	ctrl.reentrant = option_sense;
	reject		ctrl.reject_really_used = option_sense;
	rewrite		ctrl.rewrite = option_sense;
	stack		ctrl.stack_used = option_sense;
	stdinit		ctrl.do_stdinit = option_sense;
	stdout		env.use_stdout = option_sense;
	unistd		ctrl.no_unistd = ! option_sense;
	unput		ctrl.no_yyunput = ! option_sense;
	yyunput		ctrl.no_yyunput = ! option_sense;
	verbose		env.printstats = option_sense;
	warn		env.nowarn = ! option_sense;
	yylineno	ctrl.do_yylineno = option_sense;
	yymore		ctrl.yymore_really_used = option_sense;
	yywrap		ctrl.do_yywrap = option_sense;
	yyread		ctrl.noyyread = !option_sense;

	yypanic		ctrl.no_yypanic = !option_sense;

	yy_push_state	ctrl.no_yy_push_state = ! option_sense;
	yy_pop_state	ctrl.no_yy_pop_state = ! option_sense;
	yy_top_state	ctrl.no_yy_top_state = ! option_sense;

	yy_scan_buffer	ctrl.no_yy_scan_buffer = ! option_sense;
	yy_scan_bytes	ctrl.no_yy_scan_bytes = ! option_sense;
	yy_scan_string	ctrl.no_yy_scan_string = ! option_sense;

	yyalloc         ctrl.no_flex_alloc = ! option_sense;
	yyrealloc       ctrl.no_flex_realloc = ! option_sense;
	yyfree          ctrl.no_flex_free = ! option_sense;

	yyget_debug     ctrl.no_get_debug = ! option_sense;
	yyset_debug     ctrl.no_set_debug = ! option_sense;
	yyget_extra     ctrl.no_yyget_extra = ! option_sense;
	yyset_extra     ctrl.no_yyset_extra = ! option_sense;
	yyget_leng      ctrl.no_yyget_leng = ! option_sense;
	yyget_text      ctrl.no_yyget_text = ! option_sense;
	yyget_column    ctrl.no_yyget_column = ! option_sense;
	yyset_column    ctrl.no_yyset_column = ! option_sense;
	yyget_lineno    ctrl.no_yyget_lineno = ! option_sense;
	yyset_lineno    ctrl.no_yyset_lineno = ! option_sense;
	yyget_in        ctrl.no_yyget_in = ! option_sense;
	yyset_in        ctrl.no_yyset_in = ! option_sense;
	yyget_out       ctrl.no_yyget_out = ! option_sense;
	yyset_out       ctrl.no_yyset_out = ! option_sense;
	yyget_lval      ctrl.no_yyget_lval = ! option_sense;
	yyset_lval      ctrl.no_yyset_lval = ! option_sense;
	yyget_lloc      ctrl.no_yyget_lloc = ! option_sense;
	yyset_lloc      ctrl.no_yyset_lloc = ! option_sense;

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
                    tablesverify = option_sense;
                    if (!tablesext && option_sense)
                        tablesext = true;
                    }


	"\""[^""\n]*"\""	{
			if(yyleng-1 < MAXLINE)
        		 {
			strncpy( nmstr, yytext + 1, sizeof(nmstr) );
			 }
			else
			 {
			   synerr( _("Option line too long\n"));
			   FLEX_EXIT(EXIT_FAILURE);
			 }
			nmstr[strlen( nmstr ) - 1] = '\0';
			return NAME;
			}

	(([a-mo-z]|n[a-np-z])[[:alpha:]\-+]*)|.	{
			format_synerr( _( "unrecognized %%option: %s" ),
				yytext );
			BEGIN(RECOVER);
			}
}

<RECOVER>.*{NL}		++linenum; BEGIN(INITIAL);


<SECT2PROLOG>{
	^"%{".*	++bracelevel; yyless( 2 );	/* eat only %{ */
	^"%}".*	--bracelevel; yyless( 2 );	/* eat only %} */

	^{WS} START_CODEBLOCK(true); /* indented code in prolog */

	^{NOT_WS}.*	{
        /* non-indented code */
		if ( bracelevel <= 0 ) {
            /* not in %{ ... %} */
            yyless( 0 );	/* put it all back */
            yy_set_bol( 1 );
            mark_prolog();
            BEGIN(SECT2);
        } else {
            START_CODEBLOCK(true);
        }
    }

	.		ACTION_ECHO;
	{NL}	++linenum; ACTION_ECHO;

	<<EOF>>		{
			mark_prolog();
			sectnum = 0;
			return YY_NULL; /* to stop the parser */
			}
}

<SECT2>{
	^{OPTWS}{NL}	++linenum; /* allow blank lines in section 2 */

	^{OPTWS}"%{"	{
			indented_code = false;
			doing_codeblock = true;
			bracelevel = 1;
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
			if ( ctrl.lex_compat || ctrl.posix_compat )
				return BEGIN_REPEAT_POSIX;
			else
				return BEGIN_REPEAT_FLEX;
			}
	"$"/([[:blank:]]|{NL})	return '$';

	{WS}"%{"		{
			bracelevel = 1;
			BEGIN(PERCENT_BRACE_ACTION);

			if ( in_rule )
				{
				doing_rule_action = true;
				in_rule = false;
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
                            add_action("]""]");
                            continued_action = true;
                            ++linenum;
                            return '\n';
                        }
                    }

	^{WS}"/*"	{

                if (sf_skip_ws()){
                    /* We're in the middle of a (?x: ) pattern. */
                    yy_push_state(COMMENT_DISCARD);
                }
                else{
                    yyless( yyleng - 2 );	/* put back '/', '*' */
                    bracelevel = 0;
                    continued_action = false;
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
                bracelevel = 0;
                continued_action = false;
                BEGIN(ACTION);

                if ( in_rule )
                    {
                    doing_rule_action = true;
                    in_rule = false;
                    return '\n';
                    }
            }
			}

	{OPTWS}{NL}	{
            if (sf_skip_ws()){
                /* We're in the middle of a (?x: ) pattern. */
                ++linenum;
            }
            else{
                bracelevel = 0;
                continued_action = false;
                BEGIN(ACTION);
                unput( '\n' );	/* so <ACTION> sees it */

                if ( in_rule )
                    {
                    doing_rule_action = true;
                    in_rule = false;
                    return '\n';
                    }
            }
			}

	^{OPTWS}"<<EOF>>"	|
	"<<EOF>>"	return EOF_OP;

	^"%%".*		{
			sectnum = 3;
			BEGIN(ctrl.no_section3_escape ? SECT3_NOESCAPE : SECT3);
			yyterminate(); /* to stop the parser */

			}

	"["({FIRST_CCL_CHAR}|{CCL_EXPR})({CCL_CHAR}|{CCL_EXPR})*	{
			int cclval;

			if(yyleng < MAXLINE)
        		 {
			strncpy( nmstr, yytext, sizeof(nmstr) );
			 }
			else
			 {
			   synerr( _("Input line too long\n"));
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
                &&  (cclval = ccllookup( nmstr )) != 0 )
				{
     				 /* Dead code removed */
				}
			else
				{
				/* We fudge a bit.  We know that this ccl will
				 * soon be numbered as lastccl + 1 by cclinit.
				 */
				cclinstal( nmstr, lastccl + 1 );

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
			strncpy( nmstr, yytext + 1, sizeof(nmstr) );
 			 }
 			else
 			 {
 			   synerr( _("Input line too long\n"));
 			   FLEX_EXIT(EXIT_FAILURE);
 			 }
nmstr[yyleng - 2 - end_is_ws] = '\0';  /* chop trailing brace */

			if ( (nmdefptr = ndlookup( nmstr )) == 0 )
				format_synerr(
					_( "undefined definition {%s}" ),
						nmstr );

			else
				{ /* push back name surrounded by ()'s */
				size_t len = strlen( nmdefptr );
                if (end_is_ws)
                    unput(end_ch);

				if ( ctrl.lex_compat || nmdefptr[0] == '^' ||
				     (len > 0 && nmdefptr[len - 1] == '$')
                     || (end_is_ws && trlcontxt && !sf_skip_ws()))
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
                        yy_push_state(COMMENT_DISCARD);
                    else{
                        /* Push back the "*" and return "/" as usual. */
                        yyless(1);
                        return '/';
                    }
                }

    "(?#"       {
                    if (ctrl.lex_compat || ctrl.posix_compat){
                        /* Push back the "?#" and treat it like a normal parens. */
                        yyless(1);
                        sf_push(); 
                        return '(';
                    }
                    else
                        yy_push_state(EXTENDED_COMMENT);
                }
    "(?"        {
                    sf_push();
                    if (ctrl.lex_compat || ctrl.posix_compat)
                        /* Push back the "?" and treat it like a normal parens. */
                        yyless(1);
                    else
                        BEGIN(GROUP_WITH_PARAMS);
                    return '(';
                }
    "("         sf_push(); return '(';
    ")"         {
                    if (_sf_top_ix > 0) {
                        sf_pop();
                        return ')';
                    } else
                        synerr(_("unbalanced parenthesis"));
                }

	[/|*+?.(){}]	return (unsigned char) yytext[0];
	.		RETURNCHAR;
}


<SC>{
	{OPTWS}{NL}{OPTWS}	++linenum;	/* Allow blank lines & continuations */
	[,*]		return (unsigned char) yytext[0];
	">"		BEGIN(SECT2); return '>';
	">"/^		BEGIN(CARETISBOL); return '>';
	{SCNAME}	RETURNNAME;
	.		{
			format_synerr( _( "bad <start condition>: %s" ),
				yytext );
			}
}

<CARETISBOL>"^"		BEGIN(SECT2); return '^';


<QUOTE>{
	[^""\n]		RETURNCHAR;
	"\""		BEGIN(SECT2); return '"';

	{NL}		{
			synerr( _( "missing quote" ) );
			BEGIN(SECT2);
			++linenum;
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
			synerr( _( "bad character class" ) );
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
			format_synerr(
				_( "bad character class expression: %s" ),
					yytext );
			BEGIN(CCL); return CCE_ALNUM;
			}
}

<NUM>{
	[[:digit:]]+	{
			yylval = myctoi( yytext );
			return NUMBER;
			}

	","		return ',';
	"}"		{
			BEGIN(SECT2);
			if ( ctrl.lex_compat || ctrl.posix_compat )
				return END_REPEAT_POSIX;
			else
				return END_REPEAT_FLEX;
			}

	.		{
			synerr( _( "bad character inside {}'s" ) );
			BEGIN(SECT2);
			return '}';
			}

	{NL}		{
			synerr( _( "missing }" ) );
			BEGIN(SECT2);
			++linenum;
			return '}';
			}
}


<PERCENT_BRACE_ACTION>{
	{OPTWS}"%}".*		bracelevel = 0;

	<ACTION>"/*"		ACTION_ECHO; yy_push_state( CODE_COMMENT );

	<CODEBLOCK,ACTION>{
		"reject" {
			ACTION_ECHO;
			CHECK_REJECT(yytext);
        }
		"yyreject"{OPTWS}"("{OPTWS}")" {
			add_action("]""]M4_HOOK_REJECT[""[");
			CHECK_YYREJECT(yytext);
        }
		"yymore"{OPTWS}"("{OPTWS}")" {
			yymore_used = true;
			if (ctrl.rewrite)
				context_call(yytext);
			else
				ACTION_ECHO;
        }
	}

    .       ACTION_ECHO;
	{NL}	{
		++linenum;
		ACTION_ECHO;
		if (bracelevel <= 0 || (doing_codeblock && indented_code)) {
            if ( doing_rule_action )
                add_action( "\t]""]M4_HOOK_STATE_CASE_BREAK\n" );

            doing_rule_action = doing_codeblock = false;
            BEGIN(SECT2);
        }
    }
}


	/* yyreject and yymore() are checked for above, in PERCENT_BRACE_ACTION */
<ACTION>{
	"{"		ACTION_ECHO; ++bracelevel;
	"}"		ACTION_ECHO; --bracelevel;
	yyecho\(\)|yyinput\(\)|yystart\({FUNARGS}\)|yybegin\({FUNARGS}\)|yyunput\({FUNARGS}\)|yypanic\({FUNARGS}\)|yyatbol\(\)|yysetbol\({FUNARGS}\) {
			if (ctrl.rewrite)
				context_call(yytext);
			else
				ACTION_ECHO;
	}
	yyterminate\(\)|yyless\({FUNARGS}\) {
			add_action("]""]");
			add_action(yytext);
			add_action("[""[");
	}
	(yyin|yyout|yyextra|yyleng|yytext|yyflexdebug)/[^[:alnum:]_] {
			if (ctrl.rewrite)
				context_member(yytext, "M4_PROPERTY_CONTEXT_FORMAT");
			else
				ACTION_ECHO;
	}
	(yylineno|yycolumn)/[^[:alnum:]_] {
			if (ctrl.rewrite)
				context_member(yytext, "M4_PROPERTY_BUFFERSTACK_CONTEXT_FORMAT");
			else
				ACTION_ECHO;
	}
	[^[:Alpha:]_{}""''/\n\[\]]+	ACTION_ECHO;
        {NAME}		ACTION_ECHO;
        "'"([^''\\\n]|\\.)"'" ACTION_ECHO; /* character constant */
        "'"             ACTION_ECHO; BEGIN(CHARACTER_CONSTANT);
	"\""		ACTION_ECHO; BEGIN(ACTION_STRING);
	{NL} {
                ++linenum;
                ACTION_ECHO;
                if (bracelevel <= 0) {
                   if ( doing_rule_action )
                      add_action( "\t]""]M4_HOOK_STATE_CASE_BREAK\n" );

                   doing_rule_action = false;
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
	{NL}	++linenum; ACTION_ECHO; if (bracelevel <= 0) { BEGIN(SECT2); } else { BEGIN(ACTION); }
        .	ACTION_ECHO;
}

<COMMENT,CODE_COMMENT,COMMENT_DISCARD,ACTION,ACTION_STRING,CHARACTER_CONSTANT><<EOF>>	{
			synerr( _( "EOF encountered inside an action" ) );
			yyterminate();
			}

<EXTENDED_COMMENT,GROUP_WITH_PARAMS,GROUP_MINUS_PARAMS><<EOF>>	{
			synerr( _( "EOF encountered inside pattern" ) );
			yyterminate();
			}

<SECT2,QUOTE,FIRSTCCL,CCL>{ESCSEQ}	{
			yylval = myesc( (unsigned char *) yytext );

			if ( YY_START == FIRSTCCL )
				BEGIN(CCL);

			return CHAR;
			}

<SECT3>{
    {M4QSTART}   fputs(escaped_qstart, yyout);
    {M4QEND}     fputs(escaped_qend, yyout);
    [^\[\]]*     ECHO;
    [][]         ECHO;
    <<EOF>>      {
        sectnum = 0;
        yyterminate();
    }
}
<SECT3_NOESCAPE>{
    {M4QSTART}  fprintf(yyout, "[""[%s]""]", escaped_qstart);
    {M4QEND}    fprintf(yyout, "[""[%s]""]", escaped_qend);
    [^][]*      ECHO;
    [][]        ECHO;
    <<EOF>>		{
       sectnum = 0;
       yyterminate();
    }
}
<*>.|\n			format_synerr( _( "bad character: %s" ), yytext );

%%


int yywrap(void)
	{
	if ( --num_input_files > 0 )
		{
		set_input_file( *++input_files );
		return 0;
		}

	else
		return 1;
	}


/* set_input_file - open the given file (if NULL, stdin) for scanning */

void set_input_file( char *file )
	{
	if ( file && strcmp( file, "-" ) )
		{
		infilename = xstrdup(file);
		yyin = fopen( infilename, "r" );

		if ( yyin == NULL )
			lerr( _( "can't open %s: %s" ), file, strerror(errno));
		}

	else
		{
		yyin = stdin;
		infilename = xstrdup("<stdin>");
		}

	linenum = 1;
	}

void context_call(char *txt) {
	const char *context_arg = skel_property("M4_PROPERTY_CONTEXT_ARG");

	/* if there's no such property, simply pass through */
	if (context_arg == NULL) {
		add_action(txt);
	} else {
		char buf[BUFSIZ];

		/* otherwise, supply the context string as a final argument */
		assert(txt[strlen(txt)-1] == ')');
		strncpy(buf, txt, sizeof(buf));
		buf[strlen(buf)-1] = '\0';	/* remove trailing ) */
		add_action(buf);
		if (txt[strlen(buf)-1] != '(') {
			add_action(", ");
		}
		add_action(context_arg);
		add_action(")");
	}
}

void context_member(char *txt, const char *prop) {
	const char *context_format = skel_property(prop);

	/* if there's no such property, simply pass through */
	if (context_format == NULL) {
		add_action(txt);
	} else {
		char buf[128];
		snprintf(buf, sizeof(buf), context_format, txt);
		add_action(buf);
	}
}
