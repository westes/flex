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

/* Almost everything is done in terms of arrays starting at 1, so provide
 * a null entry for the zero element of all C arrays.  (The exception
 * to this is that the fast table representation generally uses the
 * 0 elements of its arrays, too.)
 */

static const char *get_int_decl (void)
{
	return (gentables)
		? "DEFINE_ARRAY( %s, %d )"
		: "DEFINE_TABLE_EMPTY( %s )";
}

static const char *get_int32_decl (void)
{
	return (gentables)
               ? "DEFINE_LONG_ARRAY( %s, %d )"
               : "DEFINE_LONG_TABLE_EMPTY( %s )";
}

static const char *get_state_decl (void)
{
	return (gentables)
               ? "DEFINE_STATE_ARRAY( %s, %d )"
               : "DEFINE_STATE_TABLE_EMPTY( %s )";
}

static const char *get_typed_decl (void)
{
	return (gentables)
	       ? "DEFINE_TYPE_ARRAY( %s, %s, %d )"
	       : "DEFINE_TYPE_TABLE_EMPTY( %s, %s )";
}

static const char *get_yy_char_decl (void)
{
	return (gentables)
               ? "DEFINE_CHAR_ARRAY( %s, %d )"
               : "DEFINE_CHAR_TABLE_EMPTY( %s )";
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

	buf_strappend (&yydmap_buf, "INDENT[[]]YYDMAP_ENTRY_LONG(YYTD_ID_RULE_CAN_MATCH_EOL, yy_rule_can_match_eol)");
	return tbl;
}

