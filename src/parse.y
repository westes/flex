/* parse.y - parser for flex input */
%pure-parser
%lex-param {yyscan_t  yyscanner}
%parse-param {yyscan_t yyscanner, FlexState* gv}
%token CHAR NUMBER SECTEND SCDECL XSCDECL NAME PREVCCL EOF_OP
%token TOK_OPTION TOK_OUTFILE TOK_PREFIX TOK_YYCLASS TOK_HEADER_FILE TOK_EXTRA_TYPE
%token TOK_TABLES_FILE TOK_YYLMAX TOK_NUMERIC TOK_YYDECL TOK_PREACTION TOK_POSTACTION
%token TOK_USERINIT TOK_EMIT TOK_BUFSIZE TOK_YYTERMINATE

%token CCE_ALNUM CCE_ALPHA CCE_BLANK CCE_CNTRL CCE_DIGIT CCE_GRAPH
%token CCE_LOWER CCE_PRINT CCE_PUNCT CCE_SPACE CCE_UPPER CCE_XDIGIT

%token CCE_NEG_ALNUM CCE_NEG_ALPHA CCE_NEG_BLANK CCE_NEG_CNTRL CCE_NEG_DIGIT CCE_NEG_GRAPH
%token CCE_NEG_LOWER CCE_NEG_PRINT CCE_NEG_PUNCT CCE_NEG_SPACE CCE_NEG_UPPER CCE_NEG_XDIGIT

%left CCL_OP_DIFF CCL_OP_UNION

/*
 *POSIX and AT&T lex place the
 * precedence of the repeat operator, {}, below that of concatenation.
 * Thus, ab{3} is ababab.  Most other POSIX utilities use an Extended
 * Regular Expression (ERE) precedence that has the repeat operator
 * higher than concatenation.  This causes ab{3} to yield abbb.
 *
 * In order to support the POSIX and AT&T precedence and the flex
 * precedence we define two token sets for the begin and end tokens of
 * the repeat operator, '{' and '}'.  The lexical scanner chooses
 * which tokens to return based on whether posix_compat or lex_compat
 * are specified. Specifying either posix_compat or lex_compat will
 * cause flex to parse scanner files as per the AT&T and
 * POSIX-mandated behavior.
 */

%token BEGIN_REPEAT_POSIX END_REPEAT_POSIX BEGIN_REPEAT_FLEX END_REPEAT_FLEX


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

#include "flexdef.h"
#include "tables.h"

//int pat, scnum, eps, headcnt, trailcnt, lastchar, i, rulelen;
//static int currccl;
//bool trlcontxt;
//static bool sc_is_exclusive, cclsorted, varlength, variable_trail_rule;

//int *scon_stk;
//int scon_stk_ptr;

//static int madeany = false;  /* whether we've made the '.' character class */
//static int ccldot, cclany;
//int previous_continued_action;	/* whether the previous rule's action was '|' */

#define format_warn3(fmt, a1, a2) \
	do{ \
        char fw3_msg[MAXLINE];\
        snprintf( fw3_msg, MAXLINE,(fmt), (a1), (a2) );\
        lwarn( gv, fw3_msg );\
	}while(0)

/* Expand a POSIX character class expression. */
#define CCL_EXPR(func) \
	do{ \
	int c; \
	for ( c = 0; c < gv->ctrl.csize; ++c ) \
		if ( isascii(c) && func(c) ) \
			ccladd( gv, gv->currccl, c ); \
	}while(0)

/* negated class */
#define CCL_NEG_EXPR(func) \
	do{ \
	int c; \
	for ( c = 0; c < gv->ctrl.csize; ++c ) \
		if ( !func(c) ) \
			ccladd( gv, gv->currccl, c ); \
	}while(0)

/* While POSIX defines isblank(), it's not ANSI C. */
#define IS_BLANK(c) ((c) == ' ' || (c) == '\t')

/* On some over-ambitious machines, such as DEC Alpha's, the default
 * token type is "long" instead of "int"; this leads to problems with
 * declaring yylval in flexdef.h.  But so far, all the yacc's I've seen
 * wrap their definitions of YYSTYPE with "#ifndef YYSTYPE"'s, so the
 * following should ensure that the default token type is "int".
 */
#define YYSTYPE int

%}

