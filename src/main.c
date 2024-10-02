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


#include "flexdef.h"
#include "version.h"
#include "options.h"
#include "tables.h"
#include "parse.h"

static const char flex_version[] = FLEX_VERSION;

/* declare functions that have forward references */

void flexinit(FlexState* gv, int, char **);
void readin(FlexState* gv);
void set_up_initial_allocations(FlexState* gv);

///* these globals are all defined and commented in flexdef.h */
//bool    syntaxerror, eofseen;
//int     yymore_used, reject, real_reject, continued_action, in_rule;
//int     datapos, dataline, linenum;
//FILE   *skelfile = NULL;
//int     skel_ind = 0;
//char   *action_array;
//int     action_size, defs1_offset, prolog_offset, action_offset,
//	action_index;
//char   *infilename = NULL;
//char   *extra_type = NULL;
//int     onestate[ONE_STACK_SIZE], onesym[ONE_STACK_SIZE];
//int     onenext[ONE_STACK_SIZE], onedef[ONE_STACK_SIZE], onesp;
//int     maximum_mns, current_mns, current_max_rules;
//int     num_rules, num_eof_rules, default_rule, lastnfa;
//int    *firstst, *lastst, *finalst, *transchar, *trans1, *trans2;
//int    *accptnum, *assoc_rule, *state_type;
//int    *rule_type, *rule_linenum;
//int     current_state_type;
//bool    variable_trailing_context_rules;
//int     numtemps, numprots, protprev[MSP], protnext[MSP], prottbl[MSP];
//int     protcomst[MSP], firstprot, lastprot, protsave[PROT_SAVE_SIZE];
//int     numecs, nextecm[CSIZE + 1], ecgroup[CSIZE + 1], nummecs,
//	tecfwd[CSIZE + 1];
//int     tecbck[CSIZE + 1];
//int     lastsc, *scset, *scbol;
///* scxclu[] and sceof[] are boolean arrays, but allocated as char
// * arrays for size. */
//char   *scxclu, *sceof;
//int     current_max_scs;
//const char **scname;
//int     current_max_dfa_size, current_max_xpairs;
//int     current_max_template_xpairs, current_max_dfas;
//int     lastdfa, *nxt, *chk, *tnxt;
//int    *base, *def, *nultrans, NUL_ec, tblend, firstfree, **dss, *dfasiz;
//union dfaacc_union *dfaacc;
//int    *accsiz, *dhash, numas;
//int     numsnpairs, jambase, jamstate;
//int     lastccl, *cclmap, *ccllen, *cclng, cclreuse;
//int     current_maxccls, current_max_ccl_tbl_size;
//unsigned char   *ccltbl;
//char    nmstr[MAXLINE];
//int     sectnum, nummt, hshcol, dfaeql, numeps, eps2, num_reallocs, nmval;
//int     tmpuses, totnst, peakpairs, numuniq, numdup, hshsave;
//int     num_backing_up, bol_needed;
//int     end_of_buffer_state;
//char  **input_files;
//int     num_input_files;
//jmp_buf flex_main_jmp_buf;
///* rule_useful[], rule_has_nl[] and ccl_has_nl[] are boolean arrays,
// * but allocated as char arrays for size. */
//char   *rule_useful, *rule_has_nl, *ccl_has_nl;
//int     nlch = '\n';
//
//bool    tablesext, tablesverify, gentables;
//char   *tablesfilename=0,*tablesname=0;
//struct yytbl_writer tableswr;
//size_t footprint;
//
//struct ctrl_bundle_t ctrl;
//struct env_bundle_t env;
//
///* Make sure program_name is initialized so we don't crash if writing
// * out an error message before getting the program name from argv[0].
// */
//char   *program_name = "flex";

//FlexState *gv;
FlexState* newFlexState(void)
{
    FlexState *fgv = malloc(sizeof(FlexState));
    if(!fgv) return NULL;
    memset(fgv, 0, sizeof(*fgv));
    fgv->nlch = '\n';
    fgv->program_name = "flex";
    fgv->escaped_qstart = "]]M4_YY_NOOP[M4_YY_NOOP[M4_YY_NOOP[[";
    fgv->escaped_qend   = "]]M4_YY_NOOP]M4_YY_NOOP]M4_YY_NOOP[[";
    fgv->called_before = -1;
    fgv->outfile_template = "lex.%s.%s";
    fgv->backing_name = "lex.backup";
    fgv->tablesfile_template = "lex.%s.tables";
    fgv->preproc_level = 1000;
    return fgv;
}

static void freeHashTable(struct hash_entry **tbl, int sz, int freeName)
{
    for(int i=0; i< sz; ++i)
    {
        struct hash_entry *sym_entry = tbl[i];
        while(sym_entry)
        {
            struct hash_entry *tmp = sym_entry->next;
            free(sym_entry->str_val);
            if(freeName) free(sym_entry->name);
            free(sym_entry);
            sym_entry = tmp;
        }
    }
}

void closeFlexState(FlexState* gv)
{
    //from misc.c
    free(gv->action_array);
    
    //from tblcmp.c
    free(gv->tnxt);
    free(gv->nxt);
    free(gv->chk);
    
    //from nfa.c
    free(gv->firstst);
    free(gv->lastst);
    free(gv->finalst);
    free(gv->transchar);
    free(gv->trans1);
    free(gv->trans2);
    free(gv->accptnum);
    free(gv->assoc_rule);
    free(gv->state_type);
    free(gv->rule_type);
    free(gv->rule_linenum);
    free(gv->rule_useful);
    free(gv->rule_has_nl);
    
    //from dfa.c
    free(gv->base);
    free(gv->def);
    free(gv->dfasiz);
    free(gv->accsiz);
    free(gv->dhash);
    for(int i=0; i <= gv->lastdfa; ++i )
        free(gv->dss[i]);
    free(gv->dss);
    free(gv->dfaacc);
    free(gv->nultrans);
    free(gv->stk);
    
    //from ccl.c
    free(gv->ccltbl);
    free(gv->cclmap);
    free(gv->ccllen);
    free(gv->cclng);
    free(gv->ccl_has_nl);
    
    //from sym.c
    free(gv->scset);
    free(gv->scbol);
    free(gv->scxclu);
    free(gv->sceof);
    for(int i=0; i<= gv->lastsc; ++i){
        free(gv->scname[i]);
    }
    free(gv->scname);
    freeHashTable(gv->sctbl, START_COND_HASH_SIZE, 0);
    freeHashTable(gv->ndtbl, NAME_TABLE_HASH_SIZE, 1);
    freeHashTable(gv->ccltab, CCL_HASH_SIZE, 1);
    
    //from regex.c
    regfree(&gv->regex_linedir);
    
    //from buf.c
    free(gv->top_buf.elts);
    
    //from main.c
    while(gv->output_chain)
    {
        struct filter *tmp = gv->output_chain->next;
        free(gv->output_chain->argv);
        free(gv->output_chain);
        gv->output_chain = tmp;
    }
    
    //from sleletons.c
    free(gv->ctrl.backend_name);
    free(gv->ctrl.traceline_re);
    free(gv->ctrl.traceline_template);
    
    //from scan.l
    free(gv->infilename);
    if(gv->fp_infilename && gv->fp_infilename != stdin) fclose(gv->fp_infilename);

    //from scanflags.c
    free(gv->_sf_stk);
    
    //from parse.y
    free(gv->scon_stk);
    
    
    free(gv);
}

//static const char outfile_template[] = "lex.%s.%s";
//static const char *backing_name = "lex.backup";
//static const char tablesfile_template[] = "lex.%s.tables";

/* From scan.l */
//extern FILE* yyout;

//static char outfile_path[MAXLINE];
//static int outfile_created = 0;
//static int _stdout_closed = 0; /* flag to prevent double-fclose() on stdout. */
//const char *escaped_qstart = "]]M4_YY_NOOP[M4_YY_NOOP[M4_YY_NOOP[[";
//const char *escaped_qend   = "]]M4_YY_NOOP]M4_YY_NOOP]M4_YY_NOOP[[";

/* For debugging. The max number of filters to apply to skeleton. */
//static int preproc_level = 1000;

int flex_main (int argc, char *argv[]);

