/* flex - tool to generate fast lexical analyzers */

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

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1990 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#include "flexdef.h"
#include "version.h"
#include "options.h"

static char flex_version[] = FLEX_VERSION;


/* declare functions that have forward references */

void flexinit PROTO((int, char**));
void readin PROTO((void));
void set_up_initial_allocations PROTO((void));
static char * basename2 PROTO((char* path, int should_strip_ext));


/* these globals are all defined and commented in flexdef.h */
int printstats, syntaxerror, eofseen, ddebug, trace, nowarn, spprdflt;
int interactive, caseins, lex_compat, do_yylineno, useecs, fulltbl, usemecs;
int fullspd, gen_line_dirs, performance_report, backing_up_report;
int C_plus_plus, long_align, use_read, yytext_is_array, do_yywrap, csize;
int reentrant, reentrant_bison_pure;
int yymore_used, reject, real_reject, continued_action, in_rule;
int yymore_really_used, reject_really_used;
int datapos, dataline, linenum, out_linenum;
FILE *skelfile = NULL;
int skel_ind = 0;
char *action_array;
int action_size, defs1_offset, prolog_offset, action_offset, action_index;
char *infilename = NULL, *outfilename = NULL, *headerfilename = NULL;
int did_outfilename;
char *prefix, *yyclass;
int do_stdinit, use_stdout;
int onestate[ONE_STACK_SIZE], onesym[ONE_STACK_SIZE];
int onenext[ONE_STACK_SIZE], onedef[ONE_STACK_SIZE], onesp;
int maximum_mns, current_mns, current_max_rules;
int num_rules, num_eof_rules, default_rule, lastnfa;
int *firstst, *lastst, *finalst, *transchar, *trans1, *trans2;
int *accptnum, *assoc_rule, *state_type;
int *rule_type, *rule_linenum, *rule_useful;
int current_state_type;
int variable_trailing_context_rules;
int numtemps, numprots, protprev[MSP], protnext[MSP], prottbl[MSP];
int protcomst[MSP], firstprot, lastprot, protsave[PROT_SAVE_SIZE];
int numecs, nextecm[CSIZE + 1], ecgroup[CSIZE + 1], nummecs, tecfwd[CSIZE + 1];
int tecbck[CSIZE + 1];
int lastsc, *scset, *scbol, *scxclu, *sceof;
int current_max_scs;
char **scname;
int current_max_dfa_size, current_max_xpairs;
int current_max_template_xpairs, current_max_dfas;
int lastdfa, *nxt, *chk, *tnxt;
int *base, *def, *nultrans, NUL_ec, tblend, firstfree, **dss, *dfasiz;
union dfaacc_union *dfaacc;
int *accsiz, *dhash, numas;
int numsnpairs, jambase, jamstate;
int lastccl, *cclmap, *ccllen, *cclng, cclreuse;
int current_maxccls, current_max_ccl_tbl_size;
Char *ccltbl;
char nmstr[MAXLINE];
int sectnum, nummt, hshcol, dfaeql, numeps, eps2, num_reallocs;
int tmpuses, totnst, peakpairs, numuniq, numdup, hshsave;
int num_backing_up, bol_needed;
FILE *backing_up_file;
int end_of_buffer_state;
char **input_files;
int num_input_files;

/* Make sure program_name is initialized so we don't crash if writing
 * out an error message before getting the program name from argv[0].
 */
char *program_name = "flex";

#ifndef SHORT_FILE_NAMES
static char *outfile_template = "lex.%s.%s";
static char *backing_name = "lex.backup";
#else
static char *outfile_template = "lex%s.%s";
static char *backing_name = "lex.bck";
#endif

#ifdef THINK_C
#include <console.h>
#endif

#ifdef MS_DOS
extern unsigned _stklen = 16384;
#endif

static char outfile_path[MAXLINE];
static int outfile_created = 0;
static char *skelname = NULL;


int main( argc, argv )
int argc;
char **argv;
	{
	int i;

	setlocale(LC_MESSAGES, "");
	textdomain(PACKAGE);
	bindtextdomain(PACKAGE, LOCALEDIR);

#ifdef THINK_C
	argc = ccommand( &argv );
#endif

	flexinit( argc, argv );

	readin();

	ntod();

	for ( i = 1; i <= num_rules; ++i )
		if ( ! rule_useful[i] && i != default_rule )
			line_warning( _( "rule cannot be matched" ),
					rule_linenum[i] );

	if ( spprdflt && ! reject && rule_useful[default_rule] )
		line_warning(
			_( "-s option given but default rule can be matched" ),
			rule_linenum[default_rule] );

	/* Generate the C state transition tables from the DFA. */
	make_tables();

	/* Note, flexend does not return.  It exits with its argument
	 * as status.
	 */
	flexend( 0 );

	return 0;	/* keep compilers/lint happy */
	}


/* check_options - check user-specified options */