/* Generate the table for possible eol matches. */
static void geneoltbl (void)
{
	int     i;

	outn ("m4_ifdef( [[M4_YY_USE_LINENO]],[[");
	outn ("/* Table of booleans, true if rule could match eol. */");
	out_str_dec (get_int32_decl (), "yy_rule_can_match_eol",
		     num_rules + 1);

	if (gentables) {
		for (i = 1; i <= num_rules; i++) {
			out_dec ("TABLE_DATA(%5d)[[]]COLUMN_SEPARATOR[[]]", rule_has_nl[i] ? 1 : 0);
			/* format nicely, 20 numbers per line. */
			if ((i % 20) == 19)
				out ("TABLE_BLOCK[[]]INDENT[[]]");
		}
		dataend();
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

        if( (tblend + numecs + 1) >= INT16_MAX ) {
		buf_strappend (&yydmap_buf, "INDENT[[]]YYDMAP_ENTRY_LONG(YYTD_ID_TRANSITION, yy_transition)");
        }
        else {
		buf_strappend (&yydmap_buf, "INDENT[[]]YYDMAP_ENTRY(YYTD_ID_TRANSITION, yy_transition)");
        }

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

	buf_strappend (&yydmap_buf, "INDENT[[]]" \
                       "YYDMAP_ENTRY_TYPE(YYTD_ID_START_STATE_LIST," \
                       "yy_start_state_list, M4_YY_TRANS_INFO_P_TYPE)");

	return tbl;
}



/* genctbl - generates full speed compressed transition table */

void genctbl (void)
{
	int i;
	int     end_of_buffer_action = num_rules + 1;

	/* Table of verify for transition and offset to next state. */
	if (gentables)
		out_str2_dec (get_typed_decl(), "M4_YY_TRANS_INFO_TYPE", "yy_transition", tblend + numecs + 1);
              
	else
		out_str2 (get_typed_decl(), "M4_YY_TRANS_INFO_P_TYPE", "yy_transition");

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

	dataend();

	/* Table of pointers to start states. */
	if (gentables)
		out_str2_dec (get_typed_decl(), "M4_YY_TRANS_INFO_P_TYPE", "yy_start_state_list", lastsc * 2 + 1);
	else
		out_str2 (get_typed_decl(), "M4_YY_TRANS_INFO_PP_TYPE", "yy_start_state_list");

	if (gentables) {
		

		for (i = 0; i <= lastsc * 2; ++i)
			out_dec ("REFERENCE_TABLE_ENTRY(yy_transition, %d)[[]]COLUMN_SEPARATOR[[]]TABLE_BLOCK[[]]", base[i]);

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

	buf_strappend (&yydmap_buf, "INDENT[[]]YYDMAP_ENTRY_TYPE(YYTD_ID_EC, yy_ec, YY_CHAR)");

	return tbl;
}

/* Generate equivalence-class tables. */

void genecs (void)
{
	int ch, row;
	int     numrows;

	out_str_dec (get_yy_char_decl (), "yy_ec", csize);

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

	buf_strappend (&yydmap_buf, "INDENT[[]]YYDMAP_ENTRY(YYTD_ID_ACCEPT, yy_accept)");

	return tbl;
}


/* genftbl - generate full transition table */

void genftbl (void)
{
	int i;
	int     end_of_buffer_action = num_rules + 1;

	out_str_dec (get_int_decl (), "yy_accept", lastdfa + 1);

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
	++num_backing_up;

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

		out_str_dec (get_int_decl(), "yy_acclist", MAX (numas, 1) + 1);
        
        buf_strappend (&yydmap_buf, "INDENT[[]]YYDMAP_ENTRY(YYTD_ID_ACCLIST, yy_acclist)");

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

	out_str_dec (get_int_decl (), "yy_accept", k);

	buf_strappend (&yydmap_buf, "INDENT[[]]YYDMAP_ENTRY(YYTD_ID_ACCEPT, yy_accept)");

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

		out_str_dec (get_yy_char_decl (), "yy_meta", numecs + 1);
		buf_strappend (&yydmap_buf,"INDENT[[]]YYDMAP_ENTRY_TYPE(YYTD_ID_META, yy_meta, YY_CHAR)");

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
		     get_int32_decl () : get_int_decl (),
		     "yy_base", total_states + 1);

	if( tblend >= INT16_MAX ) {
		buf_strappend (&yydmap_buf, "INDENT[[]]YYDMAP_ENTRY_TYPE(YYTD_ID_BASE, yy_base, flex_int32_t)");
	}
	else {
		buf_strappend (&yydmap_buf, "INDENT[[]]YYDMAP_ENTRY_TYPE(YYTD_ID_BASE, yy_base, YY_INT_ALIGNED)");
	}
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
		     get_int32_decl () : get_int_decl (),
		     "yy_def", total_states + 1);

	if( total_states >= INT16_MAX ) {
          buf_strappend (&yydmap_buf, "INDENT[[]]YYDMAP_ENTRY_TYPE(YYTD_ID_DEF, yy_def, flex_int32_t)");
	}
	else {
          buf_strappend (&yydmap_buf, "INDENT[[]]YYDMAP_ENTRY_TYPE(YYTD_ID_DEF, yy_def, YY_INT_ALIGNED)");
	}
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
		     get_int32_decl () : get_int_decl (), "yy_nxt",
		     tblend + 1);

	if( total_states >= INT16_MAX ) {
          buf_strappend (&yydmap_buf, "INDENT[[]]YYDMAP_ENTRY_TYPE(YYTD_ID_NXT, yy_nxt, flex_int32_t)");
	}
	else {
          buf_strappend (&yydmap_buf, "INDENT[[]]YYDMAP_ENTRY_TYPE(YYTD_ID_NXT, yy_nxt, YY_INT_ALIGNED)");
	}
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
		     get_int32_decl () : get_int_decl (), "yy_chk",
		     tblend + 1);

	if( total_states >= INT16_MAX ) {
          buf_strappend (&yydmap_buf, "INDENT[[]]YYDMAP_ENTRY_TYPE(YYTD_ID_CHK, yy_chk, flex_int32_t)");
	}
	else {
          buf_strappend (&yydmap_buf, "INDENT[[]]YYDMAP_ENTRY_TYPE(YYTD_ID_CHK, yy_chk, YY_INT_ALIGNED)");
	}
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


/* make_tables - generate transition tables and finishes generating output file
 */

