/* gen - actual generation (writing) of flex scanners */

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


/* declare functions that have forward references */

void	genecs(void);


int indent_level = 0;	/* each level is 8 spaces */

#define set_indent(indent_val) indent_level = indent_val

/* Almost everything is done in terms of arrays starting at 1, so provide
 * a null entry for the zero element of all C arrays.  (The exception
 * to this is that the fast table representation generally uses the
 * 0 elements of its arrays, too.)
 */

/* Indent to the current level. */

void do_indent (void)
{
	int i = indent_level * 8;

	while (i >= 8) {
		outc ('\t');
		i -= 8;
	}

	while (i > 0) {
		outc (' ');
		--i;
	}
}


void open_block(void)
{
    ++indent_level;
    indent_puts(backend->open_block);
}   

void close_block(void)
{
    --indent_level;
    indent_puts(backend->close_block);
}   

/** Make the table for possible eol matches.
 *  @return the newly allocated rule_can_match_eol table
 */
static struct yytbl_data *mkeoltbl (void)
{
	int     i;
	flex_int8_t *tdata = 0;
	struct yytbl_data *tbl;

	tbl = calloc(1, sizeof (struct yytbl_data));
	yytbl_data_init (tbl, YYTD_ID_RULE_CAN_MATCH_EOL);
	tbl->td_flags = YYTD_DATA8;
	tbl->td_lolen = (flex_uint32_t) (num_rules + 1);
	tbl->td_data = tdata =
		calloc(tbl->td_lolen, sizeof (flex_int8_t));

	for (i = 1; i <= num_rules; i++)
		tdata[i] = rule_has_nl[i] ? 1 : 0;

	backend->mkeoltbl();
	return tbl;
}

/* Generate the table for possible eol matches. */
static void geneoltbl (void)
{
	int     i;

	outn ("m4_ifdef( [[M4_YY_USE_LINENO]],[[");
	backend->geneoltbl(num_rules + 1);

	if (gentables) {
		for (i = 1; i <= num_rules; i++) {
			out_dec ("%d, ", rule_has_nl[i] ? 1 : 0);
			/* format nicely, 20 numbers per line. */
			if ((i % 20) == 19)
				out ("\n    ");
		}
		out (backend->table_closer);
	}
	outn ("]])");
}


/** mkctbl - make full speed compressed transition table
 * This is an array of structs; each struct a pair of integers.
 * You should call mkssltbl() immediately after this.
 * Then, I think, mkecstbl(). Arrrg.
 * @return the newly allocated trans table
 */

static struct yytbl_data *mkctbl (void)
{
	int i;
	struct yytbl_data *tbl = 0;
	flex_int32_t *tdata = 0, curr = 0;
	int     end_of_buffer_action = num_rules + 1;

	backend->mkctbl(tblend + numecs + 1);

	tbl = calloc(1, sizeof (struct yytbl_data));
	yytbl_data_init (tbl, YYTD_ID_TRANSITION);
	tbl->td_flags = YYTD_DATA32 | YYTD_STRUCT;
	tbl->td_hilen = 0;
	tbl->td_lolen = (flex_uint32_t) (tblend + numecs + 1);	/* number of structs */

	tbl->td_data = tdata =
		calloc(tbl->td_lolen * 2, sizeof (flex_int32_t));

	/* We want the transition to be represented as the offset to the
	 * next state, not the actual state number, which is what it currently
	 * is.  The offset is base[nxt[i]] - (base of current state)].  That's
	 * just the difference between the starting points of the two involved
	 * states (to - from).
	 *
	 * First, though, we need to find some way to put in our end-of-buffer
	 * flags and states.  We do this by making a state with absolutely no
	 * transitions.  We put it at the end of the table.
	 */

	/* We need to have room in nxt/chk for two more slots: One for the
	 * action and one for the end-of-buffer transition.  We now *assume*
	 * that we're guaranteed the only character we'll try to index this
	 * nxt/chk pair with is EOB, i.e., 0, so we don't have to make sure
	 * there's room for jam entries for other characters.
	 */

	while (tblend + 2 >= current_max_xpairs)
		expand_nxt_chk ();

	while (lastdfa + 1 >= current_max_dfas)
		increase_max_dfas ();

	base[lastdfa + 1] = tblend + 2;
	nxt[tblend + 1] = end_of_buffer_action;
	chk[tblend + 1] = numecs + 1;
	chk[tblend + 2] = 1;	/* anything but EOB */

	/* So that "make test" won't show arb. differences. */
	nxt[tblend + 2] = 0;

	/* Make sure every state has an end-of-buffer transition and an
	 * action #.
	 */
	for (i = 0; i <= lastdfa; ++i) {
		int     anum = dfaacc[i].dfaacc_state;
		int     offset = base[i];

		chk[offset] = EOB_POSITION;
		chk[offset - 1] = ACTION_POSITION;
		nxt[offset - 1] = anum;	/* action number */
	}

	for (i = 0; i <= tblend; ++i) {
		if (chk[i] == EOB_POSITION) {
			tdata[curr++] = 0;
			tdata[curr++] = base[lastdfa + 1] - i;
		}

		else if (chk[i] == ACTION_POSITION) {
			tdata[curr++] = 0;
			tdata[curr++] = nxt[i];
		}

		else if (chk[i] > numecs || chk[i] == 0) {
			tdata[curr++] = 0;
			tdata[curr++] = 0;
		}
		else {		/* verify, transition */

			tdata[curr++] = chk[i];
			tdata[curr++] = base[nxt[i]] - (i - chk[i]);
		}
	}


	/* Here's the final, end-of-buffer state. */
	tdata[curr++] = chk[tblend + 1];
	tdata[curr++] = nxt[tblend + 1];

	tdata[curr++] = chk[tblend + 2];
	tdata[curr++] = nxt[tblend + 2];

	return tbl;
}


/** Make start_state_list table.
 *  @return the newly allocated start_state_list table
 */
static struct yytbl_data *mkssltbl (void)
{
	struct yytbl_data *tbl = 0;
	flex_int32_t *tdata = 0;
	flex_int32_t i;

	tbl = calloc(1, sizeof (struct yytbl_data));
	yytbl_data_init (tbl, YYTD_ID_START_STATE_LIST);
	tbl->td_flags = YYTD_DATA32 | YYTD_PTRANS;
	tbl->td_hilen = 0;
	tbl->td_lolen = (flex_uint32_t) (lastsc * 2 + 1);