void check_options()
	{
	int i;

	if ( lex_compat )
		{
		if ( C_plus_plus )
			flexerror( _( "Can't use -+ with -l option" ) );

		if ( fulltbl || fullspd )
			flexerror( _( "Can't use -f or -F with -l option" ) );

        if( reentrant || reentrant_bison_pure )
            flexerror( _( "Can't use -R or -Rb with -l option" ) );

		/* Don't rely on detecting use of yymore() and REJECT,
		 * just assume they'll be used.
		 */
		yymore_really_used = reject_really_used = true;

		yytext_is_array = true;
		do_yylineno = true;
		use_read = false;
		}

	/* See comments in flexend() for an explanation of this error condition. */
	if ( use_stdout && headerfilename )
	    flexerror( _( "Can't specify header option if writing to stdout.") );

	if ( do_yylineno )
		/* This should really be "maintain_backup_tables = true" */
		reject_really_used = true;

	if ( csize == unspecified )
		{
		if ( (fulltbl || fullspd) && ! useecs )
			csize = DEFAULT_CSIZE;
		else
			csize = CSIZE;
		}

	if ( interactive == unspecified )
		{
		if ( fulltbl || fullspd )
			interactive = false;
		else
			interactive = true;
		}

	if ( fulltbl || fullspd )
		{
		if ( usemecs )
			flexerror(
			_( "-Cf/-CF and -Cm don't make sense together" ) );

		if ( interactive )
			flexerror( _( "-Cf/-CF and -I are incompatible" ) );

		if ( lex_compat )
			flexerror(
		_( "-Cf/-CF are incompatible with lex-compatibility mode" ) );

		if ( do_yylineno )
			flexerror(
			_( "-Cf/-CF and %option yylineno are incompatible" ) );

		if ( fulltbl && fullspd )
			flexerror( _( "-Cf and -CF are mutually exclusive" ) );
		}

	if ( C_plus_plus && fullspd )
		flexerror( _( "Can't use -+ with -CF option" ) );

	if ( C_plus_plus && yytext_is_array )
		{
		warn( _( "%array incompatible with -+ option" ) );
		yytext_is_array = false;
		}

    if ( C_plus_plus && (reentrant || reentrant_bison_pure) )
        flexerror( _( "Options -+ and -R are mutually exclusive." ) );


	if ( useecs )
		{ /* Set up doubly-linked equivalence classes. */

		/* We loop all the way up to csize, since ecgroup[csize] is
		 * the position used for NUL characters.
		 */
		ecgroup[1] = NIL;

		for ( i = 2; i <= csize; ++i )
			{
			ecgroup[i] = i - 1;
			nextecm[i - 1] = i;
			}

		nextecm[csize] = NIL;
		}

	else
		{
		/* Put everything in its own equivalence class. */
		for ( i = 1; i <= csize; ++i )
			{
			ecgroup[i] = i;
			nextecm[i] = BAD_SUBSCRIPT;	/* to catch errors */
			}
		}

	if ( ! use_stdout )
		{
		FILE *prev_stdout;

		if ( ! did_outfilename )
			{
			char *suffix;

			if ( C_plus_plus )
				suffix = "cc";
			else
				suffix = "c";

			sprintf( outfile_path, outfile_template,
				prefix, suffix );

			outfilename = outfile_path;
			}

		prev_stdout = freopen( outfilename, "w+", stdout );

		if ( prev_stdout == NULL )
			lerrsf( _( "could not create %s" ), outfilename );

		outfile_created = 1;
		}

	if ( skelname && (skelfile = fopen( skelname, "r" )) == NULL )
		lerrsf( _( "can't open skeleton file %s" ), skelname );

    if ( reentrant )
        {
            outn("#define YY_REENTRANT 1");
            if( yytext_is_array )
                outn("#define YY_TEXT_IS_ARRAY");
        }

    if ( reentrant_bison_pure )
            outn("#define YY_REENTRANT_BISON_PURE 1");

	if ( strcmp( prefix, "yy" ) )
		{
#define GEN_PREFIX(name) out_str3( "#define yy%s %s%s\n", name, prefix, name )
		if ( C_plus_plus )
			GEN_PREFIX( "FlexLexer" );
		else
			{
            outn( "#ifndef YY_REENTRANT" );
                GEN_PREFIX( "text" );
                GEN_PREFIX( "leng" );
                GEN_PREFIX( "in" );
                GEN_PREFIX( "out" );
            outn( "#endif" );
			GEN_PREFIX( "_create_buffer" );
			GEN_PREFIX( "_delete_buffer" );
			GEN_PREFIX( "_scan_buffer" );
			GEN_PREFIX( "_scan_string" );
			GEN_PREFIX( "_scan_bytes" );
			GEN_PREFIX( "_flex_debug" );
			GEN_PREFIX( "_init_buffer" );
			GEN_PREFIX( "_flush_buffer" );
			GEN_PREFIX( "_load_buffer_state" );
			GEN_PREFIX( "_switch_to_buffer" );
			GEN_PREFIX( "lex" );
			GEN_PREFIX( "restart" );
            GEN_PREFIX( "lex_init" );
            GEN_PREFIX( "lex_destroy" );
            GEN_PREFIX( "get_extra" );
            GEN_PREFIX( "set_extra" );
            GEN_PREFIX( "get_in" );
            GEN_PREFIX( "set_in" );
            GEN_PREFIX( "get_out" );
            GEN_PREFIX( "set_out" );
            GEN_PREFIX( "get_leng" );
            GEN_PREFIX( "get_text" );
            GEN_PREFIX( "get_lineno" );
            GEN_PREFIX( "set_lineno" );

            outn( "#ifdef YY_REENTRANT_BISON_PURE" );
            GEN_PREFIX( "get_lval" );
            GEN_PREFIX( "set_lval" );
            outn( "#ifdef YYLTYPE" );
            GEN_PREFIX( "get_lloc" );
            GEN_PREFIX( "set_lloc" );
            outn( "#endif" );
            outn( "#endif" );

            if ( do_yylineno && reentrant)
                outn ( "#define YY_USE_LINENO 1");
			}

            if ( do_yylineno && !reentrant )
                GEN_PREFIX( "lineno" );

		if ( do_yywrap )
			GEN_PREFIX( "wrap" );

		outn( "" );
		}

	if ( did_outfilename )
		line_directive_out( stdout, 0 );

        /* Dump the user defined preproc directives. */
        if (userdef_buf.elts)
            outn( (char*)(userdef_buf.elts) );

	skelout();
    /* %% [1.0] */
	}

/* Alter #line directives from the generated source, destined
 * for the generated header. We chaneg the line number and filename.
 * linebuf is modified in place.
 */
void fix_line_dirs(linebuf, outfilename, headerfilename, nlines)
char * linebuf;
char * outfilename;
char * headerfilename;
int nlines;
{
    char *pname, *p;
    /* Match pattern:  ^#line +[:digit:]+ +"outfilename" */
    p = linebuf;
    if (strncmp(p, "#line ", 6))
        return;
    p += 6;
    
    /* match spaces */
    while (*p == ' ')
        p++;    
    if (!isdigit(*p))
        return;

    /* match numbers */
    while (isdigit(*p))
        p++;
    if (*p != ' ')
        return;

    /* match spaces */
    while (*p == ' ')
        p++;
    if (*p != '"')
        return;
    p++;

    pname = p;
    
    /* find end of filename. Note: If scanner filename has
     * embedded '"' chars, then the generated #line directive
     * may fail. */
    while (*p != '\0' && *p != '"')
        p++;
    if (*p != '"')
        return;

    if (strncmp(pname, outfilename, p-pname) != 0)
        return;

    /* We have a match. */
    
    sprintf(linebuf,"#line %d \"%s\"\n", nlines+2, headerfilename);
}

/* flexend - terminate flex
 *
 * note
 *    This routine does not return.
 */