%%
goal		:  initlex sect1 sect1end sect2 initforrule
			{ /* add default rule */
			int def_rule;

			gv->pat = cclinit(gv);
			cclnegate( gv, gv->pat );

			def_rule = mkstate( gv, -gv->pat );

			/* Remember the number of the default rule so we
			 * don't generate "can't match" warnings for it.
			 */
			gv->default_rule = gv->num_rules;

			finish_rule( gv, def_rule, false, 0, 0, 0);

			for ( gv->i_parse = 1; gv->i_parse <= gv->lastsc; ++gv->i_parse )
				gv->scset[gv->i_parse] = mkbranch( gv, gv->scset[gv->i_parse], def_rule );

			add_action(gv, "]]");

			if ( gv->ctrl.spprdflt )
				add_action(gv,
				"M4_HOOK_FATAL_ERROR(\"flex scanner jammed\")");
			else {
			    add_action(gv, "M4_HOOK_ECHO");
			}

			add_action( gv, "\n\tM4_HOOK_STATE_CASE_BREAK\n" );
			}
		;

initlex		:
			{ /* initialize for processing rules */

			/* Create default DFA start condition. */
			scinstal( gv, "INITIAL", false );
			}
		;

sect1		:  sect1 startconddecl namelist1
		|  sect1 options
		|
		|  error
			{ synerr(gv, _("unknown error processing section 1") ); }
		;

sect1end	:  SECTEND
			{
			check_options(gv);
			gv->scon_stk = allocate_integer_array( gv->lastsc + 1 );
			gv->scon_stk_ptr = 0;
			}
		;

startconddecl	:  SCDECL
			{ gv->sc_is_exclusive = false; }

		|  XSCDECL
			{ gv->sc_is_exclusive = true; }
		;

namelist1	:  namelist1 NAME
			{ scinstal( gv, gv->nmstr, gv->sc_is_exclusive ); }

		|  NAME
			{ scinstal( gv, gv->nmstr, gv->sc_is_exclusive ); }

		|  error
			{ synerr(gv, _("bad start condition list") ); }
		;

options		:  TOK_OPTION optionlist
		;

optionlist	:  optionlist option
		|
		;

option		:  TOK_OUTFILE '=' NAME
			{
			gv->env.outfilename = xstrdup(gv, gv->nmstr);
			gv->env.did_outfilename = 1;
			}
		|  TOK_EXTRA_TYPE '=' NAME
			{ gv->extra_type = xstrdup(gv, gv->nmstr); }
		|  TOK_PREFIX '=' NAME
			{ gv->ctrl.prefix = xstrdup(gv, gv->nmstr);
                          if (strchr(gv->ctrl.prefix, '[') || strchr(gv->ctrl.prefix, ']'))
                              flexerror(gv, _("Prefix must not contain [ or ]")); }
		|  TOK_YYCLASS '=' NAME
			{ gv->ctrl.yyclass = xstrdup(gv, gv->nmstr); }
		|  TOK_HEADER_FILE '=' NAME
			{ gv->env.headerfilename = xstrdup(gv, gv->nmstr); }
		|  TOK_YYLMAX '=' TOK_NUMERIC
			{ gv->ctrl.yylmax = gv->nmval; }
		|  TOK_YYDECL '=' NAME
			{ gv->ctrl.yydecl = xstrdup(gv, gv->nmstr); }
		|  TOK_PREACTION '=' NAME
			{ gv->ctrl.preaction = xstrdup(gv, gv->nmstr); }
		|  TOK_POSTACTION '=' NAME
			{ gv->ctrl.postaction = xstrdup(gv, gv->nmstr); }
		|  TOK_BUFSIZE '=' TOK_NUMERIC
			{ gv->ctrl.bufsize = gv->nmval; }
		|  TOK_EMIT '=' NAME
			{ gv->ctrl.emit = xstrdup(gv, gv->nmstr); backend_by_name(gv, gv->ctrl.emit); }
		|  TOK_USERINIT '=' NAME
			{ gv->ctrl.userinit = xstrdup(gv, gv->nmstr); }
		|  TOK_YYTERMINATE '=' NAME
			{ gv->ctrl.yyterminate = xstrdup(gv, gv->nmstr); }
		|  TOK_TABLES_FILE '=' NAME
        		{ gv->tablesext = true; gv->tablesfilename = xstrdup(gv, gv->nmstr); }
		;