	tbl->td_data = tdata =
		calloc(tbl->td_lolen, sizeof (flex_int32_t));

	for (i = 0; i <= lastsc * 2; ++i)
		tdata[i] = base[i];

	backend->mkssltbl();

	return tbl;
}



/* genctbl - generates full speed compressed transition table */

void genctbl (void)
{
	int i;
	int     end_of_buffer_action = num_rules + 1;

	/* Table of verify for transition and offset to next state. */
	backend->gen_yy_trans(tblend + numecs + 1);

	/* We want the transition to be represented as the offset to the
	 * next state, not the actual state number, which is what it currently
	 * is.  The offset is base[nxt[i]] - (base of current state)].  That's
	 * just the difference between the starting points of the two involved
	 * states (to - from).
	 *
	 * First, though, we need to find some way to put in our end-of-buffer
	 * flags and states.  We do this by making a state with absolutely no
	 * transitions.  We put it at the end of the table.
	 */

	/* We need to have room in nxt/chk for two more slots: One for the
	 * action and one for the end-of-buffer transition.  We now *assume*
	 * that we're guaranteed the only character we'll try to index this
	 * nxt/chk pair with is EOB, i.e., 0, so we don't have to make sure
	 * there's room for jam entries for other characters.
	 */

	while (tblend + 2 >= current_max_xpairs)
		expand_nxt_chk ();

	while (lastdfa + 1 >= current_max_dfas)
		increase_max_dfas ();

	base[lastdfa + 1] = tblend + 2;
	nxt[tblend + 1] = end_of_buffer_action;
	chk[tblend + 1] = numecs + 1;
	chk[tblend + 2] = 1;	/* anything but EOB */

	/* So that "make test" won't show arb. differences. */
	nxt[tblend + 2] = 0;

	/* Make sure every state has an end-of-buffer transition and an
	 * action #.
	 */
	for (i = 0; i <= lastdfa; ++i) {
		int     anum = dfaacc[i].dfaacc_state;
		int     offset = base[i];

		chk[offset] = EOB_POSITION;
		chk[offset - 1] = ACTION_POSITION;
		nxt[offset - 1] = anum;	/* action number */
	}

	for (i = 0; i <= tblend; ++i) {
		if (chk[i] == EOB_POSITION)
			transition_struct_out (0, base[lastdfa + 1] - i);

		else if (chk[i] == ACTION_POSITION)
			transition_struct_out (0, nxt[i]);

		else if (chk[i] > numecs || chk[i] == 0)
			transition_struct_out (0, 0);	/* unused slot */

		else		/* verify, transition */
			transition_struct_out (chk[i],
					       base[nxt[i]] - (i -
							       chk[i]));
	}


	/* Here's the final, end-of-buffer state. */
	transition_struct_out (chk[tblend + 1], nxt[tblend + 1]);
	transition_struct_out (chk[tblend + 2], nxt[tblend + 2]);

	if (gentables)
		outn (backend->table_closer);

	backend->start_state_list(lastsc * 2 + 1);

	if (gentables) {
		outn (backend->table_opener);

		for (i = 0; i <= lastsc * 2; ++i)
			out_dec (backend->state_entry_fmt, base[i]);

		dataend ();
	}

	if (useecs)
		genecs ();
}


/* mkecstbl - Make equivalence-class tables.  */

static struct yytbl_data *mkecstbl (void)
{
	int i;
	struct yytbl_data *tbl = 0;
	flex_int32_t *tdata = 0;

	tbl = calloc(1, sizeof (struct yytbl_data));
	yytbl_data_init (tbl, YYTD_ID_EC);
	tbl->td_flags |= YYTD_DATA32;
	tbl->td_hilen = 0;
	tbl->td_lolen = (flex_uint32_t) csize;

	tbl->td_data = tdata =
		calloc(tbl->td_lolen, sizeof (flex_int32_t));

	for (i = 1; i < csize; ++i) {
		ecgroup[i] = ABS (ecgroup[i]);
		tdata[i] = ecgroup[i];
	}

	backend->mkecstbl();

	return tbl;
}

/* Generate equivalence-class tables. */

void genecs (void)
{
	int ch, row;
	int     numrows;

	out_str_dec (backend->get_yy_char_decl (), "yy_ec", csize);

	for (ch = 1; ch < csize; ++ch) {
		ecgroup[ch] = ABS (ecgroup[ch]);
		mkdata (ecgroup[ch]);
	}

	dataend ();

	if (trace) {
		fputs (_("\n\nEquivalence Classes:\n\n"), stderr);

		/* Print in 8 columns */
		numrows = csize / 8;

		for (row = 0; row < numrows; ++row) {
			for (ch = row; ch < csize; ch += numrows) {
				fprintf (stderr, "%4s = %-2d",
					 readable_form (ch), ecgroup[ch]);

				putc (' ', stderr);
			}

			putc ('\n', stderr);
		}
	}
}


/* mkftbl - make the full table and return the struct .
 * you should call mkecstbl() after this.
 */

struct yytbl_data *mkftbl (void)
{
	int i;
	int     end_of_buffer_action = num_rules + 1;
	struct yytbl_data *tbl;
	flex_int32_t *tdata = 0;

	tbl = calloc(1, sizeof (struct yytbl_data));
	yytbl_data_init (tbl, YYTD_ID_ACCEPT);
	tbl->td_flags |= YYTD_DATA32;
	tbl->td_hilen = 0;	/* it's a one-dimensional array */
	tbl->td_lolen = (flex_uint32_t) (lastdfa + 1);

	tbl->td_data = tdata =
		calloc(tbl->td_lolen, sizeof (flex_int32_t));

	dfaacc[end_of_buffer_state].dfaacc_state = end_of_buffer_action;

	for (i = 1; i <= lastdfa; ++i) {
		int anum = dfaacc[i].dfaacc_state;

		tdata[i] = anum;

		if (trace && anum)
			fprintf (stderr, _("state # %d accepts: [%d]\n"),
				 i, anum);
	}

	backend->mkftbl();
	return tbl;
}


/* genftbl - generate full transition table */