void flexend( exit_status )
int exit_status;

	{
	static int called_before = -1; /* prevent infinite recursion. */
	int tblsiz;	
	int i;
	int unlink();

	if( ++called_before )
		exit( exit_status );

	if ( skelfile != NULL )
		{
		if ( ferror( skelfile ) )
			lerrsf( _( "input error reading skeleton file %s" ),
				skelname );

		else if ( fclose( skelfile ) )
			lerrsf( _( "error closing skeleton file %s" ),
				skelname );
		}

	/* flex generates the header file by rewinding the output FILE
	 * pointer. However, since we can't rewind stdout, we must disallow
	 * %option header if we are writing to stdout. This is a kludge. 
	 * This kludge can be rewritten when we get around to buffering
	 * Section 1 of the input file, because then we'll have seen all the
	 * %options BEFORE we begin generating the scanner. The lack of
	 * buffering causes other problems, too. For example, it is the
	 * reason we currently can't provide a mechanism to allow the user
	 * to inject arbitrary class members into the generated C++ scanner. - JM
	 */
	if ( headerfilename && exit_status == 0 && outfile_created && !ferror(stdout))
		{
			/* Copy the file we just wrote to a header file. */
			#define LINE_SZ 512
			FILE *header_out;
			char linebuf[LINE_SZ];
			int nlines=0;

			/* rewind the outfile file. */
			fflush(stdout);
			fseek(stdout, 0L, SEEK_SET);

			header_out = fopen(headerfilename, "w");
			if ( header_out == NULL)
				lerrsf( _( "could not create %s"), headerfilename );

			fprintf(header_out,
					"#ifndef %sHEADER_H\n"
					"#define %sHEADER_H 1\n"
					"#define %sIN_HEADER 1\n\n",
					prefix,prefix,prefix);
			fflush(header_out);

            nlines=4;
			while(fgets(linebuf, LINE_SZ, stdout)) {
                fix_line_dirs(linebuf, outfilename, headerfilename, nlines);
				fputs(linebuf, header_out);
                nlines++;
            }

            /* Kill ALL flex-related macros. This is so the user
             * can #include more than one generated header file. */
            fprintf(header_out,"\n");
            fprintf(header_out,"#undef BEGIN\n");
            fprintf(header_out,"#undef ECHO\n");
            fprintf(header_out,"#undef EOB_ACT_CONTINUE_SCAN\n");
            fprintf(header_out,"#undef EOB_ACT_END_OF_FILE\n");
            fprintf(header_out,"#undef EOB_ACT_LAST_MATCH\n");
            fprintf(header_out,"#undef FLEX_SCANNER\n");
            fprintf(header_out,"#undef FLEX_STD\n");
            fprintf(header_out,"#undef INITIAL\n");
            fprintf(header_out,"#undef REJECT\n");
            fprintf(header_out,"#undef YYLMAX\n");
            fprintf(header_out,"#undef YYSTATE\n");
            fprintf(header_out,"#undef YY_AT_BOL\n");
            fprintf(header_out,"#undef YY_BREAK\n");
            fprintf(header_out,"#undef YY_BUFFER_EOF_PENDING\n");
            fprintf(header_out,"#undef YY_BUFFER_NEW\n");
            fprintf(header_out,"#undef YY_BUFFER_NORMAL\n");
            fprintf(header_out,"#undef YY_BUF_SIZE\n");
            fprintf(header_out,"#undef YY_CALL_LAST_ARG\n");
            fprintf(header_out,"#undef YY_CALL_ONLY_ARG\n");
            fprintf(header_out,"#undef YY_CURRENT_BUFFER\n");
            fprintf(header_out,"#undef YY_DECL\n");
            fprintf(header_out,"#undef YY_DECL_LAST_ARG\n");
            fprintf(header_out,"#undef YY_DO_BEFORE_ACTION\n");
            fprintf(header_out,"#undef YY_END_OF_BUFFER\n");
            fprintf(header_out,"#undef YY_END_OF_BUFFER_CHAR\n");
            fprintf(header_out,"#undef YY_EXIT_FAILURE\n");
            fprintf(header_out,"#undef YY_EXTRA_TYPE\n");
            fprintf(header_out,"#undef YY_FATAL_ERROR\n");
            fprintf(header_out,"#undef YY_FLEX_DEFINED_ECHO\n");
            fprintf(header_out,"#undef YY_FLEX_LEX_COMPAT\n");
            fprintf(header_out,"#undef YY_FLEX_MAJOR_VERSION\n");
            fprintf(header_out,"#undef YY_FLEX_MINOR_VERSION\n");
            fprintf(header_out,"#undef YY_FLUSH_BUFFER\n");
            fprintf(header_out,"#undef YY_G\n");
            fprintf(header_out,"#undef YY_INPUT\n");
            fprintf(header_out,"#undef YY_INTERACTIVE\n");
            fprintf(header_out,"#undef YY_LAST_ARG\n");
            fprintf(header_out,"#undef YY_LEX_ARGS\n");
            fprintf(header_out,"#undef YY_MAIN\n");
            fprintf(header_out,"#undef YY_MORE_ADJ\n");
            fprintf(header_out,"#undef YY_NEED_STRLEN\n");
            fprintf(header_out,"#undef YY_NEW_FILE\n");
            fprintf(header_out,"#undef YY_NO_GET_EXTRA\n");
            fprintf(header_out,"#undef YY_NO_GET_IN\n");
            fprintf(header_out,"#undef YY_NO_GET_LENG\n");
            fprintf(header_out,"#undef YY_NO_GET_LINENO\n");
            fprintf(header_out,"#undef YY_NO_GET_LLOC\n");
            fprintf(header_out,"#undef YY_NO_GET_LVAL\n");
            fprintf(header_out,"#undef YY_NO_GET_OUT\n");
            fprintf(header_out,"#undef YY_NO_GET_TEXT\n");
            fprintf(header_out,"#undef YY_NO_INPUT\n");
            fprintf(header_out,"#undef YY_NO_POP_STATE\n");
            fprintf(header_out,"#undef YY_NO_PUSH_STATE\n");
            fprintf(header_out,"#undef YY_NO_SCAN_BUFFER\n");
            fprintf(header_out,"#undef YY_NO_SCAN_BYTES\n");
            fprintf(header_out,"#undef YY_NO_SCAN_STRING\n");
            fprintf(header_out,"#undef YY_NO_SET_EXTRA\n");
            fprintf(header_out,"#undef YY_NO_SET_IN\n");
            fprintf(header_out,"#undef YY_NO_SET_LINENO\n");
            fprintf(header_out,"#undef YY_NO_SET_LLOC\n");
            fprintf(header_out,"#undef YY_NO_SET_LVAL\n");
            fprintf(header_out,"#undef YY_NO_SET_OUT\n");
            fprintf(header_out,"#undef YY_NO_TOP_STATE\n");
            fprintf(header_out,"#undef YY_NO_UNISTD_H\n");
            fprintf(header_out,"#undef YY_NO_UNPUT\n");
            fprintf(header_out,"#undef YY_NULL\n");
            fprintf(header_out,"#undef YY_NUM_RULES\n");
            fprintf(header_out,"#undef YY_ONLY_ARG\n");
            fprintf(header_out,"#undef YY_PROTO\n");
            fprintf(header_out,"#undef YY_READ_BUF_SIZE\n");
            fprintf(header_out,"#undef YY_REENTRANT\n");
            fprintf(header_out,"#undef YY_REENTRANT_BISON_PURE\n");
            fprintf(header_out,"#undef YY_RESTORE_YY_MORE_OFFSET\n");
            fprintf(header_out,"#undef YY_RULE_SETUP\n");
            fprintf(header_out,"#undef YY_SC_TO_UI\n");
            fprintf(header_out,"#undef YY_SKIP_YYWRAP\n");
            fprintf(header_out,"#undef YY_STACK_USED\n");
            fprintf(header_out,"#undef YY_START\n");
            fprintf(header_out,"#undef YY_START_STACK_INCR\n");
            fprintf(header_out,"#undef YY_STATE_EOF\n");
            fprintf(header_out,"#undef YY_STDINIT\n");
            fprintf(header_out,"#undef YY_TEXT_IS_ARRAY\n");
            fprintf(header_out,"#undef YY_TRAILING_HEAD_MASK\n");
            fprintf(header_out,"#undef YY_TRAILING_MASK\n");
            fprintf(header_out,"#undef YY_USER_ACTION\n");
            fprintf(header_out,"#undef YY_USES_REJECT\n");
            fprintf(header_out,"#undef YY_USE_CONST\n");
            fprintf(header_out,"#undef YY_USE_LINENO\n");
            fprintf(header_out,"#undef YY_USE_PROTOS\n");
            fprintf(header_out,"#undef unput\n");
            fprintf(header_out,"#undef yy_create_buffer\n");
            fprintf(header_out,"#undef yy_delete_buffer\n");
            fprintf(header_out,"#undef yy_flex_debug\n");
            fprintf(header_out,"#undef yy_flush_buffer\n");
            fprintf(header_out,"#undef yy_init_buffer\n");
            fprintf(header_out,"#undef yy_load_buffer_state\n");
            fprintf(header_out,"#undef yy_new_buffer\n");
            fprintf(header_out,"#undef yy_scan_buffer\n");
            fprintf(header_out,"#undef yy_scan_bytes\n");
            fprintf(header_out,"#undef yy_scan_string\n");
            fprintf(header_out,"#undef yy_set_bol\n");
            fprintf(header_out,"#undef yy_set_interactive\n");
            fprintf(header_out,"#undef yy_switch_to_buffer\n");
            fprintf(header_out,"#undef yyconst\n");
            fprintf(header_out,"#undef yyextra\n");
            fprintf(header_out,"#undef yyget_extra\n");
            fprintf(header_out,"#undef yyget_in\n");
            fprintf(header_out,"#undef yyget_leng\n");
            fprintf(header_out,"#undef yyget_lineno\n");
            fprintf(header_out,"#undef yyget_lloc\n");
            fprintf(header_out,"#undef yyget_lval\n");
            fprintf(header_out,"#undef yyget_out\n");
            fprintf(header_out,"#undef yyget_text\n");
            fprintf(header_out,"#undef yyin\n");
            fprintf(header_out,"#undef yyleng\n");
            fprintf(header_out,"#undef yyless\n");
            fprintf(header_out,"#undef yylex\n");
            fprintf(header_out,"#undef yylex_destroy\n");
            fprintf(header_out,"#undef yylex_init\n");
            fprintf(header_out,"#undef yylineno\n");
            fprintf(header_out,"#undef yylloc\n");
            fprintf(header_out,"#undef yylval\n");
            fprintf(header_out,"#undef yymore\n");
            fprintf(header_out,"#undef yyout\n");
            fprintf(header_out,"#undef yyrestart\n");
            fprintf(header_out,"#undef yyset_extra\n");
            fprintf(header_out,"#undef yyset_in\n");
            fprintf(header_out,"#undef yyset_lineno\n");
            fprintf(header_out,"#undef yyset_lloc\n");
            fprintf(header_out,"#undef yyset_lval\n");
            fprintf(header_out,"#undef yyset_out\n");
            fprintf(header_out,"#undef yyterminate\n");
            fprintf(header_out,"#undef yytext\n");
            fprintf(header_out,"#undef yytext_ptr\n");
            fprintf(header_out,"#undef yywrap\n");

			/* undef any of the auto-generated symbols. */
			for(i=0; i < defs_buf.nelts; i++)
				fprintf(header_out, "#undef %s\n", ((char**)defs_buf.elts)[i]);

			fprintf(header_out,
					"\n"
					"#undef %sIN_HEADER\n"
					"#endif /* %sHEADER_H */\n",
					prefix, prefix);

			if ( ferror( header_out ) )
				lerrsf( _( "error creating header file %s" ), headerfilename);
			fflush(header_out);
			fclose(header_out);
		}

	if ( exit_status != 0 && outfile_created )
		{
		if ( ferror( stdout ) )
			lerrsf( _( "error writing output file %s" ),
				outfilename );

		else if ( fclose( stdout ) )
			lerrsf( _( "error closing output file %s" ),
				outfilename );

		else if ( unlink( outfilename ) )
			lerrsf( _( "error deleting output file %s" ),
				outfilename );
		}


	if ( backing_up_report && backing_up_file )
		{
		if ( num_backing_up == 0 )
			fprintf( backing_up_file, _( "No backing up.\n" ) );
		else if ( fullspd || fulltbl )
			fprintf( backing_up_file,
				_( "%d backing up (non-accepting) states.\n" ),
				num_backing_up );
		else
			fprintf( backing_up_file,
				_( "Compressed tables always back up.\n" ) );

		if ( ferror( backing_up_file ) )
			lerrsf( _( "error writing backup file %s" ),
				backing_name );

		else if ( fclose( backing_up_file ) )
			lerrsf( _( "error closing backup file %s" ),
				backing_name );
		}

	if ( printstats )
		{
		fprintf( stderr, _( "%s version %s usage statistics:\n" ),
			program_name, flex_version );

		fprintf( stderr, _( "  scanner options: -" ) );

		if ( C_plus_plus )
			putc( '+', stderr );
		if ( backing_up_report )
			putc( 'b', stderr );
		if ( ddebug )
			putc( 'd', stderr );
		if ( caseins )
			putc( 'i', stderr );
		if ( lex_compat )
			putc( 'l', stderr );
		if ( performance_report > 0 )
			putc( 'p', stderr );
		if ( performance_report > 1 )
			putc( 'p', stderr );
		if ( spprdflt )
			putc( 's', stderr );
        if ( reentrant )
            {
            putc( 'R', stderr );

            if( reentrant_bison_pure )
                putc( 'b', stderr );
            }
		if ( use_stdout )
			putc( 't', stderr );
		if ( printstats )
			putc( 'v', stderr );	/* always true! */
		if ( nowarn )
			putc( 'w', stderr );
		if ( interactive == false )
			putc( 'B', stderr );
		if ( interactive == true )
			putc( 'I', stderr );
		if ( ! gen_line_dirs )
			putc( 'L', stderr );
		if ( trace )
			putc( 'T', stderr );

		if ( csize == unspecified )
			/* We encountered an error fairly early on, so csize
			 * never got specified.  Define it now, to prevent
			 * bogus table sizes being written out below.
			 */
			csize = 256;

		if ( csize == 128 )
			putc( '7', stderr );
		else
			putc( '8', stderr );

		fprintf( stderr, " -C" );

		if ( long_align )
			putc( 'a', stderr );
		if ( fulltbl )
			putc( 'f', stderr );
		if ( fullspd )
			putc( 'F', stderr );
		if ( useecs )
			putc( 'e', stderr );
		if ( usemecs )
			putc( 'm', stderr );
		if ( use_read )
			putc( 'r', stderr );

		if ( did_outfilename )
			fprintf( stderr, " -o%s", outfilename );

		if ( skelname )
			fprintf( stderr, " -S%s", skelname );

		if ( strcmp( prefix, "yy" ) )
			fprintf( stderr, " -P%s", prefix );

		putc( '\n', stderr );

		fprintf( stderr, _( "  %d/%d NFA states\n" ),
			lastnfa, current_mns );
		fprintf( stderr, _( "  %d/%d DFA states (%d words)\n" ),
			lastdfa, current_max_dfas, totnst );
		fprintf( stderr, _( "  %d rules\n" ),
		num_rules + num_eof_rules - 1 /* - 1 for def. rule */ );

		if ( num_backing_up == 0 )
			fprintf( stderr, _( "  No backing up\n" ) );
		else if ( fullspd || fulltbl )
			fprintf( stderr,
			_( "  %d backing-up (non-accepting) states\n" ),
				num_backing_up );
		else
			fprintf( stderr,
				_( "  Compressed tables always back-up\n" ) );

		if ( bol_needed )
			fprintf( stderr,
				_( "  Beginning-of-line patterns used\n" ) );

		fprintf( stderr, _( "  %d/%d start conditions\n" ), lastsc,
			current_max_scs );
		fprintf( stderr,
			_( "  %d epsilon states, %d double epsilon states\n" ),
			numeps, eps2 );

		if ( lastccl == 0 )
			fprintf( stderr, _( "  no character classes\n" ) );
		else
			fprintf( stderr,
_( "  %d/%d character classes needed %d/%d words of storage, %d reused\n" ),
				lastccl, current_maxccls,
				cclmap[lastccl] + ccllen[lastccl],
				current_max_ccl_tbl_size, cclreuse );

		fprintf( stderr, _( "  %d state/nextstate pairs created\n" ),
			numsnpairs );
		fprintf( stderr, _( "  %d/%d unique/duplicate transitions\n" ),
			numuniq, numdup );

		if ( fulltbl )
			{
			tblsiz = lastdfa * numecs;
			fprintf( stderr, _( "  %d table entries\n" ), tblsiz );
			}

		else
			{
			tblsiz = 2 * (lastdfa + numtemps) + 2 * tblend;

			fprintf( stderr,
				_( "  %d/%d base-def entries created\n" ),
				lastdfa + numtemps, current_max_dfas );
			fprintf( stderr,
			_( "  %d/%d (peak %d) nxt-chk entries created\n" ),
				tblend, current_max_xpairs, peakpairs );
			fprintf( stderr,
		_( "  %d/%d (peak %d) template nxt-chk entries created\n" ),
				numtemps * nummecs,
				current_max_template_xpairs,
				numtemps * numecs );
			fprintf( stderr, _( "  %d empty table entries\n" ),
				nummt );
			fprintf( stderr, _( "  %d protos created\n" ),
				numprots );
			fprintf( stderr,
				_( "  %d templates created, %d uses\n" ),
				numtemps, tmpuses );
			}

		if ( useecs )
			{
			tblsiz = tblsiz + csize;
			fprintf( stderr,
				_( "  %d/%d equivalence classes created\n" ),
				numecs, csize );
			}

		if ( usemecs )
			{
			tblsiz = tblsiz + numecs;
			fprintf( stderr,
			_( "  %d/%d meta-equivalence classes created\n" ),
				nummecs, csize );
			}

		fprintf( stderr,
		_( "  %d (%d saved) hash collisions, %d DFAs equal\n" ),
			hshcol, hshsave, dfaeql );
		fprintf( stderr, _( "  %d sets of reallocations needed\n" ),
			num_reallocs );
		fprintf( stderr, _( "  %d total table entries needed\n" ),
			tblsiz );
		}

	exit( exit_status );
	}