int flex_main (int argc, char *argv[])
{
	int     i, exit_status, child_status, did_eof_rule, yylval;
        FlexState *gv;
        gv = newFlexState();
        if(!gv) {
		printf(" Failed to initialize the flex state.\n");
                return -1;
        }

        exit_status = yylex_init (&gv->scanner);
	if(exit_status) {
		printf(" Failed to initialize the scanner: %d\n", exit_status);
		//return exit_status;
	}
        else yyset_extra(gv, gv->scanner);
	/* Set a longjmp target. Yes, I know it's a hack, but it gets worse: The
	 * return value of setjmp, if non-zero, is the desired exit code PLUS ONE.
	 * For example, if you want 'main' to return with code '2', then call
	 * longjmp() with an argument of 3. This is because it is invalid to
	 * specify a value of 0 to longjmp. FLEX_EXIT(n) should be used instead of
	 * exit(n);
	 */
        did_eof_rule = false;
	exit_status = setjmp (gv->flex_main_jmp_buf);
	if (exit_status){
		if (stdout && !gv->_stdout_closed && !ferror(stdout)){
			fflush(stdout);
			fclose(stdout);
		}
		while (wait(&child_status) > 0){
			if (!WIFEXITED (child_status)
			    || WEXITSTATUS (child_status) != 0){
				/* report an error of a child
				 */
				if( exit_status <= 1 )
					exit_status = 2;

			}
		}
                yylex_destroy (gv->scanner);
                closeFlexState(gv);
		return exit_status - 1;
	}

	flexinit (gv, argc, argv);

	readin (gv);
	skelout (gv, true);		/* %% [1.0] DFA */
	gv->footprint += ntod (gv);

	for (i = 1; i <= gv->num_rules; ++i)
		if (!gv->rule_useful[i] && i != gv->default_rule)
			line_warning (gv, _("rule cannot be matched"),
				      gv->rule_linenum[i]);

	if (gv->ctrl.spprdflt && !gv->reject && gv->rule_useful[gv->default_rule])
		line_warning (gv, _
			      ("-s option given but default rule can be matched"),
			      gv->rule_linenum[gv->default_rule]);

	comment("START of m4 controls\n");

	// mode switches for yy_trans_info specification
	// nultrans
	if (gv->nultrans)
		visible_define ( "M4_MODE_NULTRANS");
	else {
		visible_define ( "M4_MODE_NO_NULTRANS");
		if (gv->ctrl.fulltbl)
		    visible_define ( "M4_MODE_NULTRANS_FULLTBL");
		else
		    visible_define ( "M4_MODE_NO_NULTRANS_FULLTBL");
		if (gv->ctrl.fullspd)
		    visible_define ( "M4_MODE_NULTRANS_FULLSPD");
		else
		    visible_define ( "M4_MODE_NO_NULTRANS_FULLSPD");
	}
	
	comment("END of m4 controls\n");
	out ("\n");

	comment("START of Flex-generated definitions\n");
	out_str_dec ("M4_HOOK_CONST_DEFINE_UINT(%s, %d)", "YY_NUM_RULES", gv->num_rules);
	out_str_dec ("M4_HOOK_CONST_DEFINE_STATE(%s, %d)", "YY_END_OF_BUFFER", gv->num_rules + 1);
	out_str_dec ("M4_HOOK_CONST_DEFINE_STATE(%s, %d)", "YY_JAMBASE", gv->jambase);
	out_str_dec ("M4_HOOK_CONST_DEFINE_STATE(%s, %d)", "YY_JAMSTATE", gv->jamstate);
	out_str_dec ("M4_HOOK_CONST_DEFINE_BYTE(%s, %d)", "YY_NUL_EC", gv->NUL_ec);
	/* Need to define the transet type as a size large
	 * enough to hold the biggest offset.
	 */
	out_str ("M4_HOOK_SET_OFFSET_TYPE(%s)", optimize_pack(gv, gv->tblend + gv->numecs + 1)->name);
	comment("END of Flex-generated definitions\n");

	skelout (gv, true);		/* %% [2.0] - tables get dumped here */

	/* Generate the C state transition tables from the DFA. */
	make_tables (gv);

	skelout (gv, true);		/* %% [3.0] - mode-dependent static declarations get dumped here */

	out (&gv->action_array[gv->defs1_offset]);

	line_directive_out (gv, stdout, NULL, gv->linenum);

	skelout (gv, true);		/* %% [4.0] - various random yylex internals get dumped here */

	/* Copy prolog to output file. */
	out (&gv->action_array[gv->prolog_offset]);

	line_directive_out (gv, stdout, NULL, gv->linenum);

	skelout (gv, true);		/* %% [5.0] - main loop of matching-engine code gets dumped here */

	/* Copy actions to output file. */
	out (&gv->action_array[gv->action_offset]);

	line_directive_out (gv, stdout, NULL, gv->linenum);

	/* generate cases for any missing EOF rules */
	for (i = 1; i <= gv->lastsc; ++i)
		if (!gv->sceof[i]) {
			out_str ("M4_HOOK_EOF_STATE_CASE_ARM(%s)", gv->scname[i]);
			outc('\n');
			out ("M4_HOOK_EOF_STATE_CASE_FALLTHROUGH");
			outc('\n');
			did_eof_rule = true;
		}

	if (did_eof_rule) {
		out ("M4_HOOK_EOF_STATE_CASE_TERMINATE");
	}

	skelout (gv, true);

	/* Copy remainder of input to output. */

	line_directive_out (gv, stdout, gv->infilename, gv->linenum);

	if (gv->sectnum == 3) {
		OUT_BEGIN_CODE ();
                if (!gv->ctrl.no_section3_escape)
                   fputs("[[", stdout);
		(void) flexscan (&yylval, gv->scanner);	/* copy remainder of input to output */
                if (!gv->ctrl.no_section3_escape)
                   fputs("]]", stdout);
		OUT_END_CODE ();
	}

	/* Note, flexend does not return.  It exits with its argument
	 * as status.
	 */
	flexend (gv, 0);

	return 0;		/* keep compilers/lint happy */
}

/* Wrapper around flex_main, so flex_main can be built as a library. */
int main (int argc, char *argv[])
{
#if defined(ENABLE_NLS) && ENABLE_NLS
#if HAVE_LOCALE_H
	setlocale (LC_MESSAGES, "");
        setlocale (LC_CTYPE, "");
	textdomain (PACKAGE);
	bindtextdomain (PACKAGE, LOCALEDIR);
#endif
#endif

	return flex_main (argc, argv);
}

/* Set up the output filter chain. */

void initialize_output_filters(FlexState* gv)
{
	const char * m4 = NULL;

	gv->output_chain = filter_create_int(gv, NULL, filter_tee_header, gv->env.headerfilename);
	if ( !(m4 = getenv("M4"))) {
		m4 = M4;
	}
        printf("/*==Stack bug? ==*/\n");        
	filter_create_ext(gv, gv->output_chain, m4, "-P");
	filter_create_int(gv, gv->output_chain, filter_fix_linedirs, NULL);

	/* For debugging, only run the requested number of filters. */
	if (gv->preproc_level > 0) {
		filter_truncate(gv->output_chain, gv->preproc_level);
		filter_apply_chain(gv, gv->output_chain);
	}
}


/* check_options - check user-specified options */

void check_options (FlexState* gv)
{
	int     i;

	if (gv->ctrl.lex_compat) {
		if (gv->ctrl.C_plus_plus)
			flexerror (gv, _("Can't use -+ with -l option"));

		if (gv->ctrl.fulltbl || gv->ctrl.fullspd)
			flexerror (gv, _("Can't use -f or -F with -l option"));

		if (gv->ctrl.reentrant || gv->ctrl.bison_bridge_lval)
			flexerror (gv, _
				   ("Can't use --ctrl.reentrant or --bison-bridge with -l option"));

		gv->ctrl.yytext_is_array = true;
		gv->ctrl.do_yylineno = true;
		gv->ctrl.use_read = false;
	}


#if 0
	/* This makes no sense whatsoever. I'm removing it. */
	if (gv->ctrl.do_yylineno)
		/* This should really be "maintain_backup_tables = true" */
		gv->ctrl.reject_really_used = true;
#endif

	if (gv->ctrl.csize == trit_unspecified) {
		if ((gv->ctrl.fulltbl || gv->ctrl.fullspd) && !gv->ctrl.useecs)
			gv->ctrl.csize = DEFAULT_CSIZE;
		else
			gv->ctrl.csize = CSIZE;
	}

	if (gv->ctrl.interactive == trit_unspecified) {
		if (gv->ctrl.fulltbl || gv->ctrl.fullspd)
			gv->ctrl.interactive = trit_false;
		else
			gv->ctrl.interactive = trit_true;
	}

	if (gv->ctrl.fulltbl || gv->ctrl.fullspd) {
		if (gv->ctrl.usemecs)
			flexerror (gv, _
				   ("-Cf/-CF and -Cm don't make sense together"));

		if (gv->ctrl.interactive != trit_false)
			flexerror (gv, _("-Cf/-CF and -I are incompatible"));

		if (gv->ctrl.lex_compat)
			flexerror (gv, _
				   ("-Cf/-CF are incompatible with lex-compatibility mode"));


		if (gv->ctrl.fulltbl && gv->ctrl.fullspd)
			flexerror (gv, _
				   ("-Cf and -CF are mutually exclusive"));
	}

	if (gv->ctrl.C_plus_plus && gv->ctrl.fullspd)
		flexerror (gv, _("Can't use -+ with -CF option"));

	if (gv->ctrl.C_plus_plus && gv->ctrl.yytext_is_array) {
		lwarn (gv, _("%array incompatible with -+ option"));
		gv->ctrl.yytext_is_array = false;
	}

	if (gv->ctrl.C_plus_plus && (gv->ctrl.reentrant))
		flexerror (gv, _("Options -+ and --reentrant are mutually exclusive."));

	if (gv->ctrl.C_plus_plus && gv->ctrl.bison_bridge_lval)
		flexerror (gv, _("bison bridge not supported for the C++ scanner."));


	if (gv->ctrl.useecs) {		/* Set up doubly-linked equivalence classes. */

		/* We loop all the way up to ctrl.csize, since ecgroup[ctrl.csize] is
		 * the position used for NUL characters.
		 */
		gv->ecgroup[1] = NIL;

		for (i = 2; i <= gv->ctrl.csize; ++i) {
			gv->ecgroup[i] = i - 1;
			gv->nextecm[i - 1] = i;
		}

		gv->nextecm[gv->ctrl.csize] = NIL;
	}

	else {
		/* Put everything in its own equivalence class. */
		for (i = 1; i <= gv->ctrl.csize; ++i) {
			gv->ecgroup[i] = i;
			gv->nextecm[i] = BAD_SUBSCRIPT;	/* to catch errors */
		}
	}

	if (!gv->env.use_stdout) {
		FILE   *prev_stdout;

		if (!gv->env.did_outfilename) {
			snprintf (gv->outfile_path, sizeof(gv->outfile_path), gv->outfile_template,
				  gv->ctrl.prefix, suffix(gv));

			gv->env.outfilename = gv->outfile_path;
		}

		prev_stdout = freopen (gv->env.outfilename, "w+", stdout);

		if (prev_stdout == NULL)
			lerr (gv, _("could not create %s"), gv->env.outfilename);

		gv->outfile_created = 1;
	}
}