sect2		:  sect2 scon initforrule flexrule '\n'
			{ gv->scon_stk_ptr = $2; }
		|  sect2 scon '{' sect2 '}'
			{ gv->scon_stk_ptr = $2; }
		|
		;

initforrule	:
			{
			/* Initialize for a parse of one rule. */
			gv->trlcontxt = gv->variable_trail_rule = gv->varlength = false;
			gv->trailcnt = gv->headcnt = gv->rulelen = 0;
			gv->current_state_type = STATE_NORMAL;
			gv->previous_continued_action = gv->continued_action;
			gv->in_rule = true;

			new_rule(gv);
			}
		;

flexrule	:  '^' rule
			{
			gv->pat = $2;
			finish_rule( gv, gv->pat, gv->variable_trail_rule,
				gv->headcnt, gv->trailcnt , gv->previous_continued_action);

			if ( gv->scon_stk_ptr > 0 )
				{
				for ( gv->i_parse = 1; gv->i_parse <= gv->scon_stk_ptr; ++gv->i_parse )
					gv->scbol[gv->scon_stk[gv->i_parse]] =
						mkbranch( gv, gv->scbol[gv->scon_stk[gv->i_parse]],
								gv->pat );
				}

			else
				{
				/* Add to all non-exclusive start conditions,
				 * including the default (0) start condition.
				 */

				for ( gv->i_parse = 1; gv->i_parse <= gv->lastsc; ++gv->i_parse )
					if ( ! gv->scxclu[gv->i_parse] )
						gv->scbol[gv->i_parse] = mkbranch( gv, gv->scbol[gv->i_parse],
									gv->pat );
				}

			if ( ! gv->bol_needed )
				{
				gv->bol_needed = true;

				if ( gv->env.performance_hint > 1 )
					pinpoint_message(gv,
			"'^' operator results in sub-optimal performance" );
				}
			}

		|  rule
			{
			gv->pat = $1;
			finish_rule( gv, gv->pat, gv->variable_trail_rule,
				gv->headcnt, gv->trailcnt , gv->previous_continued_action);

			if ( gv->scon_stk_ptr > 0 )
				{
				for ( gv->i_parse = 1; gv->i_parse <= gv->scon_stk_ptr; ++gv->i_parse )
					gv->scset[gv->scon_stk[gv->i_parse]] =
						mkbranch( gv, gv->scset[gv->scon_stk[gv->i_parse]],
								gv->pat );
				}

			else
				{
				for ( gv->i_parse = 1; gv->i_parse <= gv->lastsc; ++gv->i_parse )
					if ( ! gv->scxclu[gv->i_parse] )
						gv->scset[gv->i_parse] =
							mkbranch( gv, gv->scset[gv->i_parse],
								gv->pat );
				}
			}

		|  EOF_OP
			{
			if ( gv->scon_stk_ptr > 0 )
				build_eof_action(gv);
	
			else
				{
				/* This EOF applies to all start conditions
				 * which don't already have EOF actions.
				 */
				for ( gv->i_parse = 1; gv->i_parse <= gv->lastsc; ++gv->i_parse )
					if ( ! gv->sceof[gv->i_parse] )
						gv->scon_stk[++gv->scon_stk_ptr] = gv->i_parse;

				if ( gv->scon_stk_ptr == 0 )
					lwarn(gv,
			"all start conditions already have <<EOF>> rules" );

				else
					build_eof_action(gv);
				}
			}

		|  error
			{ synerr(gv, _("unrecognized rule") ); }
		;

scon_stk_ptr	:
			{ $$ = gv->scon_stk_ptr; }
		;

scon		:  '<' scon_stk_ptr namelist2 '>'
			{ $$ = $2; }

		|  '<' '*' '>'
			{
			$$ = gv->scon_stk_ptr;

			for ( gv->i_parse = 1; gv->i_parse <= gv->lastsc; ++gv->i_parse )
				{
				int j;

				for ( j = 1; j <= gv->scon_stk_ptr; ++j )
					if ( gv->scon_stk[j] == gv->i_parse )
						break;

				if ( j > gv->scon_stk_ptr )
					gv->scon_stk[++gv->scon_stk_ptr] = gv->i_parse;
				}
			}

		|
			{ $$ = gv->scon_stk_ptr; }
		;