/* flexinit - initialize flex */

void flexinit( argc, argv )
int argc;
char **argv;
	{
	int i, sawcmpflag, rv, optind;
	char *arg;
        scanopt_t sopt;

	printstats = syntaxerror = trace = spprdflt = caseins = false;
	lex_compat = C_plus_plus = backing_up_report = ddebug = fulltbl = false;
	fullspd = long_align = nowarn = yymore_used = continued_action = false;
	do_yylineno = yytext_is_array = in_rule = reject = do_stdinit = false;
	yymore_really_used = reject_really_used = unspecified;
	interactive = csize = unspecified;
	do_yywrap = gen_line_dirs = usemecs = useecs = true;
    reentrant = reentrant_bison_pure = false;
	performance_report = 0;
	did_outfilename = 0;
	prefix = "yy";
	yyclass = 0;
	use_read = use_stdout = false;

	sawcmpflag = false;

	/* Initialize dynamic array for holding the rule actions. */
	action_size = 2048;	/* default size of action array in bytes */
	action_array = allocate_character_array( action_size );
	defs1_offset = prolog_offset = action_offset = action_index = 0;
	action_array[0] = '\0';

	/* Initialize any buffers. */
	buf_init(&userdef_buf, sizeof(char));
	buf_init(&defs_buf, sizeof(char*));
	

    /* Enable C++ if program name ends with '+'. */
	program_name = basename2(argv[0],0);

	if ( program_name[0] != '\0' &&
	     program_name[strlen( program_name ) - 1] == '+' )
		C_plus_plus = true;

	/* read flags */
        sopt = scanopt_init(flexopts, argc, argv, 0);
        if (!sopt) {
            /* This will only happen when flexopts array is altered. */
            fprintf(stderr,
                    _("Internal error. flexopts are malformed.\n"));
            exit(1);
        }

        while((rv=scanopt(sopt, &arg, &optind)) != 0){

            if (rv < 0) {
                /* Scanopt has already printed an option-specific error message. */
                fprintf( stderr, _( "For usage, try\n\t%s --help\n" ),
                    program_name );
                exit( 1 );
                break;
            }

            switch ((enum flexopt_flag_t)rv){
            case OPT_CPLUSPLUS:
                    C_plus_plus = true;
                    break;

            case OPT_BATCH:
                    interactive = false;
                    break;

            case OPT_BACKUP:
                    backing_up_report = true;
                    break;

            case OPT_DONOTHING:
                    break;

            case OPT_COMPRESSION:
                    if ( ! sawcmpflag )
                            {
                            useecs = false;
                            usemecs = false;
                            fulltbl = false;
                            sawcmpflag = true;
                            }

                    for( i=0 ; arg && arg[i] != '\0'; i++)
                            switch ( arg[i] )
                                    {
                                    case 'a':
                                            long_align = true;
                                            break;

                                    case 'e':
                                            useecs = true;
                                            break;

                                    case 'F':
                                            fullspd = true;
                                            break;

                                    case 'f':
                                            fulltbl = true;
                                            break;

                                    case 'm':
                                            usemecs = true;
                                            break;

                                    case 'r':
                                            use_read = true;
                                            break;

                                    default:
                                            lerrif(
                            _( "unknown -C option '%c'" ),
                                            (int) arg[i] );
                                            break;
                                    }
                    break;

            case OPT_DEBUG:
                    ddebug = true;
                    break;

            case OPT_NO_DEBUG:
                    ddebug = false;
                    break;

            case OPT_FULL:
                    useecs = usemecs = false;
                    use_read = fulltbl = true;
                    break;

            case OPT_FAST:
                    useecs = usemecs = false;
                    use_read = fullspd = true;
                    break;

            case OPT_HELP:
                    usage();
                    exit( 0 );

            case OPT_INTERACTIVE:
                    interactive = true;
                    break;

            case OPT_CASE_INSENSITIVE:
                    caseins = true;
                    break;

            case OPT_LEX_COMPAT:
                    lex_compat = true;
                    break;

            case OPT_MAIN:
                    buf_strdefine(&userdef_buf, "YY_MAIN", "1");
                    do_yywrap = false;
                    break;

            case OPT_NO_MAIN:
                    buf_strdefine(&userdef_buf, "YY_MAIN", "0");
                    break;

            case OPT_NO_LINE:
                    gen_line_dirs = false;
                    break;

            case OPT_OUTFILE:
                    outfilename = arg;
                    did_outfilename = 1;
                    break;

            case OPT_PREFIX:
                    prefix = arg;
                    break;

            case OPT_PERF_REPORT:
                    ++performance_report;
                    break;

            case OPT_REENTRANT_BISON:
                    reentrant = true;
                    reentrant_bison_pure = true;
                    break;

            case OPT_REENTRANT:
                    reentrant = true;

                    /* Optional 'b' follows -R */
                    if (arg) {
                        if (strcmp(arg,"b")==0)
                            reentrant_bison_pure = true;
                        else
                            lerrif(_( "unknown -R option '%c'" ),(int)arg[0]);
                    }
                    break;

            case OPT_NO_REENTRANT:
                    reentrant = reentrant_bison_pure = false;
                    break;

            case OPT_SKEL:
                    skelname = arg;
                    break;

            case OPT_DEFAULT:
                    spprdflt = false;
                    break;

            case OPT_NO_DEFAULT:
                    spprdflt = true;
                    break;

            case OPT_STDOUT:
                    use_stdout = true;
                    break;
                    
            case OPT_NO_UNISTD_H:
                    buf_strdefine(&userdef_buf,"YY_NO_UNISTD_H", "1");
                    break;
                
            case OPT_TRACE:
                    trace = true;
                    break;

            case OPT_VERBOSE:
                    printstats = true;
                    break;

            case OPT_VERSION:
                    printf( _( "%s %s\n" ),
                            program_name, flex_version );
                    exit( 0 );

            case OPT_WARN:
                    nowarn = false;
                    break;

            case OPT_NO_WARN:
                    nowarn = true;
                    break;

            case OPT_7BIT:
                    csize = 128;
                    break;

            case OPT_8BIT:
                    csize = CSIZE;
                    break;

            case OPT_ALIGN:
                    long_align = true;
                    break;

            case OPT_NO_ALIGN:
                    long_align = false;
                    break;

            case OPT_ALWAYS_INTERACTIVE:
                    buf_strdefine(&userdef_buf,"YY_ALWAYS_INTERACTIVE", "1");
                    break;

            case OPT_NEVER_INTERACTIVE:
                    buf_strdefine(&userdef_buf, "YY_NEVER_INTERACTIVE", "1" );
                    break;

            case OPT_ARRAY:
                    yytext_is_array = true;
                    break;

            case OPT_POINTER:
                    yytext_is_array = false;
                    break;

            case OPT_ECS:
                    useecs = true;
                    break;

            case OPT_NO_ECS:
                    useecs = false;
                    break;

            case OPT_HEADER:
                    headerfilename = arg;
                    break;

            case OPT_META_ECS:
                    usemecs = true;
                    break;

            case OPT_NO_META_ECS:
                    usemecs = false;
                    break;

            case OPT_PREPROCDEFINE: 
                    {
                    /* arg is "symbol" or "symbol=definition". */
                        char *def;

                        for(def=arg; *def != '\0' && *def!='='; ++def)
                            ;

                        buf_strappend(&userdef_buf,"#define ");
                        if (*def=='\0'){
                            buf_strappend(&userdef_buf,arg);
                            buf_strappend(&userdef_buf, " 1\n");
                        }else{
                            buf_strnappend(&userdef_buf, arg,def-arg);
                            buf_strappend(&userdef_buf, " ");
                            buf_strappend(&userdef_buf, def+1);
                            buf_strappend(&userdef_buf, "\n");
                        }
                    }
                    break;

            case OPT_READ:
                    use_read = true;
                    break;

            case OPT_STACK:
                    buf_strdefine(&userdef_buf,"YY_STACK_USED","1");
                    break;

            case OPT_STDINIT:
                    do_stdinit = true;
                    break;

            case OPT_NO_STDINIT:
                    do_stdinit = false;
                    break;

            case OPT_YYCLASS:
                    yyclass = arg;
                    break;

            case OPT_YYLINENO:
                    do_yylineno = true;
                    break;

            case OPT_NO_YYLINENO:
                    do_yylineno = false;
                    break;

            case OPT_YYWRAP:
                    do_yywrap = true;
                    break;

            case OPT_NO_YYWRAP:
                    do_yywrap = false;
                    break;

            case OPT_YYMORE:
                    yymore_really_used = true;
                    break;

            case OPT_NO_YYMORE:
                    yymore_really_used = false;
                    break;

            case OPT_REJECT:
                    reject_really_used = true;
                    break;

            case OPT_NO_REJECT:
                    reject_really_used = false;
                    break;

            case OPT_NO_YY_PUSH_STATE:
                    buf_strdefine(&userdef_buf, "YY_NO_PUSH_STATE", "1");
                    break;
            case OPT_NO_YY_POP_STATE:
                    buf_strdefine(&userdef_buf, "YY_NO_POP_STATE", "1");
                    break;
            case OPT_NO_YY_TOP_STATE:
                    buf_strdefine(&userdef_buf, "YY_NO_TOP_STATE", "1");
                    break;
            case OPT_NO_UNPUT:
                    buf_strdefine(&userdef_buf, "YY_NO_UNPUT", "1");
                    break;
            case OPT_NO_YY_SCAN_BUFFER:
                    buf_strdefine(&userdef_buf, "YY_NO_SCAN_BUFFER", "1");
                    break;
            case OPT_NO_YY_SCAN_BYTES:
                    buf_strdefine(&userdef_buf, "YY_NO_SCAN_BYTES", "1");
                    break;
            case OPT_NO_YY_SCAN_STRING:
                    buf_strdefine(&userdef_buf, "YY_NO_SCAN_STRING", "1");
                    break;
            case OPT_NO_YYGET_EXTRA:
                    buf_strdefine(&userdef_buf, "YY_NO_GET_EXTRA", "1");
                    break;
            case OPT_NO_YYSET_EXTRA:
                    buf_strdefine(&userdef_buf, "YY_NO_SET_EXTRA", "1");
                    break;
            case OPT_NO_YYGET_LENG:
                    buf_strdefine(&userdef_buf, "YY_NO_GET_LENG", "1");
                    break;
            case OPT_NO_YYGET_TEXT:
                    buf_strdefine(&userdef_buf, "YY_NO_GET_TEXT", "1");
                    break;
            case OPT_NO_YYGET_LINENO:
                    buf_strdefine(&userdef_buf, "YY_NO_GET_LINENO", "1");
                    break;
            case OPT_NO_YYSET_LINENO:
                    buf_strdefine(&userdef_buf, "YY_NO_SET_LINENO", "1");
                    break;
            case OPT_NO_YYGET_IN:
                    buf_strdefine(&userdef_buf, "YY_NO_GET_IN", "1");
                    break;
            case OPT_NO_YYSET_IN:
                    buf_strdefine(&userdef_buf, "YY_NO_SET_IN", "1");
                    break;
            case OPT_NO_YYGET_OUT:
                    buf_strdefine(&userdef_buf, "YY_NO_GET_OUT", "1");
                    break;
            case OPT_NO_YYSET_OUT:
                    buf_strdefine(&userdef_buf, "YY_NO_SET_OUT", "1");
                    break;
            case OPT_NO_YYGET_LVAL:
                    buf_strdefine(&userdef_buf, "YY_NO_GET_LVAL", "1");
                    break;
            case OPT_NO_YYSET_LVAL:
                    buf_strdefine(&userdef_buf, "YY_NO_SET_LVAL", "1");
                    break;
            case OPT_NO_YYGET_LLOC:
                    buf_strdefine(&userdef_buf, "YY_NO_GET_LLOC", "1");
                    break;
            case OPT_NO_YYSET_LLOC:
                    buf_strdefine(&userdef_buf, "YY_NO_SET_LLOC", "1");
                    break;

            } /* switch */
        } /* while scanopt() */

        scanopt_destroy(sopt);

	num_input_files = argc - optind;
	input_files = argv + optind;
	set_input_file( num_input_files > 0 ? input_files[0] : NULL );

	lastccl = lastsc = lastdfa = lastnfa = 0;
	num_rules = num_eof_rules = default_rule = 0;
	numas = numsnpairs = tmpuses = 0;
	numecs = numeps = eps2 = num_reallocs = hshcol = dfaeql = totnst = 0;
	numuniq = numdup = hshsave = eofseen = datapos = dataline = 0;
	num_backing_up = onesp = numprots = 0;
	variable_trailing_context_rules = bol_needed = false;

	out_linenum = linenum = sectnum = 1;
	firstprot = NIL;

	/* Used in mkprot() so that the first proto goes in slot 1
	 * of the proto queue.
	 */
	lastprot = 1;

	set_up_initial_allocations();
	}