void make_tables (void)
{
	int i;
	int did_eof_rule = false;
	struct yytbl_data *yynultrans_tbl = NULL;

	out_m4_define_dec("M4_YY_NUM_RULES", num_rules);
	out_m4_define_dec("M4_YY_END_OF_BUFFER", num_rules + 1);
        
        if (interactive)
          out_m4_define( "M4_YY_INTERACTIVE", NULL );
        
        if (!fullspd && !fulltbl) {
          out_m4_define_dec( "M4_YY_JAMBASE", jambase );
          out_m4_define_dec( "M4_YY_JAMSTATE", jamstate );
        }
        
        if (nultrans) {
          out_m4_define( "M4_YY_NULTRANS", NULL );
        }
        
        if (usemecs) {
          out_m4_define( "M4_YY_USE_MECS", NULL );
          out_m4_define_dec( "M4_YY_LASTDFA", lastdfa );
        }
        
        if(variable_trailing_context_rules) {
          out_m4_define("M4_VARIABLE_TRAILING_CONTEXT_RULES", NULL);
        }
        out_m4_define_hex ("M4_YY_TRAILING_MASK", (unsigned int) YY_TRAILING_MASK);
        out_m4_define_hex ("M4_YY_TRAILING_HEAD_MASK", (unsigned int) YY_TRAILING_HEAD_MASK);
        
        if (use_read) {
          out_m4_define("M4_YY_USE_READ", NULL);
        }

        if (bol_needed) {
          out_m4_define("M4_YY_BOL_NEEDED", NULL);
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
	else {
		gentabs ();
	}
              
        /* num_backing_up can be changed by gentabs() so we don't  no whether backing up is needed until here. */
	if( num_backing_up > 0 ) {
		out_m4_define("M4_YY_NEEDS_BACKING_UP", NULL);
	}
              
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



	if (nultrans) {
		flex_int32_t *yynultrans_data = 0;

		/* Begin generating yy_NUL_trans */
		out_str_dec (get_state_decl (), "yy_NUL_trans", lastdfa + 1);
                
		if(fullspd) {
			buf_strappend (&yydmap_buf, "INDENT[[]]YYDMAP_ENTRY_TYPE(YYTD_ID_NUL_TRANS, yy_NUL_trans, M4_YY_TRANS_INFO_P_TYPE)");
		}
		else {
			buf_strappend (&yydmap_buf, "INDENT[[]]YYDMAP_ENTRY_LONG(YYTD_ID_NUL_TRANS, yy_NUL_trans)");
		}

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
				out_dec ("INDENT[[]]REFERENCE_TABLE_ENTRY(yy_transition, %d)[[]]COLUMN_SEPARATOR[[]]TABLE_BLOCK[[]]",
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

	if (ddebug) {		/* Spit out table mapping rules to line numbers. */
		out_str_dec (get_int_decl (), "yy_rule_linenum", num_rules);
		for (i = 1; i < num_rules; ++i)
			mkdata (rule_linenum[i]);
		dataend ();
	}

    
	/* Emit the user's section 1 definitions into the output file. */
	skelout ();		/* %% [3.0] - break point in skel */
	out (&action_array[defs1_offset]);
	line_directive_out (stdout, 0);
    
	/* Emit the user's prolog into the output file. */
	skelout ();		/* %% [4.0] - break point in skel */
	out (&action_array[prolog_offset]);
	line_directive_out (stdout, 0);

	/* Copy actions to output file. */
	skelout ();		/* %% [5.0] - break point in skel */
	out (&action_array[action_offset]);
	line_directive_out (stdout, 0);

        
	/* generate cases for any missing EOF rules */
	for (i = 1; i <= lastsc; ++i)
		if (!sceof[i]) {
			out_str ("EOF_RULE(%s)", scname[i]);
			did_eof_rule = true;
		}

	if (did_eof_rule) {
                out_m4_define("M4_YY_DID_EOF_RULE", NULL);
	}


	/* Copy remainder of skeleton to output. */
	skelout (); /* %% [6.0] - break point in skel */
	line_directive_out (stdout, 1);

        /* Emit the user's section 3 into the output file. */
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