namelist2	:  namelist2 ',' sconname

		|  sconname

		|  error
			{ synerr(gv, _("bad start condition list") ); }
		;

sconname	:  NAME
			{
			if ( (gv->scnum = sclookup( gv, gv->nmstr )) == 0 )
				format_pinpoint_message(gv,
					"undeclared start condition %s",
					gv->nmstr );
			else
				{
				for ( gv->i_parse = 1; gv->i_parse <= gv->scon_stk_ptr; ++gv->i_parse )
					if ( gv->scon_stk[gv->i_parse] == gv->scnum )
						{
						format_warn(gv,
							"<%s> specified twice",
							gv->scname[gv->scnum] );
						break;
						}

				if ( gv->i_parse > gv->scon_stk_ptr )
					gv->scon_stk[++gv->scon_stk_ptr] = gv->scnum;
				}
			}
		;

rule		:  re2 re
			{
			if ( gv->transchar[gv->lastst[$2]] != SYM_EPSILON )
				/* Provide final transition \now/ so it
				 * will be marked as a trailing context
				 * state.
				 */
				$2 = link_machines( gv, $2,
						mkstate( gv, SYM_EPSILON ) );

			mark_beginning_as_normal( gv, $2 );
			gv->current_state_type = STATE_NORMAL;

			if ( gv->previous_continued_action )
				{
				/* We need to treat this as variable trailing
				 * context so that the backup does not happen
				 * in the action but before the action switch
				 * statement.  If the backup happens in the
				 * action, then the rules "falling into" this
				 * one's action will *also* do the backup,
				 * erroneously.
				 */
				if ( ! gv->varlength || gv->headcnt != 0 )
					lwarn(gv,
		"trailing context made variable due to preceding '|' action" );

				/* Mark as variable. */
				gv->varlength = true;
				gv->headcnt = 0;

				}

			if ( gv->ctrl.lex_compat || (gv->varlength && gv->headcnt == 0) )
				{ /* variable trailing context rule */
				/* Mark the first part of the rule as the
				 * accepting "head" part of a trailing
				 * context rule.
				 *
				 * By the way, we didn't do this at the
				 * beginning of this production because back
				 * then current_state_type was set up for a
				 * trail rule, and add_accept() can create
				 * a new state ...
				 */
				add_accept( gv, $1,
					gv->num_rules | YY_TRAILING_HEAD_MASK );
				gv->variable_trail_rule = true;
				}
			
			else
				gv->trailcnt = gv->rulelen;

			$$ = link_machines( gv, $1, $2 );
			}

		|  re2 re '$'
			{ synerr(gv, _("trailing context used twice") ); }

		|  re '$'
			{
			gv->headcnt = 0;
			gv->trailcnt = 1;
			gv->rulelen = 1;
			gv->varlength = false;

			gv->current_state_type = STATE_TRAILING_CONTEXT;

			if ( gv->trlcontxt )
				{
				synerr(gv, _("trailing context used twice") );
				$$ = mkstate( gv, SYM_EPSILON );
				}

			else if ( gv->previous_continued_action )
				{
				/* See the comment in the rule for "re2 re"
				 * above.
				 */
				lwarn(gv,
		"trailing context made variable due to preceding '|' action" );

				gv->varlength = true;
				}

			if ( gv->ctrl.lex_compat || gv->varlength )
				{
				/* Again, see the comment in the rule for
				 * "re2 re" above.
				 */
				add_accept( gv, $1,
					gv->num_rules | YY_TRAILING_HEAD_MASK );
				gv->variable_trail_rule = true;
				}

			gv->trlcontxt = true;

			gv->eps = mkstate( gv, SYM_EPSILON );
			$$ = link_machines( gv, $1,
				link_machines( gv, gv->eps, mkstate( gv, '\n' ) ) );
			}

		|  re
			{
			$$ = $1;

			if ( gv->trlcontxt )
				{
				if ( gv->ctrl.lex_compat || (gv->varlength && gv->headcnt == 0) )
					/* Both head and trail are
					 * variable-length.
					 */
					gv->variable_trail_rule = true;
				else
					gv->trailcnt = gv->rulelen;
				}
			}
		;