void genftbl (void)
{
	int i;
	int     end_of_buffer_action = num_rules + 1;

	out_str_dec (long_align ? backend->get_int32_decl () : backend->get_int16_decl (),
		     "yy_accept", lastdfa + 1);

	dfaacc[end_of_buffer_state].dfaacc_state = end_of_buffer_action;

	for (i = 1; i <= lastdfa; ++i) {
		int anum = dfaacc[i].dfaacc_state;

		mkdata (anum);

		if (trace && anum)
			fprintf (stderr, _("state # %d accepts: [%d]\n"),
				 i, anum);
	}

	dataend ();

	if (useecs)
		genecs ();

	/* Don't have to dump the actual full table entries - they were
	 * created on-the-fly.
	 */
}

/* Generate the code to find the next state.  Only used when we're worried about NULs */

void gen_next_state ()
{
	/* NOTE - changes in here should be reflected in gen_next_match() */
	char    *char_map;

	if (!nultrans) {
		char_map = "(*yy_cp ? M4_EC(YY_SC_TO_UI(*yy_cp)) : YY_NUL_EC)";
	} else {
	    	char_map = "M4_EC(YY_SC_TO_UI(*yy_cp))";
	}
	
	if (nultrans) {
		if (!fulltbl && !fullspd)
			/* Compressed tables back up *before* they match. */
			outn("M4_GEN_BACKING_UP");

		indent_puts ("if ( *yy_cp )");
		open_block();
	}

	if (fulltbl) {
		if (gentables)
			backend->assign("yy_current_state", "yy_nxt[yy_current_state][%s]",
				 char_map);
		else
			backend->assign("yy_current_state", "yy_nxt[yy_current_state*YY_NXT_LOLEN + %s];",
				 char_map);
	}
	else if (fullspd)
		backend->statement("yy_current_state += yy_current_state[%s].yy_nxt", char_map);

	else
		out_str ("M4_GEN_NEXT_COMPRESSED_STATE(%s)", char_map);

	if (nultrans) {

		close_block();
		--indent_level;
		indent_puts ("else");
		open_block();
		backend->assign("yy_current_state", "yy_NUL_trans[yy_current_state]");
		close_block();
	}

	if (fullspd || fulltbl)
		outn("M4_GEN_BACKING_UP");

	if (reject)
		indent_puts ("*YY_G(yy_state_ptr)++ = yy_current_state;");	// POINTER
}


/* Generate the code to make a NUL transition. */

void gen_NUL_trans (void)
{
	/* NOTE - changes in here should be reflected in gen_next_match() */
	/* Only generate a definition for "yy_cp" if we'll generate code
	 * that uses it.  Otherwise lint and the like complain.
	 */
	if (!reject && (!nultrans || fullspd || fulltbl))
		/* We're going to need yy_cp lying around for the call
		 * below to gen_backing_up().
		 */
		indent_puts ("char *yy_cp = YY_G(yy_c_buf_p);");

	outc ('\n');

	if (nultrans) {
		indent_puts
			("yy_current_state = yy_NUL_trans[yy_current_state];");
		indent_puts ("yy_is_jam = (yy_current_state == 0);");
	}

	else if (fulltbl) {
		do_indent ();
		if (gentables)
			outn ("yy_current_state = yy_nxt[yy_current_state][YY_NUL_EC];");
		else
			outn ("yy_current_state = yy_nxt[yy_current_state*YY_NXT_LOLEN + YY_NUL_EC];");
		indent_puts ("yy_is_jam = (yy_current_state <= 0);");
	}

	else if (fullspd) {
		indent_puts ("int yy_c = YY_NUL_EC;");
		indent_puts
			("const struct yy_trans_info *yy_trans_info;\n");
		indent_puts
			("yy_trans_info = &yy_current_state[(unsigned int) yy_c];");
		indent_puts ("yy_current_state += yy_trans_info->yy_nxt;");

		indent_puts
			("yy_is_jam = (yy_trans_info->yy_verify != yy_c);");
	}

	else {
		out ("M4_GEN_NEXT_COMPRESSED_STATE(YY_NUL_EC)");

		indent_puts ("yy_is_jam = (yy_current_state == YY_JAMSTATE);");

		if (reject) {
			/* Only stack this state if it's a transition we
			 * actually make.  If we stack it on a jam, then
			 * the state stack and yy_c_buf_p get out of sync.
			 */
			indent_puts ("if ( ! yy_is_jam )");
			++indent_level;
			indent_puts
				("*YY_G(yy_state_ptr)++ = yy_current_state;");
			--indent_level;
		}
	}

	/* If we've entered an accepting state, back up; note that
	 * compressed tables have *already* done such backing up, so
	 * we needn't bother with it again.
	 */
	if (!reject && (fullspd || fulltbl)) {
		outc ('\n');
		indent_puts ("if ( ! yy_is_jam )");
		open_block();
		outn("M4_GEN_BACKING_UP");
		close_block();
	}
}

/* gentabs - generate data statements for the transition tables */