/* flexend - terminate flex
 *
 * note
 *    This routine does not return.
 */

void flexend (FlexState* gv, int exit_status)
{
	//static int called_before = -1;	/* prevent infinite recursion. */
	int     tblsiz;

	if (++gv->called_before)
		FLEX_EXIT (exit_status);

	if (gv->ctrl.yyclass != NULL && !gv->ctrl.C_plus_plus)
		flexerror (gv, _("%option yyclass only meaningful for C++ scanners"));

	if (gv->skelfile != NULL) {
		if (ferror (gv->skelfile))
			lerr (gv, _("input error reading skeleton file %s"),
				gv->env.skelname);

		else if (fclose (gv->skelfile))
			lerr (gv, _("error closing skeleton file %s"),
				gv->env.skelname);
	}

	if (exit_status != 0 && gv->outfile_created) {
		if (ferror (stdout))
			lerr (gv, _("error writing output file %s"),
				gv->env.outfilename);

		else if ((gv->_stdout_closed = 1) && fclose (stdout))
			lerr (gv, _("error closing output file %s"),
				gv->env.outfilename);

		else if (unlink (gv->env.outfilename))
			lerr (gv, _("error deleting output file %s"),
				gv->env.outfilename);
	}


	if (gv->env.backing_up_report && gv->ctrl.backing_up_file) {
		if (gv->num_backing_up == 0)
			fprintf (gv->ctrl.backing_up_file, _("No backing up.\n"));
		else if (gv->ctrl.fullspd || gv->ctrl.fulltbl)
			fprintf (gv->ctrl.backing_up_file,
				 _
				 ("%d backing up (non-accepting) states.\n"),
				 gv->num_backing_up);
		else
			fprintf (gv->ctrl.backing_up_file,
				 _("Compressed tables always back up.\n"));

		if (ferror (gv->ctrl.backing_up_file))
			lerr (gv, _("error writing backup file %s"),
				gv->backing_name);

		else if (fclose (gv->ctrl.backing_up_file))
			lerr (gv, _("error closing backup file %s"),
				gv->backing_name);
	}

	if (gv->env.printstats) {
		fprintf (stderr, _("%s version %s usage statistics:\n"),
			 gv->program_name, flex_version);

		fprintf (stderr, _("  scanner options: -"));

		if (gv->ctrl.C_plus_plus)
			putc ('+', stderr);
		if (gv->env.backing_up_report)
			putc ('b', stderr);
		if (gv->ctrl.ddebug)
			putc ('d', stderr);
		if (sf_case_ins())
			putc ('i', stderr);
		if (gv->ctrl.lex_compat)
			putc ('l', stderr);
		if (gv->ctrl.posix_compat)
			putc ('X', stderr);
		if (gv->env.performance_hint > 0)
			putc ('p', stderr);
		if (gv->env.performance_hint > 1)
			putc ('p', stderr);
		if (gv->ctrl.spprdflt)
			putc ('s', stderr);
		if (gv->ctrl.reentrant)
			fputs ("--reentrant", stderr);
        if (gv->ctrl.bison_bridge_lval)
            fputs ("--bison-bridge", stderr);
        if (gv->ctrl.bison_bridge_lloc)
            fputs ("--bison-locations", stderr);
		if (gv->env.use_stdout)
			putc ('t', stderr);
		if (gv->env.printstats)
			putc ('v', stderr);	/* always true! */
		if (gv->env.nowarn)
			putc ('w', stderr);
		if (gv->ctrl.interactive == trit_false)
			putc ('B', stderr);
		if (gv->ctrl.interactive == trit_true)
			putc ('I', stderr);
		if (!gv->ctrl.gen_line_dirs)
			putc ('L', stderr);
		if (gv->env.trace)
			putc ('T', stderr);

		if (gv->ctrl.csize == trit_unspecified)
			/* We encountered an error fairly early on, so ctrl.csize
			 * never got specified.  Define it now, to prevent
			 * bogus table sizes being written out below.
			 */
			gv->ctrl.csize = 256;

		if (gv->ctrl.csize == 128)
			putc ('7', stderr);
		else
			putc ('8', stderr);

		fprintf (stderr, " -C");

		if (gv->ctrl.long_align)
			putc ('a', stderr);
		if (gv->ctrl.fulltbl)
			putc ('f', stderr);
		if (gv->ctrl.fullspd)
			putc ('F', stderr);
		if (gv->ctrl.useecs)
			putc ('e', stderr);
		if (gv->ctrl.usemecs)
			putc ('m', stderr);
		if (gv->ctrl.use_read)
			putc ('r', stderr);

		if (gv->env.did_outfilename)
			fprintf (stderr, " -o%s", gv->env.outfilename);

		if (gv->env.skelname != NULL)
			fprintf (stderr, " -S%s", gv->env.skelname);

		if (strcmp (gv->ctrl.prefix, "yy"))
			fprintf (stderr, " -P%s", gv->ctrl.prefix);

		putc ('\n', stderr);

		fprintf (stderr, _("  %d/%d NFA states\n"),
			 gv->lastnfa, gv->current_mns);
		fprintf (stderr, _("  %d/%d DFA states (%d words)\n"),
			 gv->lastdfa, gv->current_max_dfas, gv->totnst);
		fprintf (stderr, _("  %d rules\n"),
			 gv->num_rules + gv->num_eof_rules -
			 1 /* - 1 for def. rule */ );

		if (gv->num_backing_up == 0)
			fprintf (stderr, _("  No backing up\n"));
		else if (gv->ctrl.fullspd || gv->ctrl.fulltbl)
			fprintf (stderr,
				 _
				 ("  %d backing-up (non-accepting) states\n"),
				 gv->num_backing_up);
		else
			fprintf (stderr,
				 _
				 ("  Compressed tables always back-up\n"));

		if (gv->bol_needed)
			fprintf (stderr,
				 _("  Beginning-of-line patterns used\n"));

		fprintf (stderr, _("  %d/%d start conditions\n"), gv->lastsc,
			 gv->current_max_scs);
		fprintf (stderr,
			 _
			 ("  %d epsilon states, %d double epsilon states\n"),
			 gv->numeps, gv->eps2);

		if (gv->lastccl == 0)
			fprintf (stderr, _("  no character classes\n"));
		else
			fprintf (stderr,
				 _
				 ("  %d/%d character classes needed %d/%d words of storage, %d reused\n"),
				 gv->lastccl, gv->current_maxccls,
				 gv->cclmap[gv->lastccl] + gv->ccllen[gv->lastccl],
				 gv->current_max_ccl_tbl_size, gv->cclreuse);

		fprintf (stderr, _("  %d state/nextstate pairs created\n"),
			 gv->numsnpairs);
		fprintf (stderr,
			 _("  %d/%d unique/duplicate transitions\n"),
			 gv->numuniq, gv->numdup);

		if (gv->ctrl.fulltbl) {
			tblsiz = gv->lastdfa * gv->numecs;
			fprintf (stderr, _("  %d table entries\n"),
				 tblsiz);
		}

		else {
			tblsiz = 2 * (gv->lastdfa + gv->numtemps) + 2 * gv->tblend;

			fprintf (stderr,
				 _("  %d/%d base-def entries created\n"),
				 gv->lastdfa + gv->numtemps, gv->current_max_dfas);
			fprintf (stderr,
				 _
				 ("  %d/%d (peak %d) nxt-chk entries created\n"),
				 gv->tblend, gv->current_max_xpairs, gv->peakpairs);
			fprintf (stderr,
				 _
				 ("  %d/%d (peak %d) template nxt-chk entries created\n"),
				 gv->numtemps * gv->nummecs,
				 gv->current_max_template_xpairs,
				 gv->numtemps * gv->numecs);
			fprintf (stderr, _("  %d empty table entries\n"),
				 gv->nummt);
			fprintf (stderr, _("  %d protos created\n"),
				 gv->numprots);
			fprintf (stderr,
				 _("  %d templates created, %d uses\n"),
				 gv->numtemps, gv->tmpuses);
		}

		if (gv->ctrl.useecs) {
			tblsiz = tblsiz + gv->ctrl.csize;
			fprintf (stderr,
				 _
				 ("  %d/%d equivalence classes created\n"),
				 gv->numecs, gv->ctrl.csize);
		}

		if (gv->ctrl.usemecs) {
			tblsiz = tblsiz + gv->numecs;
			fprintf (stderr,
				 _
				 ("  %d/%d meta-equivalence classes created\n"),
				 gv->nummecs, gv->ctrl.csize);
		}

		fprintf (stderr,
			 _
			 ("  %d (%d saved) hash collisions, %d DFAs equal\n"),
			 gv->hshcol, gv->hshsave, gv->dfaeql);
		fprintf (stderr, _("  %d sets of reallocations needed\n"),
			 gv->num_reallocs);
		fprintf (stderr, _("  %d total table entries needed\n"),
			 tblsiz);
	}

	FLEX_EXIT (exit_status);
}