re		:  re '|' series
			{
			gv->varlength = true;
			$$ = mkor( gv, $1, $3 );
			}

		|  series
			{ $$ = $1; }
		;


re2		:  re '/'
			{
			/* This rule is written separately so the
			 * reduction will occur before the trailing
			 * series is parsed.
			 */

			if ( gv->trlcontxt )
				synerr(gv, _("trailing context used twice") );
			else
				gv->trlcontxt = true;

			if ( gv->varlength )
				/* We hope the trailing context is
				 * fixed-length.
				 */
				gv->varlength = false;
			else
				gv->headcnt = gv->rulelen;

			gv->rulelen = 0;

			gv->current_state_type = STATE_TRAILING_CONTEXT;
			$$ = $1;
			}
		;

series		:  series singleton
			{
			/* This is where concatenation of adjacent patterns
			 * gets done.
			 */
			$$ = link_machines( gv, $1, $2 );
			}

		|  singleton
			{ $$ = $1; }

		|  series BEGIN_REPEAT_POSIX NUMBER ',' NUMBER END_REPEAT_POSIX
			{
			gv->varlength = true;

			if ( $3 > $5 || $3 < 0 )
				{
				synerr(gv, _("bad iteration values") );
				$$ = $1;
				}
			else
				{
				if ( $3 == 0 )
					{
					if ( $5 <= 0 )
						{
						synerr(gv,
						_("bad iteration values") );
						$$ = $1;
						}
					else
						$$ = mkopt(gv,
							mkrep( gv, $1, 1, $5 ) );
					}
				else
					$$ = mkrep( gv, $1, $3, $5 );
				}
			}

		|  series BEGIN_REPEAT_POSIX NUMBER ',' END_REPEAT_POSIX
			{
			gv->varlength = true;

			if ( $3 <= 0 )
				{
				synerr(gv, _("iteration value must be positive") );
				$$ = $1;
				}

			else
				$$ = mkrep( gv, $1, $3, INFINITE_REPEAT );
			}

		|  series BEGIN_REPEAT_POSIX NUMBER END_REPEAT_POSIX
			{
			/* The series could be something like "(foo)",
			 * in which case we have no idea what its length
			 * is, so we punt here.
			 */
			gv->varlength = true;

			if ( $3 <= 0 )
				{
				  synerr(gv, _("iteration value must be positive")
					  );
				$$ = $1;
				}

			else
				$$ = link_machines( gv, $1,
						copysingl( gv, $1, $3 - 1 ) );
			}

		;