/* readin - read in the rules section of the input file(s) */

void readin()
	{
	static char yy_stdinit[] = "FILE *yyin = stdin, *yyout = stdout;";
	static char yy_nostdinit[] =
		"FILE *yyin = (FILE *) 0, *yyout = (FILE *) 0;";

	line_directive_out( (FILE *) 0, 1 );

	if ( yyparse() )
		{
		pinpoint_message( _( "fatal parse error" ) );
		flexend( 1 );
		}

	if ( syntaxerror )
		flexend( 1 );

	if ( backing_up_report )
		{
		backing_up_file = fopen( backing_name, "w" );
		if ( backing_up_file == NULL )
			lerrsf(
			_( "could not create backing-up info file %s" ),
				backing_name );
		}

	else
		backing_up_file = NULL;

	if ( yymore_really_used == true )
		yymore_used = true;
	else if ( yymore_really_used == false )
		yymore_used = false;

	if ( reject_really_used == true )
		reject = true;
	else if ( reject_really_used == false )
		reject = false;

	if ( performance_report > 0 )
		{
		if ( lex_compat )
			{
			fprintf( stderr,
_( "-l AT&T lex compatibility option entails a large performance penalty\n" ) );
			fprintf( stderr,
_( " and may be the actual source of other reported performance penalties\n" ) );
			}

		else if ( do_yylineno )
			{
			fprintf( stderr,
	_( "%%option yylineno entails a large performance penalty\n" ) );
			}

		if ( performance_report > 1 )
			{
			if ( interactive )
				fprintf( stderr,
	_( "-I (interactive) entails a minor performance penalty\n" ) );

			if ( yymore_used )
				fprintf( stderr,
		_( "yymore() entails a minor performance penalty\n" ) );
			}

		if ( reject )
			fprintf( stderr,
			_( "REJECT entails a large performance penalty\n" ) );

		if ( variable_trailing_context_rules )
			fprintf( stderr,
_( "Variable trailing context rules entail a large performance penalty\n" ) );
		}

	if ( reject )
		real_reject = true;

	if ( variable_trailing_context_rules )
		reject = true;

	if ( (fulltbl || fullspd) && reject )
		{
		if ( real_reject )
			flexerror(
				_( "REJECT cannot be used with -f or -F" ) );
		else if ( do_yylineno )
			flexerror(
			_( "%option yylineno cannot be used with -f or -F" ) );
		else
			flexerror(
	_( "variable trailing context rules cannot be used with -f or -F" ) );
		}

	if ( reject )
		outn( "\n#define YY_USES_REJECT" );

	if ( ! do_yywrap )
		{
        outn( "\n#define yywrap(n) 1" );
		outn( "#define YY_SKIP_YYWRAP" );
		}

	if ( ddebug )
		outn( "\n#define FLEX_DEBUG" );

	OUT_BEGIN_CODE();	
	if ( csize == 256 )
		outn( "typedef unsigned char YY_CHAR;" );
	else
		outn( "typedef char YY_CHAR;" );
	OUT_END_CODE();

	if ( C_plus_plus )
		{
		outn( "#define yytext_ptr yytext" );

		if ( interactive )
			outn( "#define YY_INTERACTIVE" );
		}

	else
		{
		OUT_BEGIN_CODE();
            /* In reentrant scanner, stdinit is handled in flex.skl. */
		if ( do_stdinit )
			{
            outn( "#ifdef YY_REENTRANT" );
			outn( "#ifdef VMS" );
			outn( "#ifdef __VMS_POSIX" );
            outn( "#define YY_STDINIT" );
			outn( "#endif" );
			outn( "#else" );
            outn( "#define YY_STDINIT" );
			outn( "#endif" );
            outn( "#else /* end YY_REENTRANT */" );
			outn( "#ifdef VMS" );
			outn( "#ifndef __VMS_POSIX" );
			outn( yy_nostdinit );
			outn( "#else" );
			outn( yy_stdinit );
			outn( "#endif" );
			outn( "#else" );
			outn( yy_stdinit );
			outn( "#endif" );
            outn( "#endif" );
			}

		else
            {
            outn( "#ifndef YY_REENTRANT" );
			outn( yy_nostdinit );
            outn( "#endif" );
            }
		OUT_END_CODE();
		}

	OUT_BEGIN_CODE();
	if ( fullspd )
		outn( "typedef yyconst struct yy_trans_info *yy_state_type;" );
	else if ( ! C_plus_plus )
		outn( "typedef int yy_state_type;" );
	OUT_END_CODE();

	if ( ddebug )
		outn( "\n#define FLEX_DEBUG" );

	if ( lex_compat )
		outn( "#define YY_FLEX_LEX_COMPAT" );

	if ( do_yylineno && ! C_plus_plus && ! reentrant )
		{
		outn( "extern int yylineno;" );
		OUT_BEGIN_CODE();
		outn( "int yylineno = 1;" );
		OUT_END_CODE();
		}

	if ( C_plus_plus )
		{
		outn( "\n#include <FlexLexer.h>" );

		if ( yyclass )
			{
			outn( "int yyFlexLexer::yylex()" );
			outn( "\t{" );
			outn(
"\tLexerError( \"yyFlexLexer::yylex invoked but %option yyclass used\" );" );
			outn( "\treturn 0;" );
			outn( "\t}" );

			out_str( "\n#define YY_DECL int %s::yylex()\n",
				yyclass );
			}
		}

	else
		{

		/* Watch out: yytext_ptr is a variable when yytext is an array,
		 * but it's a macro when yytext is a pointer.
		 */
		if ( yytext_is_array )
			{
			if ( !reentrant )
				outn( "extern char yytext[];\n" );
			}
		else
		{
			if ( reentrant )
				{
				outn( "#define yytext_ptr yytext_r" );
				}
			else
				{
				outn( "extern char *yytext;" );
			    outn( "#define yytext_ptr yytext" );
				}
		}

		if ( yyclass )
			flexerror(
		_( "%option yyclass only meaningful for C++ scanners" ) );
		}

	if ( useecs )
		numecs = cre8ecs( nextecm, ecgroup, csize );
	else
		numecs = csize;

	/* Now map the equivalence class for NUL to its expected place. */
	ecgroup[0] = ecgroup[csize];
	NUL_ec = ABS( ecgroup[0] );

	if ( useecs )
		ccl2ecl();
	}