/* flexinit - initialize flex */

void flexinit (FlexState* gv, int argc, char **argv)
{
	int     i, sawcmpflag, rv, optind;
	char   *arg;
	scanopt_t sopt;

	memset(&gv->ctrl, '\0', sizeof(gv->ctrl));
	gv->syntaxerror = false;
	gv->yymore_used = gv->continued_action = false;
	gv->in_rule = gv->reject = false;
	gv->ctrl.yymore_really_used = gv->ctrl.reject_really_used = trit_unspecified;

	gv->ctrl.do_main = trit_unspecified;
	gv->ctrl.interactive = gv->ctrl.csize = trit_unspecified;
	gv->ctrl.do_yywrap = gv->ctrl.gen_line_dirs = gv->ctrl.usemecs = gv->ctrl.useecs = true;
	gv->ctrl.reentrant = gv->ctrl.bison_bridge_lval = gv->ctrl.bison_bridge_lloc = false;
	gv->env.performance_hint = 0;
	gv->ctrl.prefix = "yy";
	gv->ctrl.rewrite = false;
	gv->ctrl.yylmax = BUFSIZ;

	gv->tablesext = gv->tablesverify = false;
	gv->gentables = true;
	gv->tablesfilename = gv->tablesname = NULL;

	sawcmpflag = false;
	
	/* Initialize dynamic array for holding the rule actions. */
	gv->action_size = 2048;	/* default size of action array in bytes */
	gv->action_array = allocate_character_array (gv->action_size);
	gv->defs1_offset = gv->prolog_offset = gv->action_offset = gv->action_index = 0;
	gv->action_array[0] = '\0';

	/* Initialize any buffers. */
	buf_init (&gv->userdef_buf, sizeof (char));	/* one long string */
	buf_init (&gv->top_buf, sizeof (char));	    /* one long string */

	sf_init (gv);

	/* Enable C++ if program name ends with '+'. */
	gv->program_name = argv[0];

	if (gv->program_name != NULL &&
	    gv->program_name[strlen (gv->program_name) - 1] == '+')
		gv->ctrl.C_plus_plus = true;

	/* read flags */
	sopt = scanopt_init (flexopts, argc, argv, 0);
	if (!sopt) {
		/* This will only happen when flexopts array is altered. */
		fprintf (stderr,
			 _("Internal error. flexopts are malformed.\n"));
		FLEX_EXIT (1);
	}

	while ((rv = scanopt (sopt, &arg, &optind)) != 0) {

		if (rv < 0) {
			/* Scanopt has already printed an option-specific error message. */
			fprintf (stderr,
				 _
				 ("Try `%s --help' for more information.\n"),
				 gv->program_name);
			FLEX_EXIT (1);
		}

		switch ((enum flexopt_flag_t) rv) {
		    case OPT_CPLUSPLUS:
			gv->ctrl.C_plus_plus = true;
			break;

		    case OPT_BATCH:
			gv->ctrl.interactive = trit_false;
			break;

		    case OPT_BACKUP:
			gv->env.backing_up_report = trit_true;
			break;

		    case OPT_BACKUP_FILE:
			gv->env.backing_up_report = true;
                        gv->backing_name = arg;
			break;

		    case OPT_DONOTHING:
			break;

		    case OPT_COMPRESSION:
			if (!sawcmpflag) {
				gv->ctrl.useecs = false;
				gv->ctrl.usemecs = false;
				gv->ctrl.fulltbl = false;
				sawcmpflag = true;
			}

			for (i = 0; arg && arg[i] != '\0'; i++)
				switch (arg[i]) {
				    case 'a':
					gv->ctrl.long_align = true;
					break;

				    case 'e':
					gv->ctrl.useecs = true;
					break;

				    case 'F':
					gv->ctrl.fullspd = true;
					break;

				    case 'f':
					gv->ctrl.fulltbl = true;
					break;

				    case 'm':
					gv->ctrl.usemecs = true;
					break;

				    case 'r':
					gv->ctrl.use_read = true;
					break;

				    default:
					lerr (gv, _
					      ("unknown -C option '%c'"),
					      arg[i]);
					break;
				}
			break;

		    case OPT_DEBUG:
			gv->ctrl.ddebug = true;
			break;

		    case OPT_NO_DEBUG:
			gv->ctrl.ddebug = false;
			break;

		    case OPT_FULL:
			gv->ctrl.useecs = gv->ctrl.usemecs = false;
			gv->ctrl.use_read = gv->ctrl.fulltbl = true;
			break;

		    case OPT_FAST:
			gv->ctrl.useecs = gv->ctrl.usemecs = false;
			gv->ctrl.use_read = gv->ctrl.fullspd = true;
			break;

		    case OPT_HELP:
			usage (gv);
			FLEX_EXIT (0);

		    case OPT_INTERACTIVE:
			gv->ctrl.interactive = true;
			break;

		    case OPT_CASE_INSENSITIVE:
			sf_set_case_ins(true);
			break;

		    case OPT_LEX_COMPAT:
			gv->ctrl.lex_compat = true;
			break;

		    case OPT_POSIX_COMPAT:
			gv->ctrl.posix_compat = true;
			break;

		    case OPT_PREPROC_LEVEL:
			gv->preproc_level = (int) strtol(arg,NULL,0);
			break;

		    case OPT_MAIN:
			gv->ctrl.do_yywrap = false;
			gv->ctrl.do_main = trit_true;
			break;

		    case OPT_NO_MAIN:
			gv->ctrl.do_main = trit_false;
			break;

		    case OPT_NO_LINE:
			gv->ctrl.gen_line_dirs = false;
			break;

		    case OPT_OUTFILE:
			gv->env.outfilename = arg;
			gv->env.did_outfilename = 1;
			break;

		    case OPT_PREFIX:
			gv->ctrl.prefix = arg;
			break;

		    case OPT_PERF_REPORT:
			++gv->env.performance_hint;
			break;

		    case OPT_BISON_BRIDGE:
			gv->ctrl.bison_bridge_lval = true;
			break;

		    case OPT_BISON_BRIDGE_LOCATIONS:
			gv->ctrl.bison_bridge_lval = gv->ctrl.bison_bridge_lloc = true;
			break;

		    case OPT_REENTRANT:
			gv->ctrl.reentrant = true;
			break;

		    case OPT_NO_REENTRANT:
			gv->ctrl.reentrant = false;
			break;

		    case OPT_SKEL:
			gv->env.skelname = arg;
			break;

		    case OPT_DEFAULT:
			gv->ctrl.spprdflt = false;
			break;

		    case OPT_NO_DEFAULT:
			gv->ctrl.spprdflt = true;
			break;

		    case OPT_STDOUT:
			gv->env.use_stdout = true;
			break;

		    case OPT_NO_UNISTD_H:
			gv->ctrl.no_unistd = true;
			break;

		    case OPT_TABLES_FILE:
			gv->tablesext = true;
			gv->tablesfilename = arg;
			break;

		    case OPT_TABLES_VERIFY:
			gv->tablesverify = true;
			break;

		    case OPT_TRACE:
			gv->env.trace = true;
			break;

		    case OPT_VERBOSE:
			gv->env.printstats = true;
			break;

		    case OPT_VERSION:
			printf ("%s %s\n", (gv->ctrl.C_plus_plus ? "flex++" : "flex"), flex_version);
			FLEX_EXIT (0);

		    case OPT_WARN:
			gv->env.nowarn = false;
			break;

		    case OPT_NO_WARN:
			gv->env.nowarn = true;
			break;

		    case OPT_7BIT:
			gv->ctrl.csize = 128;
			break;

		    case OPT_8BIT:
			gv->ctrl.csize = CSIZE;
			break;

		    case OPT_ALIGN:
			gv->ctrl.long_align = true;
			break;

		    case OPT_NO_ALIGN:
			gv->ctrl.long_align = false;
			break;

		    case OPT_ALWAYS_INTERACTIVE:
			gv->ctrl.always_interactive = true;
			break;

		    case OPT_NEVER_INTERACTIVE:
			gv->ctrl.never_interactive = true;
			break;

		    case OPT_ARRAY:
			gv->ctrl.yytext_is_array = true;
			break;

		    case OPT_POINTER:
			gv->ctrl.yytext_is_array = false;
			break;

		    case OPT_ECS:
			gv->ctrl.useecs = true;
			break;

		    case OPT_NO_ECS:
			gv->ctrl.useecs = false;
			break;

		    case OPT_EMIT:
			gv->ctrl.emit = arg;
			break;

		    case OPT_HEADER_FILE:
			gv->env.headerfilename = arg;
			break;

		    case OPT_META_ECS:
			gv->ctrl.usemecs = true;
			break;

		    case OPT_NO_META_ECS:
			gv->ctrl.usemecs = false;
			break;

		    case OPT_PREPROCDEFINE:
		    {
			    /* arg is "symbol" or "symbol=definition". */
			    char   *def;
			    char buf2[4096];

			    for (def = arg;
				 *def != '\0' && *def != '='; ++def)
				    continue;
			    if (*def == '\0')
				    def = "1";

			    snprintf(buf2, sizeof(buf2), "M4_HOOK_CONST_DEFINE_UNKNOWN(%s, %s)", arg, def);
			    buf_strappend (gv, &gv->userdef_buf, buf2);
		    }
		    break;

		    case OPT_READ:
			gv->ctrl.use_read = true;
			break;

		    case OPT_STACK:
			gv->ctrl.stack_used = true;
			break;

		    case OPT_STDINIT:
			gv->ctrl.do_stdinit = true;
			break;

		    case OPT_NO_STDINIT:
			gv->ctrl.do_stdinit = false;
			break;

		    case OPT_YYCLASS:
			gv->ctrl.yyclass = arg;
			break;

		    case OPT_YYLINENO:
			gv->ctrl.do_yylineno = true;
			break;

		    case OPT_NO_YYLINENO:
			gv->ctrl.do_yylineno = false;
			break;

		    case OPT_YYWRAP:
			gv->ctrl.do_yywrap = true;
			break;

		    case OPT_NO_YYWRAP:
			gv->ctrl.do_yywrap = false;
			break;

		    case OPT_YYMORE:
			gv->ctrl.yymore_really_used = true;
			break;

		    case OPT_NO_YYMORE:
			gv->ctrl.yymore_really_used = false;
			break;

		    case OPT_REJECT:
			gv->ctrl.reject_really_used = true;
			break;

		    case OPT_NO_REJECT:
			gv->ctrl.reject_really_used = false;
			break;

		    case OPT_NO_YY_PUSH_STATE:
			gv->ctrl.no_yy_push_state = true;
			break;
		    case OPT_NO_YY_POP_STATE:
			gv->ctrl.no_yy_pop_state = true;
			break;
		    case OPT_NO_YY_TOP_STATE:
			gv->ctrl.no_yy_top_state = true;
			break;
		    case OPT_NO_YYINPUT:
			gv->ctrl.no_yyinput = true;
			break;
		    case OPT_NO_YYUNPUT:
			gv->ctrl.no_yyunput = true;
			break;
		    case OPT_NO_YY_SCAN_BUFFER:
			gv->ctrl.no_yy_scan_buffer = true;
			break;
		    case OPT_NO_YY_SCAN_BYTES:
			gv->ctrl.no_yy_scan_bytes = true;
			break;
		    case OPT_NO_YY_SCAN_STRING:
			gv->ctrl.no_yy_scan_string = true;
			break;
		    case OPT_NO_YYGET_EXTRA:
			gv->ctrl.no_yyget_extra = true;
			break;
		    case OPT_NO_YYSET_EXTRA:
			gv->ctrl.no_yyset_extra = true;
			break;
		    case OPT_NO_YYGET_LENG:
			gv->ctrl.no_yyget_leng = true;
			break;
		    case OPT_NO_YYGET_TEXT:
			gv->ctrl.no_yyget_text = true;
			break;
		    case OPT_NO_YYGET_LINENO:
			gv->ctrl.no_yyget_lineno = true;
			break;
		    case OPT_NO_YYSET_LINENO:
			gv->ctrl.no_yyset_lineno = true;
			break;
		    case OPT_NO_YYGET_COLUMN:
			gv->ctrl.no_yyget_column = true;
			break;
		    case OPT_NO_YYSET_COLUMN:
			gv->ctrl.no_yyset_column = true;
			break;
		    case OPT_NO_YYGET_IN:
			gv->ctrl.no_yyget_in = true;
			break;
		    case OPT_NO_YYSET_IN:
			gv->ctrl.no_yyset_in = true;
			break;
		    case OPT_NO_YYGET_OUT:
			gv->ctrl.no_yyget_out = true;
			break;
		    case OPT_NO_YYSET_OUT:
			gv->ctrl.no_yyset_out = true;
			break;
		    case OPT_NO_YYGET_LVAL:
			gv->ctrl.no_yyget_lval = true;
			break;
		    case OPT_NO_YYSET_LVAL:
			gv->ctrl.no_yyset_lval = true;
			break;
		    case OPT_NO_YYGET_LLOC:
			gv->ctrl.no_yyget_lloc = true;
			break;
		    case OPT_NO_YYSET_LLOC:
			gv->ctrl.no_yyset_lloc = true;
			break;
		    case OPT_NO_YYGET_DEBUG:
			gv->ctrl.no_get_debug = true;
			break;
		    case OPT_NO_YYSET_DEBUG:
			gv->ctrl.no_set_debug = true;
			break;

		    case OPT_HEX:
			gv->env.trace_hex = true;
                        break;
		    case OPT_NO_SECT3_ESCAPE:
                        gv->ctrl.no_section3_escape = true;
                        break;
		}		/* switch */
	}			/* while scanopt() */

	scanopt_destroy (sopt);

	gv->num_input_files = argc - optind;
	gv->input_files = argv + optind;
	set_input_file (gv->scanner, gv->num_input_files > 0 ? gv->input_files[0] : NULL);

	gv->lastccl = gv->lastsc = gv->lastdfa = gv->lastnfa = 0;
	gv->num_rules = gv->num_eof_rules = gv->default_rule = 0;
	gv->numas = gv->numsnpairs = gv->tmpuses = 0;
	gv->numecs = gv->numeps = gv->eps2 = gv->num_reallocs = gv->hshcol = gv->dfaeql = gv->totnst =
	    0;
	gv->numuniq = gv->numdup = gv->hshsave = gv->datapos = gv->dataline = 0;
	gv->eofseen = false;
	gv->num_backing_up = gv->onesp = gv->numprots = 0;
	gv->variable_trailing_context_rules = gv->bol_needed = false;

	gv->linenum = gv->sectnum = 1;
	gv->firstprot = NIL;

	/* Used in mkprot() so that the first proto goes in slot 1
	 * of the proto queue.
	 */
	gv->lastprot = 1;

	set_up_initial_allocations (gv);
}