singleton	:  singleton '*'
			{
			gv->varlength = true;

			$$ = mkclos( gv, $1 );
			}

		|  singleton '+'
			{
			gv->varlength = true;
			$$ = mkposcl( gv, $1 );
			}

		|  singleton '?'
			{
			gv->varlength = true;
			$$ = mkopt( gv, $1 );
			}

		|  singleton BEGIN_REPEAT_FLEX NUMBER ',' NUMBER END_REPEAT_FLEX
			{
			gv->varlength = true;

			if ( $3 > $5 || $3 < 0 )
				{
				synerr(gv, _("bad iteration values") );
				$$ = $1;
				}
			else
				{
				if ( $3 == 0 )
					{
					if ( $5 <= 0 )
						{
						synerr(gv,
						_("bad iteration values") );
						$$ = $1;
						}
					else
						$$ = mkopt(gv,
							mkrep( gv, $1, 1, $5 ) );
					}
				else
					$$ = mkrep( gv, $1, $3, $5 );
				}
			}

		|  singleton BEGIN_REPEAT_FLEX NUMBER ',' END_REPEAT_FLEX
			{
			gv->varlength = true;

			if ( $3 <= 0 )
				{
				synerr(gv, _("iteration value must be positive") );
				$$ = $1;
				}

			else
				$$ = mkrep( gv, $1, $3, INFINITE_REPEAT );
			}

		|  singleton BEGIN_REPEAT_FLEX NUMBER END_REPEAT_FLEX
			{
			/* The singleton could be something like "(foo)",
			 * in which case we have no idea what its length
			 * is, so we punt here.
			 */
			gv->varlength = true;

			if ( $3 <= 0 )
				{
				synerr(gv, _("iteration value must be positive") );
				$$ = $1;
				}

			else
				$$ = link_machines( gv, $1,
						copysingl( gv, $1, $3 - 1 ) );
			}

		|  '.'
			{
			if ( ! gv->madeany )
				{
				/* Create the '.' character class. */
                    gv->ccldot = cclinit(gv);
                    ccladd( gv, gv->ccldot, '\n' );
                    cclnegate( gv, gv->ccldot );

                    if ( gv->ctrl.useecs )
                        mkeccl( gv, gv->ccltbl + gv->cclmap[gv->ccldot],
                            gv->ccllen[gv->ccldot], gv->nextecm,
                            gv->ecgroup, gv->ctrl.csize, gv->ctrl.csize );

				/* Create the (?s:'.') character class. */
                    gv->cclany = cclinit(gv);
                    cclnegate( gv, gv->cclany );

                    if ( gv->ctrl.useecs )
                        mkeccl( gv, gv->ccltbl + gv->cclmap[gv->cclany],
                            gv->ccllen[gv->cclany],gv-> nextecm,
                            gv->ecgroup, gv->ctrl.csize, gv->ctrl.csize );

				gv->madeany = true;
				}

			++gv->rulelen;

            if (sf_dot_all())
                $$ = mkstate( gv, -gv->cclany );
            else
                $$ = mkstate( gv, -gv->ccldot );
			}

		|  fullccl
			{
				/* Sort characters for fast searching.
				 */
				qsort( gv->ccltbl + gv->cclmap[$1], (size_t) gv->ccllen[$1], sizeof (*gv->ccltbl), cclcmp );

			if ( gv->ctrl.useecs )
				mkeccl( gv, gv->ccltbl + gv->cclmap[$1], gv->ccllen[$1],
					gv->nextecm, gv->ecgroup, gv->ctrl.csize, gv->ctrl.csize);

			++gv->rulelen;

			if (gv->ccl_has_nl[$1])
				gv->rule_has_nl[gv->num_rules] = true;

			$$ = mkstate( gv, -$1 );
			}

		|  PREVCCL
			{
			++gv->rulelen;

			if (gv->ccl_has_nl[$1])
				gv->rule_has_nl[gv->num_rules] = true;

			$$ = mkstate( gv, -$1 );
			}

		|  '"' string '"'
			{ $$ = $2; }

		|  '(' re ')'
			{ $$ = $2; }

		|  CHAR
			{
			++gv->rulelen;

			if ($1 == gv->nlch)
				gv->rule_has_nl[gv->num_rules] = true;

            if (sf_case_ins() && has_case($1))
                /* create an alternation, as in (a|A) */
                $$ = mkor (gv, mkstate(gv, $1), mkstate(gv, reverse_case($1)));
            else
                $$ = mkstate( gv, $1 );
			}
		;
fullccl:
        fullccl CCL_OP_DIFF  braceccl  { $$ = ccl_set_diff  (gv, $1, $3); }
    |   fullccl CCL_OP_UNION braceccl  { $$ = ccl_set_union (gv, $1, $3); }
    |   braceccl
    ;

braceccl: 

            '[' ccl ']' { $$ = $2; }

		|  '[' '^' ccl ']'
			{
			cclnegate( gv, $3 );
			$$ = $3;
			}
		;