void gentabs (void)
{
	int     i, j, k, *accset, nacc, *acc_array, total_states;
	int     end_of_buffer_action = num_rules + 1;
	struct yytbl_data *yyacc_tbl = 0, *yymeta_tbl = 0, *yybase_tbl = 0,
	    *yydef_tbl = 0, *yynxt_tbl = 0, *yychk_tbl = 0, *yyacclist_tbl=0;
	flex_int32_t *yyacc_data = 0, *yybase_data = 0, *yydef_data = 0,
	    *yynxt_data = 0, *yychk_data = 0, *yyacclist_data=0;
	flex_int32_t yybase_curr = 0, yyacclist_curr=0,yyacc_curr=0;

	acc_array = allocate_integer_array (current_max_dfas);
	nummt = 0;

	/* The compressed table format jams by entering the "jam state",
	 * losing information about the previous state in the process.
	 * In order to recover the previous state, we effectively need
	 * to keep backing-up information.
	 */

	if (reject) {
		/* Write out accepting list and pointer list.

		 * First we generate the "yy_acclist" array.  In the process,
		 * we compute the indices that will go into the "yy_accept"
		 * array, and save the indices in the dfaacc array.
		 */
		int     EOB_accepting_list[2];

		/* Set up accepting structures for the End Of Buffer state. */
		EOB_accepting_list[0] = 0;
		EOB_accepting_list[1] = end_of_buffer_action;
		accsiz[end_of_buffer_state] = 1;
		dfaacc[end_of_buffer_state].dfaacc_set =
		    EOB_accepting_list;

		out_str_dec (long_align ? backend->get_int32_decl () :
			     backend->get_int16_decl (), "yy_acclist", MAX (numas,
									    1) + 1);

		backend->gentabs_acclist();
		
		yyacclist_tbl = calloc(1,sizeof(struct yytbl_data));
		yytbl_data_init (yyacclist_tbl, YYTD_ID_ACCLIST);
		yyacclist_tbl->td_lolen  = (flex_uint32_t) (MAX(numas,1) + 1);
		yyacclist_tbl->td_data = yyacclist_data =
		    calloc(yyacclist_tbl->td_lolen, sizeof (flex_int32_t));
		yyacclist_curr = 1;

		j = 1;		/* index into "yy_acclist" array */

		for (i = 1; i <= lastdfa; ++i) {
			acc_array[i] = j;

			if (accsiz[i] != 0) {
				accset = dfaacc[i].dfaacc_set;
				nacc = accsiz[i];

				if (trace)
					fprintf (stderr,
						 _("state # %d accepts: "),
						 i);

				for (k = 1; k <= nacc; ++k) {
					int     accnum = accset[k];

					++j;

					if (variable_trailing_context_rules
					    && !(accnum &
						 YY_TRAILING_HEAD_MASK)
					    && accnum > 0
					    && accnum <= num_rules
					    && rule_type[accnum] ==
					    RULE_VARIABLE) {
						/* Special hack to flag
						 * accepting number as part
						 * of trailing context rule.
						 */
						accnum |= YY_TRAILING_MASK;
					}

					mkdata (accnum);
					yyacclist_data[yyacclist_curr++] = accnum;

					if (trace) {
						fprintf (stderr, "[%d]",
							 accset[k]);

						if (k < nacc)
							fputs (", ",
							       stderr);
						else
							putc ('\n',
							      stderr);
					}
				}
			}
		}

		/* add accepting number for the "jam" state */
		acc_array[i] = j;

		dataend ();
		if (tablesext) {
			yytbl_data_compress (yyacclist_tbl);
			if (yytbl_data_fwrite (&tableswr, yyacclist_tbl) < 0)
				flexerror (_("Could not write yyacclist_tbl"));
			yytbl_data_destroy (yyacclist_tbl);
			yyacclist_tbl = NULL;
		}
	}

	else {
		dfaacc[end_of_buffer_state].dfaacc_state =
		    end_of_buffer_action;

		for (i = 1; i <= lastdfa; ++i)
			acc_array[i] = dfaacc[i].dfaacc_state;

		/* add accepting number for jam state */
		acc_array[i] = 0;
	}

	/* Begin generating yy_accept */

	/* Spit out "yy_accept" array.  If we're doing "reject", it'll be
	 * pointers into the "yy_acclist" array.  Otherwise it's actual
	 * accepting numbers.  In either case, we just dump the numbers.
	 */

	/* "lastdfa + 2" is the size of "yy_accept"; includes room for C arrays
	 * beginning at 0 and for "jam" state.
	 */
	k = lastdfa + 2;

	if (reject)
		/* We put a "cap" on the table associating lists of accepting
		 * numbers with state numbers.  This is needed because we tell
		 * where the end of an accepting list is by looking at where
		 * the list for the next state starts.
		 */
		++k;

	out_str_dec (long_align ? backend->get_int32_decl () : backend->get_int16_decl (),
		     "yy_accept", k);

	backend->gentabs_accept();
	
	yyacc_tbl = calloc(1, sizeof (struct yytbl_data));
	yytbl_data_init (yyacc_tbl, YYTD_ID_ACCEPT);
	yyacc_tbl->td_lolen = (flex_uint32_t) k;
	yyacc_tbl->td_data = yyacc_data =
	    calloc(yyacc_tbl->td_lolen, sizeof (flex_int32_t));
	yyacc_curr=1;

	for (i = 1; i <= lastdfa; ++i) {
		mkdata (acc_array[i]);
		yyacc_data[yyacc_curr++] = acc_array[i];

		if (!reject && trace && acc_array[i])
			fprintf (stderr, _("state # %d accepts: [%d]\n"),
				 i, acc_array[i]);
	}

	/* Add entry for "jam" state. */
	mkdata (acc_array[i]);
	yyacc_data[yyacc_curr++] = acc_array[i];

	if (reject) {
		/* Add "cap" for the list. */
		mkdata (acc_array[i]);
		yyacc_data[yyacc_curr++] = acc_array[i];
	}

	dataend ();
	if (tablesext) {
		yytbl_data_compress (yyacc_tbl);
		if (yytbl_data_fwrite (&tableswr, yyacc_tbl) < 0)
			flexerror (_("Could not write yyacc_tbl"));
	}
	yytbl_data_destroy (yyacc_tbl);
	yyacc_tbl = NULL;
	/* End generating yy_accept */

	if (useecs) {

		genecs ();
		if (tablesext) {
			struct yytbl_data *tbl;

			tbl = mkecstbl ();
			yytbl_data_compress (tbl);
			if (yytbl_data_fwrite (&tableswr, tbl) < 0)
				flexerror (_("Could not write ecstbl"));
			yytbl_data_destroy (tbl);
			tbl = 0;
		}
	}

	if (usemecs) {
		/* Begin generating yy_meta */
		/* Write out meta-equivalence classes (used to index
		 * templates with).
		 */
		flex_int32_t *yymecs_data = 0;
		yymeta_tbl = calloc(1, sizeof (struct yytbl_data));
		yytbl_data_init (yymeta_tbl, YYTD_ID_META);
		yymeta_tbl->td_lolen = (flex_uint32_t) (numecs + 1);
		yymeta_tbl->td_data = yymecs_data =
		    calloc(yymeta_tbl->td_lolen,
			   sizeof (flex_int32_t));

		if (trace)
			fputs (_("\n\nMeta-Equivalence Classes:\n"),
			       stderr);

		out_str_dec (backend->get_yy_char_decl (), "yy_meta", numecs + 1);
		backend->gentabs_yy_meta();
		
		for (i = 1; i <= numecs; ++i) {
			if (trace)
				fprintf (stderr, "%d = %d\n",
					 i, ABS (tecbck[i]));

			mkdata (ABS (tecbck[i]));
			yymecs_data[i] = ABS (tecbck[i]);
		}

		dataend ();
		if (tablesext) {
			yytbl_data_compress (yymeta_tbl);
			if (yytbl_data_fwrite (&tableswr, yymeta_tbl) < 0)
				flexerror (_("Could not write yymeta_tbl"));
		}
		yytbl_data_destroy (yymeta_tbl);
		yymeta_tbl = NULL;
		/* End generating yy_meta */
	}

	total_states = lastdfa + numtemps;

	/* Begin generating yy_base */
	out_str_dec ((tblend >= INT16_MAX || long_align) ?
		     backend->get_int32_decl () : backend->get_int16_decl (),
		     "yy_base", total_states + 1);

	backend->gentabs_yy_base();
	yybase_tbl = calloc (1, sizeof (struct yytbl_data));
	yytbl_data_init (yybase_tbl, YYTD_ID_BASE);
	yybase_tbl->td_lolen = (flex_uint32_t) (total_states + 1);
	yybase_tbl->td_data = yybase_data =
	    calloc(yybase_tbl->td_lolen,
		   sizeof (flex_int32_t));
	yybase_curr = 1;

	for (i = 1; i <= lastdfa; ++i) {
		int d = def[i];

		if (base[i] == JAMSTATE)
			base[i] = jambase;

		if (d == JAMSTATE)
			def[i] = jamstate;

		else if (d < 0) {
			/* Template reference. */
			++tmpuses;
			def[i] = lastdfa - d + 1;
		}

		mkdata (base[i]);
		yybase_data[yybase_curr++] = base[i];
	}

	/* Generate jam state's base index. */
	mkdata (base[i]);
	yybase_data[yybase_curr++] = base[i];

	for (++i /* skip jam state */ ; i <= total_states; ++i) {
		mkdata (base[i]);
		yybase_data[yybase_curr++] = base[i];
		def[i] = jamstate;
	}

	dataend ();
	if (tablesext) {
		yytbl_data_compress (yybase_tbl);
		if (yytbl_data_fwrite (&tableswr, yybase_tbl) < 0)
			flexerror (_("Could not write yybase_tbl"));
	}
	yytbl_data_destroy (yybase_tbl);
	yybase_tbl = NULL;
	/* End generating yy_base */


	/* Begin generating yy_def */
	out_str_dec ((total_states >= INT16_MAX || long_align) ?
		     backend->get_int32_decl () : backend->get_int16_decl (),
		     "yy_def", total_states + 1);

	backend->gentabs_yy_def(total_states);

	yydef_tbl = calloc(1, sizeof (struct yytbl_data));
	yytbl_data_init (yydef_tbl, YYTD_ID_DEF);
	yydef_tbl->td_lolen = (flex_uint32_t) (total_states + 1);
	yydef_tbl->td_data = yydef_data =
	    calloc(yydef_tbl->td_lolen, sizeof (flex_int32_t));

	for (i = 1; i <= total_states; ++i) {
		mkdata (def[i]);
		yydef_data[i] = def[i];
	}

	dataend ();
	if (tablesext) {
		yytbl_data_compress (yydef_tbl);
		if (yytbl_data_fwrite (&tableswr, yydef_tbl) < 0)
			flexerror (_("Could not write yydef_tbl"));
	}
	yytbl_data_destroy (yydef_tbl);
	yydef_tbl = NULL;
	/* End generating yy_def */


	/* Begin generating yy_nxt */
	out_str_dec ((total_states >= INT16_MAX || long_align) ?
		     backend->get_int32_decl () : backend->get_int16_decl (), "yy_nxt",
		     tblend + 1);

	backend->gentabs_yy_nxt(total_states);

	yynxt_tbl = calloc (1, sizeof (struct yytbl_data));
	yytbl_data_init (yynxt_tbl, YYTD_ID_NXT);
	yynxt_tbl->td_lolen = (flex_uint32_t) (tblend + 1);
	yynxt_tbl->td_data = yynxt_data =
	    calloc (yynxt_tbl->td_lolen, sizeof (flex_int32_t));

	for (i = 1; i <= tblend; ++i) {
		/* Note, the order of the following test is important.
		 * If chk[i] is 0, then nxt[i] is undefined.
		 */
		if (chk[i] == 0 || nxt[i] == 0)
			nxt[i] = jamstate;	/* new state is the JAM state */

		mkdata (nxt[i]);
		yynxt_data[i] = nxt[i];
	}

	dataend ();
	if (tablesext) {
		yytbl_data_compress (yynxt_tbl);
		if (yytbl_data_fwrite (&tableswr, yynxt_tbl) < 0)
			flexerror (_("Could not write yynxt_tbl"));
	}
	yytbl_data_destroy (yynxt_tbl);
	yynxt_tbl = NULL;
	/* End generating yy_nxt */

	/* Begin generating yy_chk */
	out_str_dec ((total_states >= INT16_MAX || long_align) ?
		     backend->get_int32_decl () : backend->get_int16_decl (), "yy_chk",
		     tblend + 1);

	backend->gentabs_yy_chk(total_states);
	
	yychk_tbl = calloc (1, sizeof (struct yytbl_data));
	yytbl_data_init (yychk_tbl, YYTD_ID_CHK);
	yychk_tbl->td_lolen = (flex_uint32_t) (tblend + 1);
	yychk_tbl->td_data = yychk_data =
	    calloc(yychk_tbl->td_lolen, sizeof (flex_int32_t));

	for (i = 1; i <= tblend; ++i) {
		if (chk[i] == 0)
			++nummt;

		mkdata (chk[i]);
		yychk_data[i] = chk[i];
	}

	dataend ();
	if (tablesext) {
		yytbl_data_compress (yychk_tbl);
		if (yytbl_data_fwrite (&tableswr, yychk_tbl) < 0)
			flexerror (_("Could not write yychk_tbl"));
	}
	yytbl_data_destroy (yychk_tbl);
	yychk_tbl = NULL;
	/* End generating yy_chk */

	free(acc_array);
}