/* readin - read in the rules section of the input file(s) */

void readin (FlexState* gv)
{
	char buf[256];

	line_directive_out(gv, NULL, gv->infilename, gv->linenum);

	if (yyparse (gv->scanner, gv)) {
		pinpoint_message (gv, _("fatal parse error"));
		flexend (gv, 1);
	}

	if (gv->syntaxerror)
		flexend (gv, 1);

	/* On --emit, -e, or change backends This is where backend
	 * properties are collected, which means they can't be set
	 * from a custom skelfile.  Note: might have been called sooner
	 * when %option emit was evaluated; this catches command-line
	 * optiins and the default case.
	 */
	backend_by_name(gv, gv->ctrl.emit);

	initialize_output_filters(gv);

        yyset_out(stdout, gv->scanner);

	if (gv->tablesext)
		gv->gentables = false;

	if (gv->tablesverify)
		/* force generation of C tables. */
		gv->gentables = true;


	if (gv->tablesext) {
		FILE   *tablesout;
		struct yytbl_hdr hdr;
		char   *pname = 0;
		size_t  nbytes = 0;

		if (!gv->tablesfilename) {
			nbytes = strlen (gv->ctrl.prefix) + strlen (gv->tablesfile_template) + 2;
			gv->tablesfilename = pname = calloc(nbytes, 1);
			snprintf (pname, nbytes, gv->tablesfile_template, gv->ctrl.prefix);
		}

		if ((tablesout = fopen (gv->tablesfilename, "w")) == NULL)
			lerr (gv, _("could not create %s"), gv->tablesfilename);
		free(pname);
		gv->tablesfilename = 0;

		yytbl_writer_init (&gv->tableswr, tablesout);

		nbytes = strlen (gv->ctrl.prefix) + strlen ("tables") + 2;
		gv->tablesname = calloc(nbytes, 1);
		snprintf (gv->tablesname, nbytes, "%stables", gv->ctrl.prefix);
		yytbl_hdr_init (gv, &hdr, flex_version, gv->tablesname);

		if (yytbl_hdr_fwrite (gv, &gv->tableswr, &hdr) <= 0)
			flexerror (gv, _("could not write tables header"));
	}

	if (gv->env.skelname && (gv->skelfile = fopen (gv->env.skelname, "r")) == NULL)
		lerr (gv, _("can't open skeleton file %s"), gv->env.skelname);

	if (strchr(gv->ctrl.prefix, '[') || strchr(gv->ctrl.prefix, ']'))
		flexerror(gv, _("Prefix cannot include '[' or ']'"));

	if (gv->env.did_outfilename)
		line_directive_out (gv, stdout, NULL, gv->linenum);

	/* This is where we begin writing to the file. */

	skelout(gv, false);	/* [0.0] Make hook macros available, silently */

	comment("A lexical scanner generated by flex\n");

	/* Dump the %top code. */
	if( gv->top_buf.elts)
		outn((char*) gv->top_buf.elts);

	/* Place a bogus line directive, it will be fixed in the filter. */
	line_directive_out(gv, NULL, NULL, 0);

	/* User may want to set the scanner prototype */
	if (gv->ctrl.yydecl != NULL) {
		out_str ("M4_HOOK_SET_YY_DECL(%s)\n", gv->ctrl.yydecl);
	}

	if (gv->ctrl.userinit != NULL) {
		out_str ("M4_HOOK_SET_USERINIT(%s)\n", gv->ctrl.userinit);
	}
	if (gv->ctrl.preaction != NULL) {
		out_str ("M4_HOOK_SET_PREACTION(%s)\n", gv->ctrl.preaction);
	}
	if (gv->ctrl.postaction != NULL) {
		out_str ("M4_HOOK_SET_POSTACTION(%s)\n", gv->ctrl.postaction);
	}

	/* This has to be a straight textual substitution rather
	 * than a constant declaration because in C a const is
	 * not const enough to be a static array bound.
	 */
	out_dec ("m4_define([[YYLMAX]], [[%d]])\n", gv->ctrl.yylmax);

	/* Dump the user defined preproc directives. */
	if (gv->userdef_buf.elts)
		outn ((char *) (gv->userdef_buf.elts));

	/* If the user explicitly requested posix compatibility by specifying the
	 * posix-compat option, then we check for conflicting options. However, if
	 * the POSIXLY_CORRECT variable is set, then we quietly make flex as
	 * posix-compatible as possible.  This is the recommended behavior
	 * according to the GNU Coding Standards.
	 *
	 * Note: The posix option was added to flex to provide the posix behavior
	 * of the repeat operator in regular expressions, e.g., `ab{3}'
	 */
	if (gv->ctrl.posix_compat) {
		/* TODO: This is where we try to make flex behave according to
		 * POSIX, *and* check for conflicting ctrl. How far should we go
		 * with this? Should we disable all the neat-o flex features?
		 */
		/* Update: Estes says no, since other flex features don't violate posix. */
	}

	if (getenv ("POSIXLY_CORRECT")) {
		gv->ctrl.posix_compat = true;
	}

	if (gv->env.backing_up_report) {
		gv->ctrl.backing_up_file = fopen (gv->backing_name, "w");
		if (gv->ctrl.backing_up_file == NULL)
			lerr (gv, _
				("could not create backing-up info file %s"),
				gv->backing_name);
	}

	else
		gv->ctrl.backing_up_file = NULL;

	if (gv->ctrl.yymore_really_used == true)
		gv->yymore_used = true;
	else if (gv->ctrl.yymore_really_used == false)
		gv->yymore_used = false;

	if (gv->ctrl.reject_really_used == true)
		gv->reject = true;
	else if (gv->ctrl.reject_really_used == false)
		gv->reject = false;

	if (gv->env.performance_hint > 0) {
		if (gv->ctrl.lex_compat) {
			fprintf (stderr,
				 _
				 ("-l AT&T lex compatibility option entails a large performance penalty\n"));
			fprintf (stderr,
				 _
				 (" and may be the actual source of other reported performance penalties\n"));
		}

		else if (gv->ctrl.do_yylineno) {
			fprintf (stderr,
				 _
				 ("%%option yylineno entails a performance penalty ONLY on rules that can match newline characters\n"));
		}

		if (gv->env.performance_hint > 1) {
			if (gv->ctrl.interactive == trit_true)
				fprintf (stderr,
					 _
					 ("-I (interactive) entails a minor performance penalty\n"));

			if (gv->yymore_used)
				fprintf (stderr,
					 _
					 ("yymore() entails a minor performance penalty\n"));
		}

		if (gv->reject)
			fprintf (stderr,
				 _
				 ("REJECT entails a large performance penalty\n"));

		if (gv->variable_trailing_context_rules)
			fprintf (stderr,
				 _
				 ("Variable trailing context rules entail a large performance penalty\n"));
	}

	if (gv->reject)
		gv->real_reject = true;

	if (gv->variable_trailing_context_rules)
		gv->reject = true;

	if ((gv->ctrl.fulltbl || gv->ctrl.fullspd) && gv->reject) {
		if (gv->real_reject)
			flexerror (gv, _
				   ("REJECT cannot be used with -f or -F"));
		else if (gv->ctrl.do_yylineno)
			flexerror (gv, _
				   ("%option yylineno cannot be used with REJECT"));
		else
			flexerror (gv, _
				   ("variable trailing context rules cannot be used with -f or -F"));
	}

	if (gv->ctrl.useecs)
		gv->numecs = cre8ecs (gv->nextecm, gv->ecgroup, gv->ctrl.csize);
	else
		gv->numecs = gv->ctrl.csize;

	/* Now map the equivalence class for NUL to its expected place. */
	gv->ecgroup[0] = gv->ecgroup[gv->ctrl.csize];
	gv->NUL_ec = ABS (gv->ecgroup[0]);

	if (gv->ctrl.useecs)
		ccl2ecl (gv);

	// These are used to conditionalize code in the lex skeleton
	// that historically used to be generated by C code in flex
	// itself; by shoving all this stuff out to the skeleton file
	// we make it easier to retarget the code generation.
	snprintf(buf, sizeof(buf), "Target: %s\n", gv->ctrl.backend_name);
	comment(buf);
	comment("START of m4 controls\n");

	/* Define the start condition macros. */
	{
		struct Buf tmpbuf;
		int i;
		buf_init(&tmpbuf, sizeof(char));
		for (i = 1; i <= gv->lastsc; i++) {
			char *str, *fmt = "M4_HOOK_CONST_DEFINE_STATE(%s, %d)";
			size_t strsz;

			strsz = strlen(fmt) + strlen(gv->scname[i]) + (size_t)(1 + ceil (log10(i))) + 2;
			str = malloc(strsz);
			if (!str)
				flexfatal(gv, _("allocation of macro definition failed"));
			snprintf(str, strsz, fmt, gv->scname[i], i - 1);
			buf_strappend(gv, &tmpbuf, str);
			free(str);
		}
		// FIXME: Not dumped visibly because we plan to do away with the indirection
		out_m4_define("M4_YY_SC_DEFS", tmpbuf.elts);
		buf_destroy(&tmpbuf);
	}

	if (gv->ctrl.bison_bridge_lval)
		visible_define("M4_YY_BISON_LVAL");

	if (gv->ctrl.bison_bridge_lloc)
		visible_define("<M4_YY_BISON_LLOC>");

	if (gv->extra_type != NULL)
		visible_define_str ("M4_MODE_EXTRA_TYPE", gv->extra_type);

	/* always generate the tablesverify flag. */
	visible_define_str ("M4_YY_TABLES_VERIFY", gv->tablesverify ? "1" : "0");

	if (gv->ctrl.reentrant) {
		visible_define ("M4_YY_REENTRANT");
		if (gv->ctrl.yytext_is_array)
			visible_define ("M4_MODE_REENTRANT_TEXT_IS_ARRAY");
	}

	if (gv->ctrl.do_main == trit_true)
		visible_define_str ( "YY_MAIN", "1");
	else if (gv->ctrl.do_main == trit_false)
		visible_define_str ( "YY_MAIN", "0");

	if (gv->ctrl.do_stdinit)
		visible_define ( "M4_MODE_DO_STDINIT");
	else
		visible_define ( "M4_MODE_NO_DO_STDINIT");

	// mode switches for YY_DO_BEFORE_ACTION code generation
	if (gv->ctrl.yytext_is_array)
		visible_define ( "M4_MODE_YYTEXT_IS_ARRAY");
	else
		visible_define ( "M4_MODE_NO_YYTEXT_IS_ARRAY");
	if (gv->yymore_used)
		visible_define ( "M4_MODE_YYMORE_USED");
	else
		visible_define ( "M4_MODE_NO_YYMORE_USED");

	if (gv->ctrl.fullspd)
		visible_define ( "M4_MODE_REAL_FULLSPD");
	else
		visible_define ( "M4_MODE_NO_REAL_FULLSPD");

	if (gv->ctrl.fulltbl)
		visible_define ( "M4_MODE_REAL_FULLTBL");
	else
		visible_define ( "M4_MODE_NO_REAL_FULLTBL");

	// niode switches for YYINPUT code generation
	if (gv->ctrl.use_read)
		visible_define ( "M4_MODE_CPP_USE_READ");
	else
		visible_define ( "M4_MODE_NO_CPP_USE_READ");

	// mode switches for next-action code
	if (gv->variable_trailing_context_rules) {
		visible_define ( "M4_MODE_VARIABLE_TRAILING_CONTEXT_RULES");
	} else {
		visible_define ( "M4_MODE_NO_VARIABLE_TRAILING_CONTEXT_RULES");
	}
	if (gv->real_reject)
		visible_define ( "M4_MODE_REAL_REJECT");
	if (gv->ctrl.reject_really_used)
		visible_define ( "M4_MODE_FIND_ACTION_REJECT_REALLY_USED");
	if (gv->reject)
		visible_define ( "M4_MODE_USES_REJECT");
	else
		visible_define ( "M4_MODE_NO_USES_REJECT");

	// mode switches for computing next compressed state
	if (gv->ctrl.usemecs)
		visible_define ( "M4_MODE_USEMECS");

	// mode switches for find-action code
	if (gv->ctrl.fullspd)
		visible_define ( "M4_MODE_FULLSPD");
	else if (gv->ctrl.fulltbl)
	    visible_define ( "M4_MODE_FIND_ACTION_FULLTBL");
	else if (gv->reject)
	    visible_define ( "M4_MODE_FIND_ACTION_REJECT");
	else
	    visible_define ( "M4_MODE_FIND_ACTION_COMPRESSED");

	// mode switches for backup generation and gen_start_state
	if (!gv->ctrl.fullspd)
		visible_define ( "M4_MODE_NO_FULLSPD");
	if (gv->bol_needed)
		visible_define ( "M4_MODE_BOL_NEEDED");
	else
		visible_define ( "M4_MODE_NO_BOL_NEEDED");

	// yylineno
	if (gv->ctrl.do_yylineno)
		visible_define ( "M4_MODE_YYLINENO");

	// Equivalence classes
	if (gv->ctrl.useecs)
		visible_define ( "M4_MODE_USEECS");
	else
		visible_define ( "M4_MODE_NO_USEECS");

	// mode switches for getting next action
	if (gv->gentables)
		visible_define ( "M4_MODE_GENTABLES");
	else
		visible_define ( "M4_MODE_NO_GENTABLES");
	if (gv->ctrl.interactive == trit_true)
		visible_define ( "M4_MODE_INTERACTIVE");
	else
		visible_define ( "M4_MODE_NO_INTERACTIVE");
	if (!(gv->ctrl.fullspd || gv->ctrl.fulltbl))
		visible_define ( "M4_MODE_NO_FULLSPD_OR_FULLTBL");
	if (gv->reject || gv->ctrl.interactive == trit_true)
		visible_define ( "M4_MODE_FIND_ACTION_REJECT_OR_INTERACTIVE");

	if (gv->ctrl.yyclass != NULL) {
		visible_define ( "M4_MODE_YYCLASS");
		out_m4_define("M4_YY_CLASS_NAME", gv->ctrl.yyclass);
	}

	if (gv->ctrl.ddebug)
		visible_define ( "M4_MODE_DEBUG");

	if (gv->ctrl.lex_compat)
		visible_define ( "M4_MODE_OPTIONS.LEX_COMPAT");

	if (gv->ctrl.do_yywrap)
		visible_define ( "M4_MODE_YYWRAP");
	else
		visible_define ( "M4_MODE_NO_YYWRAP");

	if (gv->ctrl.interactive == trit_true)
		visible_define ( "M4_MODE_INTERACTIVE");

	if (gv->ctrl.noyyread)
		visible_define("M4_MODE_USER_YYREAD");

	if (is_default_backend()) {
		if (gv->ctrl.C_plus_plus) {
			visible_define ( "M4_MODE_CXX_ONLY");
		} else {
			visible_define ( "M4_MODE_C_ONLY");
		}
	}

	if (gv->tablesext)
		visible_define ( "M4_MODE_TABLESEXT");
	if (gv->ctrl.prefix != NULL)
	    visible_define_str ( "M4_MODE_PREFIX", gv->ctrl.prefix);

	if (gv->ctrl.no_yyinput)
		visible_define("M4_MODE_NO_YYINPUT");

	if (gv->ctrl.bufsize != 0)
	    visible_define_int("M4_MODE_YY_BUFSIZE", gv->ctrl.bufsize);

	if (gv->ctrl.yyterminate != NULL)
	    visible_define_str("M4_MODE_YYTERMINATE", gv->ctrl.yyterminate);
	
	if (gv->ctrl.no_yypanic)
		visible_define("M4_YY_NO_YYPANIC");
	if (gv->ctrl.no_yy_push_state)
		visible_define("M4_YY_NO_PUSH_STATE");
	if (gv->ctrl.no_yy_pop_state)
		visible_define("M4_YY_NO_POP_STATE");
	if (gv->ctrl.no_yy_top_state)
		visible_define("M4_YY_NO_TOP_STATE");
	if (gv->ctrl.no_yyunput)
		visible_define("M4_YY_NO_YYUNPUT");
	if (gv->ctrl.no_yy_scan_buffer)
		visible_define("M4_YY_NO_SCAN_BUFFER");
	if (gv->ctrl.no_yy_scan_bytes)
		visible_define("M4_YY_NO_SCAN_BYTES");
	if (gv->ctrl.no_yy_scan_string)
		visible_define("M4_YY_NO_SCAN_STRING");
	if (gv->ctrl.no_yyget_extra)
		visible_define("M4_YY_NO_GET_EXTRA");
	if (gv->ctrl.no_yyset_extra)
		visible_define("M4_YY_NO_SET_EXTRA");
	if (gv->ctrl.no_yyget_leng)
		visible_define("M4_YY_NO_GET_LENG");
	if (gv->ctrl.no_yyget_text)
		visible_define("M4_YY_NO_GET_TEXT");
	if (gv->ctrl.no_yyget_lineno)
		visible_define("M4_YY_NO_GET_LINENO");
	if (gv->ctrl.no_yyset_lineno)
		visible_define("M4_YY_NO_SET_LINENO");
	if (gv->ctrl.no_yyget_column)
		visible_define("M4_YY_NO_GET_COLUMN");
	if (gv->ctrl.no_yyset_column)
		visible_define("M4_YY_NO_SET_COLUMN");
	if (gv->ctrl.no_yyget_in)
		visible_define("M4_YY_NO_GET_IN");
	if (gv->ctrl.no_yyset_in)
		visible_define("M4_YY_NO_SET_IN");
	if (gv->ctrl.no_yyget_out)
		visible_define("M4_YY_NO_GET_OUT");
	if (gv->ctrl.no_yyset_out)
		visible_define("M4_YY_NO_SET_OUT");
	if (gv->ctrl.no_yyget_lval)
		visible_define("M4_YY_NO_GET_LVAL");
	if (gv->ctrl.no_yyset_lval)
		visible_define("M4_YY_NO_SET_LVAL");
	if (gv->ctrl.no_yyget_lloc)
		visible_define("M4_YY_NO_GET_LLOC");
	if (gv->ctrl.no_yyset_lloc)
		visible_define("M4_YY_NO_SET_LLOC");
	if (gv->ctrl.no_flex_alloc)
		visible_define("M4_YY_NO_FLEX_ALLOC");
	if (gv->ctrl.no_flex_realloc)
		visible_define("M4_YY_NO_FLEX_REALLOC");
	if (gv->ctrl.no_flex_free)
		visible_define("M4_YY_NO_FLEX_FREE");
	if (gv->ctrl.no_get_debug)
		visible_define("M4_YY_NO_GET_DEBUG");
	if (gv->ctrl.no_set_debug)
		visible_define("M4_YY_NO_SET_DEBUG");

	if (gv->ctrl.no_unistd)
		visible_define("M4_YY_NO_UNISTD_H");

	if (gv->ctrl.always_interactive)
		visible_define("M4_YY_ALWAYS_INTERACTIVE");
	if (gv->ctrl.never_interactive)
		visible_define("M4_YY_NEVER_INTERACTIVE");
	if (gv->ctrl.stack_used)
		visible_define("M4_YY_STACK_USED");

	if (gv->ctrl.rewrite)
		visible_define ( "M4_MODE_REWRITE");
	else
		visible_define ( "M4_MODE_NO_REWRITE");

	comment("END of m4 controls\n");
	out ("\n");
}