ccl		:  ccl CHAR '-' CHAR
			{

			if (sf_case_ins())
			  {

			    /* If one end of the range has case and the other
			     * does not, or the cases are different, then we're not
			     * sure what range the user is trying to express.
			     * Examples: [@-z] or [S-t]
			     */
			    if (has_case ($2) != has_case ($4)
				     || (has_case ($2) && (b_islower ($2) != b_islower ($4)))
				     || (has_case ($2) && (b_isupper ($2) != b_isupper ($4))))
			      format_warn3 (
			      _("the character range [%c-%c] is ambiguous in a case-insensitive scanner"),
					    $2, $4);

			    /* If the range spans uppercase characters but not
			     * lowercase (or vice-versa), then should we automatically
			     * include lowercase characters in the range?
			     * Example: [@-_] spans [a-z] but not [A-Z]
			     */
			    else if (!has_case ($2) && !has_case ($4) && !range_covers_case ($2, $4))
			      format_warn3 (
			      _("the character range [%c-%c] is ambiguous in a case-insensitive scanner"),
					    $2, $4);
			  }

			if ( $2 > $4 )
				synerr(gv, _("negative range in character class") );

			else
				{
				for ( gv->i_parse = $2; gv->i_parse <= $4; ++gv->i_parse )
					ccladd( gv, $1, gv->i_parse );

				/* Keep track if this ccl is staying in
				 * alphabetical order.
				 */
				gv->cclsorted = gv->cclsorted && ($2 > gv->lastchar);
				gv->lastchar = $4;

                /* Do it again for upper/lowercase */
                if (sf_case_ins() && has_case($2) && has_case($4)){
                    $2 = reverse_case ($2);
                    $4 = reverse_case ($4);
                    
                    for ( gv->i_parse = $2; gv->i_parse <= $4; ++gv->i_parse )
                        ccladd( gv, $1, gv->i_parse );

                    gv->cclsorted = gv->cclsorted && ($2 > gv->lastchar);
                    gv->lastchar = $4;
                }

				}

			$$ = $1;
			}

		|  ccl CHAR
			{
			ccladd( gv, $1, $2 );
			gv->cclsorted = gv->cclsorted && ($2 > gv->lastchar);
			gv->lastchar = $2;

            /* Do it again for upper/lowercase */
            if (sf_case_ins() && has_case($2)){
                $2 = reverse_case ($2);
                ccladd (gv, $1, $2);

                gv->cclsorted = gv->cclsorted && ($2 > gv->lastchar);
                gv->lastchar = $2;
            }

			$$ = $1;
			}

		|  ccl ccl_expr
			{
			/* Too hard to properly maintain gv->cclsorted. */
			gv->cclsorted = false;
			$$ = $1;
			}

		|
			{
			gv->cclsorted = true;
			gv->lastchar = 0;
			gv->currccl = $$ = cclinit(gv);
			}
		;

ccl_expr:	   
           CCE_ALNUM	{ CCL_EXPR(isalnum); }
		|  CCE_ALPHA	{ CCL_EXPR(isalpha); }
		|  CCE_BLANK	{ CCL_EXPR(IS_BLANK); }
		|  CCE_CNTRL	{ CCL_EXPR(iscntrl); }
		|  CCE_DIGIT	{ CCL_EXPR(isdigit); }
		|  CCE_GRAPH	{ CCL_EXPR(isgraph); }
		|  CCE_LOWER	{ 
                          CCL_EXPR(islower);
                          if (sf_case_ins())
                              CCL_EXPR(isupper);
                        }
		|  CCE_PRINT	{ CCL_EXPR(isprint); }
		|  CCE_PUNCT	{ CCL_EXPR(ispunct); }
		|  CCE_SPACE	{ CCL_EXPR(isspace); }
		|  CCE_XDIGIT	{ CCL_EXPR(isxdigit); }
		|  CCE_UPPER	{
                    CCL_EXPR(isupper);
                    if (sf_case_ins())
                        CCL_EXPR(islower);
				}

        |  CCE_NEG_ALNUM	{ CCL_NEG_EXPR(isalnum); }
		|  CCE_NEG_ALPHA	{ CCL_NEG_EXPR(isalpha); }
		|  CCE_NEG_BLANK	{ CCL_NEG_EXPR(IS_BLANK); }
		|  CCE_NEG_CNTRL	{ CCL_NEG_EXPR(iscntrl); }
		|  CCE_NEG_DIGIT	{ CCL_NEG_EXPR(isdigit); }
		|  CCE_NEG_GRAPH	{ CCL_NEG_EXPR(isgraph); }
		|  CCE_NEG_PRINT	{ CCL_NEG_EXPR(isprint); }
		|  CCE_NEG_PUNCT	{ CCL_NEG_EXPR(ispunct); }
		|  CCE_NEG_SPACE	{ CCL_NEG_EXPR(isspace); }
		|  CCE_NEG_XDIGIT	{ CCL_NEG_EXPR(isxdigit); }
		|  CCE_NEG_LOWER	{ 
				if ( sf_case_ins() )
					lwarn(gv, _("[:^lower:] is ambiguous in case insensitive scanner"));
				else
					CCL_NEG_EXPR(islower);
				}
		|  CCE_NEG_UPPER	{
				if ( sf_case_ins() )
					lwarn(gv, _("[:^upper:] ambiguous in case insensitive scanner"));
				else
					CCL_NEG_EXPR(isupper);
				}
		;
		