/* set_up_initial_allocations - allocate memory for internal tables */

void set_up_initial_allocations()
	{
	maximum_mns = (long_align ? MAXIMUM_MNS_LONG : MAXIMUM_MNS);
	current_mns = INITIAL_MNS;
	firstst = allocate_integer_array( current_mns );
	lastst = allocate_integer_array( current_mns );
	finalst = allocate_integer_array( current_mns );
	transchar = allocate_integer_array( current_mns );
	trans1 = allocate_integer_array( current_mns );
	trans2 = allocate_integer_array( current_mns );
	accptnum = allocate_integer_array( current_mns );
	assoc_rule = allocate_integer_array( current_mns );
	state_type = allocate_integer_array( current_mns );

	current_max_rules = INITIAL_MAX_RULES;
	rule_type = allocate_integer_array( current_max_rules );
	rule_linenum = allocate_integer_array( current_max_rules );
	rule_useful = allocate_integer_array( current_max_rules );

	current_max_scs = INITIAL_MAX_SCS;
	scset = allocate_integer_array( current_max_scs );
	scbol = allocate_integer_array( current_max_scs );
	scxclu = allocate_integer_array( current_max_scs );
	sceof = allocate_integer_array( current_max_scs );
	scname = allocate_char_ptr_array( current_max_scs );

	current_maxccls = INITIAL_MAX_CCLS;
	cclmap = allocate_integer_array( current_maxccls );
	ccllen = allocate_integer_array( current_maxccls );
	cclng = allocate_integer_array( current_maxccls );

	current_max_ccl_tbl_size = INITIAL_MAX_CCL_TBL_SIZE;
	ccltbl = allocate_Character_array( current_max_ccl_tbl_size );

	current_max_dfa_size = INITIAL_MAX_DFA_SIZE;

	current_max_xpairs = INITIAL_MAX_XPAIRS;
	nxt = allocate_integer_array( current_max_xpairs );
	chk = allocate_integer_array( current_max_xpairs );

	current_max_template_xpairs = INITIAL_MAX_TEMPLATE_XPAIRS;
	tnxt = allocate_integer_array( current_max_template_xpairs );

	current_max_dfas = INITIAL_MAX_DFAS;
	base = allocate_integer_array( current_max_dfas );
	def = allocate_integer_array( current_max_dfas );
	dfasiz = allocate_integer_array( current_max_dfas );
	accsiz = allocate_integer_array( current_max_dfas );
	dhash = allocate_integer_array( current_max_dfas );
	dss = allocate_int_ptr_array( current_max_dfas );
	dfaacc = allocate_dfaacc_union( current_max_dfas );

	nultrans = (int *) 0;
	}