/* set_up_initial_allocations - allocate memory for internal tables */

void set_up_initial_allocations (FlexState* gv)
{
	gv->maximum_mns = (gv->ctrl.long_align ? MAXIMUM_MNS_LONG : MAXIMUM_MNS);
	gv->current_mns = INITIAL_MNS;
	gv->firstst = allocate_integer_array (gv->current_mns);
	gv->lastst = allocate_integer_array (gv->current_mns);
	gv->finalst = allocate_integer_array (gv->current_mns);
	gv->transchar = allocate_integer_array (gv->current_mns);
	gv->trans1 = allocate_integer_array (gv->current_mns);
	gv->trans2 = allocate_integer_array (gv->current_mns);
	gv->accptnum = allocate_integer_array (gv->current_mns);
	gv->assoc_rule = allocate_integer_array (gv->current_mns);
	gv->state_type = allocate_integer_array (gv->current_mns);

	gv->current_max_rules = INITIAL_MAX_RULES;
	gv->rule_type = allocate_integer_array (gv->current_max_rules);
	gv->rule_linenum = allocate_integer_array (gv->current_max_rules);
	gv->rule_useful = allocate_array(gv, gv->current_max_rules, sizeof(char));
	gv->rule_has_nl = allocate_array(gv, gv->current_max_rules, sizeof(char));

	gv->current_max_scs = INITIAL_MAX_SCS;
	gv->scset = allocate_integer_array (gv->current_max_scs);
	gv->scbol = allocate_integer_array (gv->current_max_scs);
	gv->scxclu = allocate_array(gv, gv->current_max_scs, sizeof(char));
	gv->sceof = allocate_array(gv, gv->current_max_scs, sizeof(char));
	gv->scname = allocate_char_ptr_array (gv->current_max_scs);

	gv->current_maxccls = INITIAL_MAX_CCLS;
	gv->cclmap = allocate_integer_array (gv->current_maxccls);
	gv->ccllen = allocate_integer_array (gv->current_maxccls);
	gv->cclng = allocate_integer_array (gv->current_maxccls);
	gv->ccl_has_nl = allocate_array(gv, gv->current_maxccls, sizeof(char));

	gv->current_max_ccl_tbl_size = INITIAL_MAX_CCL_TBL_SIZE;
	gv->ccltbl = allocate_Character_array (gv->current_max_ccl_tbl_size);

	gv->current_max_dfa_size = INITIAL_MAX_DFA_SIZE;

	gv->current_max_xpairs = INITIAL_MAX_XPAIRS;
	gv->nxt = allocate_integer_array (gv->current_max_xpairs);
	gv->chk = allocate_integer_array (gv->current_max_xpairs);

	gv->current_max_template_xpairs = INITIAL_MAX_TEMPLATE_XPAIRS;
	gv->tnxt = allocate_integer_array (gv->current_max_template_xpairs);

	gv->current_max_dfas = INITIAL_MAX_DFAS;
	gv->base = allocate_integer_array (gv->current_max_dfas);
	gv->def = allocate_integer_array (gv->current_max_dfas);
	gv->dfasiz = allocate_integer_array (gv->current_max_dfas);
	gv->accsiz = allocate_integer_array (gv->current_max_dfas);
	gv->dhash = allocate_integer_array (gv->current_max_dfas);
	gv->dss = allocate_int_ptr_array (gv->current_max_dfas);
	gv->dfaacc = allocate_array(gv, gv->current_max_dfas,
		sizeof(union dfaacc_union));

	gv->nultrans = NULL;
}