/* Write out a formatted string (with a secondary string argument) at the
 * current indentation level, adding a final newline.
 */

void indent_put2s (const char *fmt, const char *arg)
{
	do_indent ();
	out_str (fmt, arg);
	outn ("");
}


/* Write out a string at the current indentation level, adding a final
 * newline.
 */

void indent_puts (const char *str)
{
	do_indent ();
	outn (str);
}


/* make_tables - generate transition tables and finishes generating output file
 */

void make_tables (void)
{
	int i;
	int did_eof_rule = false;
	struct yytbl_data *yynultrans_tbl = NULL;


	skelout ();		/* %% [2.0] - break point in skel */

	/* First, take care of YY_DO_BEFORE_ACTION depending on yymore
	 * being used.
	 */
	set_indent (1);

	if (yymore_used && !yytext_is_array) {
		indent_puts ("YY_G(yytext_ptr) -= YY_G(yy_more_len); \\");
		indent_puts
			("yyleng = (int) (yy_cp - YY_G(yytext_ptr)); \\");
	}

	else
		indent_puts ("yyleng = (int) (yy_cp - yy_bp); \\");

	/* Now also deal with copying yytext_ptr to yytext if needed. */
	skelout ();		/* %% [3.0] - break point in skel */
	if (yytext_is_array) {
		if (yymore_used)
			indent_puts
				("if ( yyleng + YY_G(yy_more_offset) >= YYLMAX ) \\");
		else
			indent_puts ("if ( yyleng >= YYLMAX ) \\");

		++indent_level;
		indent_puts
			("YY_FATAL_ERROR( \"token too large, exceeds YYLMAX\" ); \\");
		--indent_level;

		if (yymore_used) {
			indent_puts
				("yy_flex_strncpy( &yytext[YY_G(yy_more_offset)], YY_G(yytext_ptr), yyleng + 1 M4_YY_CALL_LAST_ARG); \\");
			indent_puts ("yyleng += YY_G(yy_more_offset); \\");
			indent_puts
				("YY_G(yy_prev_more_offset) = YY_G(yy_more_offset); \\");
			indent_puts ("YY_G(yy_more_offset) = 0; \\");
		}
		else {
			indent_puts
				("yy_flex_strncpy( yytext, YY_G(yytext_ptr), yyleng + 1 M4_YY_CALL_LAST_ARG); \\");
		}
	}

	set_indent (0);

	skelout ();		/* %% [4.0] - break point in skel */


	/* This is where we REALLY begin generating the tables. */

	fprintf (stdout, backend->int_define_fmt, "YY_NUM_RULES", num_rules);
	fprintf (stdout, backend->int_define_fmt, "YY_END_OF_BUFFER", num_rules + 1);

	fprintf (stdout, backend->int_define_fmt, "YY_JAMBASE", jambase);
	fprintf (stdout, backend->int_define_fmt, "YY_JAMSTATE", jamstate);

	fprintf (stdout, backend->int_define_fmt, "YY_NUL_EC", NUL_ec);

	if (fullspd) {
		/* Need to define the transet type as a size large
		 * enough to hold the biggest offset.
		 */
		int     total_table_size = tblend + numecs + 1;
		char   *trans_offset_type =
			(total_table_size >= INT16_MAX || long_align) ?
			"flex_int32_t" : "flex_int16_t";

		set_indent (0);
		indent_puts ("struct yy_trans_info");
		open_block();

		/* We require that yy_verify and yy_nxt must be of the same size int. */
		indent_put2s ("%s yy_verify;", trans_offset_type);

		/* In cases where its sister yy_verify *is* a "yes, there is
		 * a transition", yy_nxt is the offset (in records) to the
		 * next state.  In most cases where there is no transition,
		 * the value of yy_nxt is irrelevant.  If yy_nxt is the -1th
		 * record of a state, though, then yy_nxt is the action number
		 * for that state.
		 */

		indent_put2s ("%s yy_nxt;", trans_offset_type);
		indent_puts ("};");
		--indent_level;
	}
	else {
		/* We generate a bogus 'struct yy_trans_info' data type
		 * so we can guarantee that it is always declared in the skel.
		 * This is so we can compile "sizeof(struct yy_trans_info)"
		 * in any scanner.
		 */
		indent_puts
			("/* This struct is not used in this scanner,");
		indent_puts ("   but its presence is necessary. */");
		indent_puts ("struct yy_trans_info");
		open_block();
		indent_puts ("flex_int32_t yy_verify;");
		indent_puts ("flex_int32_t yy_nxt;");
		indent_puts ("};");
		--indent_level;
	}

	if (fullspd) {
		genctbl ();
		if (tablesext) {
			struct yytbl_data *tbl;

			tbl = mkctbl ();
			yytbl_data_compress (tbl);
			if (yytbl_data_fwrite (&tableswr, tbl) < 0)
				flexerror (_("Could not write ftbl"));
			yytbl_data_destroy (tbl);

			tbl = mkssltbl ();
			yytbl_data_compress (tbl);
			if (yytbl_data_fwrite (&tableswr, tbl) < 0)
				flexerror (_("Could not write ssltbl"));
			yytbl_data_destroy (tbl);
			tbl = 0;

			if (useecs) {
				tbl = mkecstbl ();
				yytbl_data_compress (tbl);
				if (yytbl_data_fwrite (&tableswr, tbl) < 0)
					flexerror (_
						   ("Could not write ecstbl"));
				yytbl_data_destroy (tbl);
				tbl = 0;
			}
		}
	}
	else if (fulltbl) {
		genftbl ();
		if (tablesext) {
			struct yytbl_data *tbl;

			tbl = mkftbl ();
			yytbl_data_compress (tbl);
			if (yytbl_data_fwrite (&tableswr, tbl) < 0)
				flexerror (_("Could not write ftbl"));
			yytbl_data_destroy (tbl);
			tbl = 0;

			if (useecs) {
				tbl = mkecstbl ();
				yytbl_data_compress (tbl);
				if (yytbl_data_fwrite (&tableswr, tbl) < 0)
					flexerror (_
						   ("Could not write ecstbl"));
				yytbl_data_destroy (tbl);
				tbl = 0;
			}
		}
	}
	else
		gentabs ();

	if (do_yylineno) {

		geneoltbl ();

		if (tablesext) {
			struct yytbl_data *tbl;

			tbl = mkeoltbl ();
			yytbl_data_compress (tbl);
			if (yytbl_data_fwrite (&tableswr, tbl) < 0)
				flexerror (_("Could not write eoltbl"));
			yytbl_data_destroy (tbl);
			tbl = 0;
		}
	}

	/* Definitions for backing up.  We don't need them if REJECT
	 * is being used because then we use an alternative backing-up
	 * technique instead.
	 */
	if (!reject) {
		if (!C_plus_plus && !reentrant) {
			indent_puts
				("static yy_state_type yy_last_accepting_state;\n");
			indent_puts
				("static char *yy_last_accepting_cpos;\n");
		}
	}

	if (nultrans) {
		flex_int32_t *yynultrans_data = 0;

		/* Begin generating yy_NUL_trans */
		out_str_dec (backend->get_state_decl (), "yy_NUL_trans",
			     lastdfa + 1);
		backend->nultrans(fullspd);
		yynultrans_tbl = calloc(1, sizeof (struct yytbl_data));
		yytbl_data_init (yynultrans_tbl, YYTD_ID_NUL_TRANS);
		if (fullspd)
			yynultrans_tbl->td_flags |= YYTD_PTRANS;
		yynultrans_tbl->td_lolen = (flex_uint32_t) (lastdfa + 1);
		yynultrans_tbl->td_data = yynultrans_data =
			calloc(yynultrans_tbl->td_lolen,
					    sizeof (flex_int32_t));

		for (i = 1; i <= lastdfa; ++i) {
			if (fullspd) {
				out_dec ("    &yy_transition[%d],\n",
					 base[i]);
				yynultrans_data[i] = base[i];
			}
			else {
				mkdata (nultrans[i]);
				yynultrans_data[i] = nultrans[i];
			}
		}

		dataend ();
		if (tablesext) {
			yytbl_data_compress (yynultrans_tbl);
			if (yytbl_data_fwrite (&tableswr, yynultrans_tbl) <
			    0)
				flexerror (_
					   ("Could not write yynultrans_tbl"));
		}

		if (yynultrans_tbl != NULL) {
			yytbl_data_destroy (yynultrans_tbl);
			yynultrans_tbl = NULL;
        }

		/* End generating yy_NUL_trans */
	}

	if (!C_plus_plus && !reentrant) {
		indent_puts ("extern int yy_flex_debug;");
		indent_put2s ("int yy_flex_debug = %s;\n",
			      ddebug ? "1" : "0");
	}

	if (ddebug) {		/* Spit out table mapping rules to line numbers. */
		out_str_dec (long_align ? backend->get_int32_decl () :
			     backend->get_int16_decl (), "yy_rule_linenum",
			     num_rules);
		for (i = 1; i < num_rules; ++i)
			mkdata (rule_linenum[i]);
		dataend ();
	}

	if (reject) {
		outn ("m4_ifdef( [[M4_YY_USES_REJECT]],\n[[");
		/* Declare state buffer variables. */
		if (!C_plus_plus && !reentrant) {
			outn ("static yy_state_type *yy_state_buf=0, *yy_state_ptr=0;");
			outn ("static char *yy_full_match;");
			outn ("static int yy_lp;");
		}

		if (variable_trailing_context_rules) {
			if (!C_plus_plus && !reentrant) {
				outn ("static int yy_looking_for_trail_begin = 0;");
				outn ("static int yy_full_lp;");
				outn ("static int *yy_full_state;");
			}

			out_hex ("#define YY_TRAILING_MASK 0x%x\n",
				 (unsigned int) YY_TRAILING_MASK);
			out_hex ("#define YY_TRAILING_HEAD_MASK 0x%x\n",
				 (unsigned int) YY_TRAILING_HEAD_MASK);
		}

		outn ("#define REJECT \\");
		outn ("{ \\");
		outn ("*yy_cp = YY_G(yy_hold_char); /* undo effects of setting up yytext */ \\");	// POINTER
		outn ("yy_cp = YY_G(yy_full_match); /* restore poss. backed-over text */ \\");

		if (variable_trailing_context_rules) {
			outn ("YY_G(yy_lp) = YY_G(yy_full_lp); /* restore orig. accepting pos. */ \\");
			outn ("YY_G(yy_state_ptr) = YY_G(yy_full_state); /* restore orig. state */ \\");
			outn ("yy_current_state = *YY_G(yy_state_ptr); /* restore curr. state */ \\");
		}

		outn ("++YY_G(yy_lp); \\");
		outn ("goto find_rule; \\");

		outn (backend->close_block);
		outn ("]])\n");
	}

	else {
		outn ("/* The intent behind this definition is that it'll catch");
		outn (" * any uses of REJECT which flex missed.");
		outn (" */");
		outn ("#define REJECT reject_used_but_not_detected");
	}

	if (yymore_used) {
		if (!C_plus_plus) {
			if (yytext_is_array) {
				if (!reentrant){
    				indent_puts ("static int yy_more_offset = 0;");
                    indent_puts ("static int yy_prev_more_offset = 0;");
                }
			}
			else if (!reentrant) {
				indent_puts
					("static int yy_more_flag = 0;");
				indent_puts
					("static int yy_more_len = 0;");
			}
		}

		if (yytext_is_array) {
			indent_puts
				("#define yymore() (YY_G(yy_more_offset) = yy_flex_strlen( yytext M4_YY_CALL_LAST_ARG))");
			indent_puts ("#define YY_NEED_STRLEN");
			indent_puts ("#define YY_MORE_ADJ 0");
			indent_puts
				("#define YY_RESTORE_YY_MORE_OFFSET \\");
			++indent_level;
			indent_puts ("{ \\");
			indent_puts
				("YY_G(yy_more_offset) = YY_G(yy_prev_more_offset); \\");
			indent_puts ("yyleng -= YY_G(yy_more_offset); \\");
			close_block();
		}
		else {
			indent_puts
				("#define yymore() (YY_G(yy_more_flag) = 1)");
			indent_puts
				("#define YY_MORE_ADJ YY_G(yy_more_len)");
			indent_puts ("#define YY_RESTORE_YY_MORE_OFFSET");
		}
	}

	else {
		indent_puts
			("#define yymore() yymore_used_but_not_detected");
		indent_puts ("#define YY_MORE_ADJ 0");
		indent_puts ("#define YY_RESTORE_YY_MORE_OFFSET");
	}

	if (!C_plus_plus) {
		if (yytext_is_array) {
			outn ("#ifndef YYLMAX");
			outn ("#define YYLMAX 8192");
			outn ("#endif\n");
			if (!reentrant){
                outn ("char yytext[YYLMAX];");
                outn ("char *yytext_ptr;");
            }
		}

		else {
			if(! reentrant)
                outn ("char *yytext;");
		}
	}

	out (&action_array[defs1_offset]);

	line_directive_out (stdout, 0);

	skelout ();		/* %% [5.0] - break point in skel */

	if (!C_plus_plus) {
		if (use_read) {
			outn ("\terrno=0; \\");
			outn ("\twhile ( (result = (int) read( fileno(yyin), buf, (yy_size_t) max_size )) < 0 ) \\");
			outn ("\t{ \\");
			outn ("\t\tif( errno != EINTR) \\");
			outn ("\t\t{ \\");
			outn ("\t\t\tYY_FATAL_ERROR( \"input in flex scanner failed\" ); \\");
			outn ("\t\t\tbreak; \\");
			outn ("\t\t} \\");
			outn ("\t\terrno=0; \\");
			outn ("\t\tclearerr(yyin); \\");
			outn ("\t}\\");
		}

		else {
			outn ("\tif ( YY_CURRENT_BUFFER_LVALUE->yy_is_interactive ) \\");
			outn ("\t\t{ \\");
			outn ("\t\tint c = '*'; \\");
			outn ("\t\tint n; \\");
			outn ("\t\tfor ( n = 0; n < max_size && \\");
			outn ("\t\t\t     (c = getc( yyin )) != EOF && c != '\\n'; ++n ) \\");
			outn ("\t\t\tbuf[n] = (char) c; \\");
			outn ("\t\tif ( c == '\\n' ) \\");
			outn ("\t\t\tbuf[n++] = (char) c; \\");
			outn ("\t\tif ( c == EOF && ferror( yyin ) ) \\");
			outn ("\t\t\tYY_FATAL_ERROR( \"input in flex scanner failed\" ); \\");
			outn ("\t\tresult = n; \\");
			outn ("\t\t} \\");
			outn ("\telse \\");
			outn ("\t\t{ \\");
			outn ("\t\terrno=0; \\");
			outn ("\t\twhile ( (result = (int) fread(buf, 1, (yy_size_t) max_size, yyin)) == 0 && ferror(yyin)) \\");
			outn ("\t\t\t{ \\");
			outn ("\t\t\tif( errno != EINTR) \\");
			outn ("\t\t\t\t{ \\");
			outn ("\t\t\t\tYY_FATAL_ERROR( \"input in flex scanner failed\" ); \\");
			outn ("\t\t\t\tbreak; \\");
			outn ("\t\t\t\t} \\");
			outn ("\t\t\terrno=0; \\");
			outn ("\t\t\tclearerr(yyin); \\");
			outn ("\t\t\t} \\");
			outn ("\t\t}\\");
		}
	}

	skelout ();		/* %% [6.0] - break point in skel */

	indent_puts ("#define YY_RULE_SETUP \\");
	++indent_level;
	if (bol_needed) {
		indent_puts ("if ( yyleng > 0 ) \\");
		++indent_level;
		indent_puts ("YY_CURRENT_BUFFER_LVALUE->yy_at_bol = \\");
		indent_puts ("\t\t(yytext[yyleng - 1] == '\\n'); \\");
		--indent_level;
	}
	indent_puts ("YY_USER_ACTION");
	--indent_level;

	skelout ();		/* %% [7.0] - break point in skel */

	/* Copy prolog to output file. */
	out (&action_array[prolog_offset]);

	line_directive_out (stdout, 0);

	skelout ();		/* %% [8.0] - break point in skel */

	set_indent (2);

	if (yymore_used && !yytext_is_array) {
		indent_puts ("YY_G(yy_more_len) = 0;");
		indent_puts ("if ( YY_G(yy_more_flag) )");
		open_block();
		indent_puts
			("YY_G(yy_more_len) = (int) (YY_G(yy_c_buf_p) - YY_G(yytext_ptr));");
		indent_puts ("YY_G(yy_more_flag) = 0;");
		close_block();
	}

	skelout ();		/* %% [9.0] - break point in skel */

	outn("M4_GEN_START_STATE");

	/* Note, don't use any indentation. */
	outn ("yy_match:");

	skelout ();		/* %% [10.0] - break point in skel */

	skelout ();		/* %% [11.0] - break point in skel */
	set_indent (2);
	outn ("m4_ifdef( [[M4_YY_USE_LINENO]],[[");
	indent_puts
		("if ( yy_act != YY_END_OF_BUFFER && yy_rule_can_match_eol[yy_act] )");
	open_block();
	indent_puts ("int yyl;");
	do_indent ();
	out_str ("for ( yyl = %s; yyl < yyleng; ++yyl )\n",
		 yymore_used ? (yytext_is_array ? "YY_G(yy_prev_more_offset)" :
				"YY_G(yy_more_len)") : "0");
	++indent_level;
	indent_puts ("if ( yytext[yyl] == '\\n' )");
	++indent_level;
	indent_puts ("M4_YY_INCR_LINENO();");
	--indent_level;
       	close_block();
	--indent_level;
	outn ("]])");

	skelout ();		/* %% [12.0] - break point in skel */

	/* Copy actions to output file. */
	skelout ();		/* %% [13.0] - break point in skel */

	/* Conditionally generate the code to perform the backing up. */
	++indent_level;
	if (!reject) {
		set_indent (3);

		outn("M4_GEN_BU_ACTION");

		set_indent (0);
	}

	out (&action_array[action_offset]);

	line_directive_out (stdout, 0);

	/* generate cases for any missing EOF rules */
	for (i = 1; i <= lastsc; ++i)
		if (!sceof[i]) {
			do_indent ();
			out_str ("case YY_STATE_EOF(%s):\n", scname[i]);
			did_eof_rule = true;
		}

	if (did_eof_rule) {
		++indent_level;
		indent_puts ("yyterminate();");
		--indent_level;
	}


	/* Generate code for handling NUL's, if needed. */

	/* First, deal with backing up and setting up yy_cp if the scanner
	 * finds that it should JAM on the NUL.
	 */
	skelout ();		/* %% [14.0] - break point in skel */

	/* Generate code for yy_get_previous_state(). */
	skelout ();		/* %% [15.0] - break point in skel */

	outn("M4_GEN_START_STATE");

	set_indent (2);
	skelout ();		/* %% [16.0] - break point in skel */
	gen_next_state ();

	set_indent (1);
	skelout ();		/* %% [17.0] - break point in skel */
	gen_NUL_trans ();

	skelout ();		/* %% [18.0] - break point in skel */
	skelout ();		/* %% [19.0] - break point in skel */
	skelout ();

	/* Copy remainder of input to output. */

	line_directive_out (stdout, 1);

	if (sectnum == 3) {
		OUT_BEGIN_CODE ();
                if (!no_section3_escape)
                   fputs("[[", stdout);
		(void) flexscan ();	/* copy remainder of input to output */
                if (!no_section3_escape)
                   fputs("]]", stdout);
		OUT_END_CODE ();
	}
}