/* extracts basename from path, optionally stripping the extension "\.*"
 * (same concept as /bin/sh `basename`, but different handling of extension). */
static char * basename2(path, strip_ext)
    char * path;
    int strip_ext; /* boolean */
{
    char *b, *e=0;
    b = path;
    for (b=path; *path; path++)
        if (*path== '/')
            b = path+1;
        else if (*path=='.')
            e = path;

    if (strip_ext && e && e > b)
        *e = '\0';
    return b;
}

void usage()
	{
    FILE *f = stdout;
    if ( ! did_outfilename )
        {
        sprintf( outfile_path, outfile_template,
                prefix, C_plus_plus ? "cc" : "c" );
        outfilename = outfile_path;
        }

    fprintf(f,_( "Usage: %s [OPTIONS] [FILE]...\n"), program_name);
    fprintf(f,
_(
"Generates programs that perform pattern-matching on text.\n"
"\n"
"Table Compression:\n"
"  -Ca, --align      trade off larger tables for better memory alignment\n"
"  -Ce, --ecs        construct equivalence classes\n"
"  -Cf               do not compress tables; use -f representation\n"
"  -CF               do not compress tables; use -F representation\n"
"  -Cm, --meta-ecs   construct meta-equivalence classes\n"
"  -Cr, --read       use read() instead of stdio for scanner input\n"
"  -f, --full        generate fast, large scanner. Same as -Cfr\n"
"  -F, --fast        use alternate table representation. Same as -CFr\n"
"  -Cem              default compression (same as --ecs --meta-ecs)\n"

"\n"
"Debugging:\n"
"  -d, --debug             enable debug mode in scanner\n"
"  -b, --backup            write backing-up information to %s\n"
"  -p, --perf-report       write performance report to stderr\n"
"  -s, --nodefault         suppress default rule to ECHO unmatched text\n"
"  -T, --trace             %s should run in trace mode\n"
"  -w, --nowarn            do not generate warnings\n"
"  -v, --verbose           write summary of scanner statistics to stdout\n"

"\n"
"Files:\n"
"  -o, --outfile=FILE      specify output filename\n"
"  -S, --skel=FILE         specify skeleton file\n"
"  -t, --stdout            write scanner on stdout instead of %s\n"
"      --yyclass=NAME      name of C++ class\n"
"      --header=FILE       create a C header file in addition to the scanner\n"
    
"\n"
"Scanner behavior:\n"
"  -7, --7bit              generate 7-bit scanner\n"
"  -8, --8bit              generate 8-bit scanner\n"
"  -B, --batch             generate batch scanner (opposite of -I)\n"
"  -i, --case-insensitive  ignore case in patterns\n"
"  -l, --lex-compat        maximal compatibility with original lex\n"
"  -I, --interactive       generate interactive scanner (opposite of -B)\n"
"      --yylineno          track line count in yylineno\n"

"\n"
"Generated code:\n"
"  -+,  --c++               generate C++ scanner class\n"
"  -Dmacro[=defn]           #define macro defn  (default defn is '1')\n"
"  -L,  --noline            suppress #line directives in scanner\n"
"  -P,  --prefix=STRING     use STRING as prefix instead of \"yy\"\n"
"  -R,  --reentrant         generate a reentrant C scanner\n"
"  -Rb, --reentrant-bison   reentrant scanner for bison pure parser.\n"
"       --stdinit           initialize yyin/yyout to stdin/stdout\n"
"       --nounistd          do not include <unistd.h>\n"
"       --noFUNCTION        do not generate a particular FUNCTION\n"

"\n"
"Miscellaneous:\n"
"  -c                      do-nothing POSIX option\n"
"  -n                      do-nothing POSIX option\n"
"  -?\n"
"  -h, --help              produce this help message\n"
"  -V, --version           report %s version\n"
), backing_name, program_name, outfile_path, program_name);

}