void usage (FlexState* gv)
{
	FILE   *f = stdout;

	if (!gv->env.did_outfilename) {
		snprintf (gv->outfile_path, sizeof(gv->outfile_path), gv->outfile_template,
			  gv->ctrl.prefix, suffix(gv));
		gv->env.outfilename = gv->outfile_path;
	}

	fprintf (f, _("Usage: %s [OPTIONS] [FILE]...\n"), gv->program_name);
	fprintf (f,
		 _
		 ("Generates programs that perform pattern-matching on text.\n"
		  "\n" "Table Compression:\n"
		  "  -Ca, --align      trade off larger tables for better memory alignment\n"
		  "  -Ce, --ecs        construct equivalence classes\n"
		  "  -Cf               do not compress tables; use -f representation\n"
		  "  -CF               do not compress tables; use -F representation\n"
		  "  -Cm, --meta-ecs   construct meta-equivalence classes\n"
		  "  -Cr, --read       use read() instead of stdio for scanner input\n"
		  "  -f, --full        generate fast, large scanner. Same as -Cfr\n"
		  "  -F, --fast        use alternate table representation. Same as -CFr\n"
		  "  -Cem              default compression (same as --ecs --meta-ecs)\n"
		  "\n" "Debugging:\n"
		  "  -d, --debug             enable debug mode in scanner\n"
		  "  -b, --backup            write backing-up information to %s\n"
		  "  -p, --perf-report       write performance report to stderr\n"
		  "  -s, --nodefault         suppress default rule to ECHO unmatched text\n"
		  "  -T, --env.trace             %s should run in env.trace mode\n"
		  "  -w, --nowarn            do not generate warnings\n"
		  "  -v, --verbose           write summary of scanner statistics to stdout\n"
		  "      --hex               use hexadecimal numbers instead of octal in debug outputs\n"
		  "\n" "Files:\n"
		  "  -o, --outfile=FILE      specify output filename\n"
		  "  -S, --skel=FILE         specify skeleton file\n"
		  "  -t, --stdout            write scanner on stdout instead of %s\n"
		  "      --yyclass=NAME      name of C++ class\n"
		  "      --header-file=FILE   create a C header file in addition to the scanner\n"
		  "      --tables-file[=FILE] write tables to FILE\n"
		  "      --backup-file=FILE  write backing-up information to FILE\n" "\n"
		  "Scanner behavior:\n"
		  "  -7, --7bit              generate 7-bit scanner\n"
		  "  -8, --8bit              generate 8-bit scanner\n"
		  "  -B, --batch             generate batch scanner (opposite of -I)\n"
		  "  -i, --case-insensitive  ignore case in patterns\n"
		  "  -l, --lex-compat        maximal compatibility with original lex\n"
		  "  -X, --posix-compat      maximal compatibility with POSIX lex\n"
		  "  -I, --interactive       generate interactive scanner (opposite of -B)\n"
		  "      --yylineno          track line count in yylineno\n"
		  "\n" "Generated code:\n"
		  "  -+,  --c++               generate C++ scanner class\n"
		  "  -Dmacro[=defn]           #define macro defn  (default defn is '1')\n"
		  "  -e,  --emit=LANG         Specify target language\n"
		  "  -L,  --noline            suppress #line directives in scanner\n"
		  "  -P,  --prefix=STRING     use STRING as prefix instead of \"yy\"\n"
		  "  -R,  --reentrant         generate a reentrant scanner\n"
		  "       --bison-bridge      scanner for Bison pure parser.\n"
		  "       --bison-locations   include yylloc support.\n"
		  "       --stdinit           initialize yyin/yyout to stdin/stdout\n"
		  "       --nounistd          do not include <unistd.h>\n"
		  "       --noFUNCTION        do not generate a particular FUNCTION\n"
		  "\n" "Miscellaneous:\n"
		  "  -c                      do-nothing POSIX option\n"
		  "  -n                      do-nothing POSIX option\n"
		  "  -?\n"
		  "  -h, --help              produce this help message\n"
		  "  -V, --version           report %s version\n"),
		 gv->backing_name, "flex", gv->outfile_path, "flex");

}