string		:  string CHAR
			{
			if ( $2 == gv->nlch )
				gv->rule_has_nl[gv->num_rules] = true;

			++gv->rulelen;

            if (sf_case_ins() && has_case($2))
                $$ = mkor (gv, mkstate(gv, $2), mkstate(gv, reverse_case($2)));
            else
                $$ = mkstate (gv, $2);

			$$ = link_machines( gv, $1, $$);
			}

		|
			{ $$ = mkstate( gv, SYM_EPSILON ); }
		;

%%


/* build_eof_action - build the "<<EOF>>" action for the active start
 *                    conditions
 */

void build_eof_action(FlexState* gv)
	{
	int i;
	char action_text[MAXLINE];

	for ( i = 1; i <= gv->scon_stk_ptr; ++i )
		{
		if ( gv->sceof[gv->scon_stk[i]] )
			format_pinpoint_message(gv,
				"multiple <<EOF>> rules for start condition %s",
				gv->scname[gv->scon_stk[i]] );

		else
			{
			gv->sceof[gv->scon_stk[i]] = true;

			if (gv->previous_continued_action /* && previous action was regular */)
				add_action(gv, "YY_RULE_SETUP\n");

			snprintf( action_text, sizeof(action_text), "M4_HOOK_EOF_STATE_CASE_ARM(%s)\n",
				gv->scname[gv->scon_stk[i]] );
			add_action( gv, action_text );
			}
		}

	line_directive_out(gv, NULL, gv->infilename, gv->linenum);
        add_action(gv, "[[");

	/* This isn't a normal rule after all - don't count it as
	 * such, so we don't have any holes in the rule numbering
	 * (which make generating "rule can never match" warnings
	 * more difficult.
	 */
	--gv->num_rules;
	++gv->num_eof_rules;
	}


/* format_synerr - write out formatted syntax error */

void format_synerr( FlexState* gv, const char *msg, const char arg[] )
	{
	char errmsg[MAXLINE];

	(void) snprintf( errmsg, sizeof(errmsg), msg, arg );
	synerr( gv, errmsg );
	}


/* synerr - report a syntax error */

void synerr( FlexState* gv, const char *str )
	{
	gv->syntaxerror = true;
	pinpoint_message( gv, str );
	}


/* format_warn - write out formatted warning */

void format_warn( FlexState* gv, const char *msg, const char arg[] )
	{
	char warn_msg[MAXLINE];

	snprintf( warn_msg, sizeof(warn_msg), msg, arg );
	lwarn( gv, warn_msg );
	}


/* lwarn - report a warning, unless -w was given */

void lwarn( FlexState* gv, const char *str )
	{
	line_warning( gv, str, gv->linenum );
	}

/* format_pinpoint_message - write out a message formatted with one string,
 *			     pinpointing its location
 */

void format_pinpoint_message( FlexState* gv,  const char *msg, const char arg[] )
	{
	char errmsg[MAXLINE*2];

	snprintf( errmsg, sizeof(errmsg), msg, arg );
	pinpoint_message( gv, errmsg );
	}


/* pinpoint_message - write out a message, pinpointing its location */

void pinpoint_message( FlexState* gv, const char *str )
	{
	line_pinpoint( gv, str, gv->linenum );
	}


/* line_warning - report a warning at a given line, unless -w was given */

void line_warning( FlexState* gv, const char *str, int line )
	{
	char warning[MAXLINE*2];

	if ( ! gv->env.nowarn )
		{
		snprintf( warning, sizeof(warning), "warning, %s", str );
		line_pinpoint( gv, warning, line );
		}
	}


/* line_pinpoint - write out a message, pinpointing it at the given line */

void line_pinpoint( FlexState* gv, const char *str, int line )
	{
	fprintf( stderr, "%s:%d: %s\n", gv->infilename, line, str );
	}


/* yyerror - eat up an error message from the parser;
 *	     currently, messages are ignore
 */

void yyerror( yyscan_t yyscanner,  const char *msg )
	{
		(void)yyscanner;(void)msg;
	}
