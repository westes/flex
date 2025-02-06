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
#include "skeletons.h"

/* These typedefs are only used for computing footprint sizes,
 * You need to make sure they match reality in the skeleton file to
 * get accurate numbers, but they don't otherwise matter.
 */
typedef char YY_CHAR;
struct yy_trans_info {int32_t yy_verify; int32_t yy_nxt;};

/* declare functions that have forward references */

static void genecs(void);

struct packtype_t *optimize_pack(size_t sz)
{
	/* FIXME: There's a 32-bit assumption lurking here */
	static struct packtype_t out;
	if (sz == 0) {
		out.name  = ctrl.long_align ? "M4_HOOK_INT32" : "M4_HOOK_INT16";
		out.width = ctrl.long_align ? 32 : 16;
	} else {
		out.name = (ctrl.long_align || sz >= INT16_MAX) ? "M4_HOOK_INT32" : "M4_HOOK_INT16";
		out.width = (ctrl.long_align || sz >= INT16_MAX) ? 32 : 16;
	}
	return &out;
}

/* Almost everything is done in terms of arrays starting at 1, so provide
 * a null entry for the zero element of all C arrays.  (The exception
 * to this is that the fast table representation generally uses the
 * 0 elements of its arrays, too.)
 */

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

	return tbl;
}

/* Generate the table for possible eol matches. */
static void geneoltbl (void)
{
	int     i;
	struct packtype_t *ptype = optimize_pack(num_rules);
	const struct flex_backend_t *bend = get_backend();

	bend->verbatim(bend, "m4_ifdef( [[M4_MODE_YYLINENO]],[[");
	bend->newline(bend);
	bend->filter_define_vars(bend, "M4_HOOK_EOLTABLE_TYPE", ptype->name);
	bend->filter_define_vard(bend, "M4_HOOK_EOLTABLE_SIZE", num_rules + 1);
	bend->filter_define_name(bend, "M4_HOOK_EOLTABLE_BODY", true);
	bend->newline(bend);

	if (gentables) {
		for (i = 1; i <= num_rules; i++) {
			out_dec ("%d, ", rule_has_nl[i] ? 1 : 0);
			/* format nicely, 20 numbers per line. */
			if ((i % 20) == 19)
				bend->newline(bend);
				bend->indent(bend);
		}
	}
	footprint += num_rules * ptype->width;
	bend->filter_define_close(bend, NULL); /* End EOLTABLE_BODY */
	bend->newline(bend);
	bend->verbatim(bend, "]])"); /* End m4_ifdef( [[M4_MODE_YYLINENO]]...*/
	bend->newline(bend);
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

	struct packtype_t *ptype = optimize_pack(tblend + 2 + 1);
	out_str ("m4_define([[M4_HOOK_MKCTBL_TYPE]], [[%s]])", ptype->name);

	tbl = calloc(1, sizeof (struct yytbl_data));
	yytbl_data_init (tbl, YYTD_ID_TRANSITION);
	tbl->td_flags = YYTD_DATA32 | YYTD_STRUCT;
	tbl->td_hilen = 0;
	tbl->td_lolen = (flex_uint32_t) (tblend + 2 + 1);	/* number of structs */

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

	return tbl;
}



/* genctbl - generates full speed compressed transition table */

static void genctbl(void)
{
	int i;
	int     end_of_buffer_action = num_rules + 1;
	const struct flex_backend_t *bend = get_backend();

	/* Table of verify for transition and offset to next state. */
	bend->filter_define_vard(bend, "M4_HOOK_TRANSTABLE_SIZE", tblend + 2 + 1);
	bend->filter_define_name(bend, "M4_HOOK_TRANSTABLE_BODY", true);
	bend->newline(bend);

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

	bend->filter_define_close(bend, NULL); /* End TRANSTABLE_BODY */
	bend->newline(bend);
	footprint += sizeof(struct yy_trans_info) * (tblend + 2 + 1);

	bend->filter_define_vard(bend, "M4_HOOK_STARTTABLE_SIZE", lastsc * 2 + 1);
	if (gentables) {
		bend->filter_define_name(bend, "M4_HOOK_STARTTABLE_BODY", true);
		bend->newline(bend);
		for (i = 0; i <= lastsc * 2; ++i)
			bend->format_state_table_entry(bend, base[i]);

		dataend (false);
		bend->filter_define_close(bend, NULL); /* End STARTTABLE_BODY */
		bend->newline(bend);
		footprint +=  sizeof(struct yy_trans_info *) * (lastsc * 2 + 1);
	}

	if (ctrl.useecs)
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
	tbl->td_lolen = (flex_uint32_t) ctrl.csize;

	tbl->td_data = tdata =
		calloc(tbl->td_lolen, sizeof (flex_int32_t));

	for (i = 1; i < ctrl.csize; ++i) {
		ecgroup[i] = ABS (ecgroup[i]);
		tdata[i] = ecgroup[i];
	}

	return tbl;
}

/* Generate equivalence-class tables. */

static void genecs(void)
{
	int ch, row;
	int     numrows;
	const struct flex_backend_t *bend = get_backend();

	bend->filter_define_vard(bend, "M4_HOOK_ECSTABLE_SIZE", ctrl.csize);
	bend->filter_define_name(bend, "M4_HOOK_ECSTABLE_BODY", true);
	bend->newline(bend);

	for (ch = 1; ch < ctrl.csize; ++ch) {
		ecgroup[ch] = ABS (ecgroup[ch]);
		mkdata (ecgroup[ch]);
	}

	dataend (false);
	bend->filter_define_close(bend, NULL); /* End ECSTABLE_BODY */
	bend->newline(bend);
	footprint += sizeof(YY_CHAR) * ctrl.csize;

	if (env.trace) {
		fputs (_("\n\nEquivalence Classes:\n\n"), stderr);

		/* Print in 8 columns */
		numrows = ctrl.csize / 8;

		for (row = 0; row < numrows; ++row) {
			for (ch = row; ch < ctrl.csize; ch += numrows) {
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

static struct yytbl_data *mkftbl(void)
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

		if (env.trace && anum)
			fprintf (stderr, _("state # %d accepts: [%d]\n"),
				 i, anum);
	}

	return tbl;
}


/* genftbl - generate full transition table */

static void genftbl(void)
{
	int i;
	int     end_of_buffer_action = num_rules + 1;
	struct packtype_t *ptype = optimize_pack(num_rules + 1);
	const struct flex_backend_t *bend = get_backend();

	dfaacc[end_of_buffer_state].dfaacc_state = end_of_buffer_action;

	bend->filter_define_vard(bend, "M4_HOOK_NEED_ACCEPT", 1);
	bend->newline(bend);
	bend->filter_define_vars(bend, "M4_HOOK_ACCEPT_TYPE", ptype->name);
	bend->filter_define_vard(bend, "M4_HOOK_ACCEPT_SIZE", lastdfa + 1);
	bend->filter_define_name(bend, "M4_HOOK_ACCEPT_BODY", true);
	bend->newline(bend);

	for (i = 1; i <= lastdfa; ++i) {
		int anum = dfaacc[i].dfaacc_state;

		mkdata (anum);

		if (env.trace && anum)
			fprintf (stderr, _("state # %d accepts: [%d]\n"),
				 i, anum);
	}

	dataend (false);
	bend->filter_define_close(bend, NULL); /* End ACCEPT_BODY*/
	bend->newline(bend);
	footprint += (lastdfa + 1) * ptype->width;

	if (ctrl.useecs)
		genecs ();

	/* Don't have to dump the actual full table entries - they were
	 * created on-the-fly.
	 */
}

/* gentabs - generate data statements for the transition tables */

static void gentabs(void)
{
	int     sz, i, j, k, *accset, nacc, *acc_array, total_states;
	int     end_of_buffer_action = num_rules + 1;
	struct yytbl_data *yyacc_tbl = 0, *yymeta_tbl = 0, *yybase_tbl = 0,
	    *yydef_tbl = 0, *yynxt_tbl = 0, *yychk_tbl = 0, *yyacclist_tbl=0;
	flex_int32_t *yyacc_data = 0, *yybase_data = 0, *yydef_data = 0,
	    *yynxt_data = 0, *yychk_data = 0, *yyacclist_data=0;
	flex_int32_t yybase_curr = 0, yyacclist_curr=0,yyacc_curr=0;
	struct packtype_t *ptype;
	const struct flex_backend_t *bend = get_backend();

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

		sz = MAX (numas, 1) + 1;
		ptype = optimize_pack(sz);
		out_str ("m4_define([[M4_HOOK_ACCLIST_TYPE]], [[%s]])", ptype->name);
		out_dec ("m4_define([[M4_HOOK_ACCLIST_SIZE]], [[%d]])", sz);
		bend->filter_define_name(bend, "M4_HOOK_ACCLIST_BODY", true);
		bend->newline(bend);

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

				if (env.trace)
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

					if (env.trace) {
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

		dataend (false);
		bend->filter_define_close(bend, NULL); /* End ACCLIST_BODY*/
		bend->newline(bend);
		footprint += sz * ptype->width;
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
	sz = lastdfa + 2;

	if (reject)
		/* We put a "cap" on the table associating lists of accepting
		 * numbers with state numbers.  This is needed because we tell
		 * where the end of an accepting list is by looking at where
		 * the list for the next state starts.
		 */
		++sz;

	/* Note that this table is alternately defined if ctrl.fulltbl */
	ptype = optimize_pack(sz);
	bend->filter_define_vard(bend, "M4_HOOK_NEED_ACCEPT", 1);
	bend->newline(bend);
	bend->filter_define_vars(bend, "M4_HOOK_ACCEPT_TYPE", ptype->name);
	bend->filter_define_vard(bend, "M4_HOOK_ACCEPT_SIZE", sz);
	bend->filter_define_name(bend, "M4_HOOK_ACCEPT_BODY", true);
	bend->newline(bend);

	yyacc_tbl = calloc(1, sizeof (struct yytbl_data));
	yytbl_data_init (yyacc_tbl, YYTD_ID_ACCEPT);
	yyacc_tbl->td_lolen = (flex_uint32_t) sz;
	yyacc_tbl->td_data = yyacc_data =
	    calloc(yyacc_tbl->td_lolen, sizeof (flex_int32_t));
	yyacc_curr=1;

	for (i = 1; i <= lastdfa; ++i) {
		mkdata (acc_array[i]);
		yyacc_data[yyacc_curr++] = acc_array[i];

		if (!reject && env.trace && acc_array[i])
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

	dataend (false);
	bend->filter_define_close(bend, NULL); /* End ACCEPT_BODY*/
	bend->newline(bend);
	footprint += sz * ptype->width;

	if (tablesext) {
		yytbl_data_compress (yyacc_tbl);
		if (yytbl_data_fwrite (&tableswr, yyacc_tbl) < 0)
			flexerror (_("Could not write yyacc_tbl"));
	}
	yytbl_data_destroy (yyacc_tbl);
	yyacc_tbl = NULL;
	/* End generating yy_accept */

	if (ctrl.useecs) {

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

	if (ctrl.usemecs) {
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

		if (env.trace)
			fputs (_("\n\nMeta-Equivalence Classes:\n"),
			       stderr);
		bend->filter_define_vard(bend, "M4_HOOK_MECSTABLE_SIZE", numecs+1);
		bend->filter_define_name(bend, "M4_HOOK_MECSTABLE_BODY", true);
		bend->newline(bend);
 	
		for (i = 1; i <= numecs; ++i) {
			if (env.trace)
				fprintf (stderr, "%d = %d\n",
					 i, ABS (tecbck[i]));

			mkdata (ABS (tecbck[i]));
			yymecs_data[i] = ABS (tecbck[i]);
		}

		dataend (false);
		bend->filter_define_close(bend, NULL); /* End MECSTABLE_BODY */
		bend->newline(bend);
		footprint += sizeof(YY_CHAR) * (numecs + 1);
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
	sz = total_states + 1;
	ptype = optimize_pack(sz);
	bend->filter_define_vars(bend, "M4_HOOK_BASE_TYPE", ptype->name);
	bend->filter_define_vard(bend, "M4_HOOK_BASE_SIZE", sz);
	bend->filter_define_name(bend, "M4_HOOK_BASE_BODY", true);
	bend->newline(bend);

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

	dataend (false);
	bend->filter_define_close(bend, NULL); /* End BASE_BODY */
	bend->newline(bend);
	footprint += sz * ptype->width;

	if (tablesext) {
		yytbl_data_compress (yybase_tbl);
		if (yytbl_data_fwrite (&tableswr, yybase_tbl) < 0)
			flexerror (_("Could not write yybase_tbl"));
	}
	yytbl_data_destroy (yybase_tbl);
	yybase_tbl = NULL;
	/* End generating yy_base */


	/* Begin generating yy_def */
	ptype = optimize_pack(total_states + 1);
	bend->filter_define_vars(bend, "M4_HOOK_DEF_TYPE", ptype->name);
	bend->filter_define_vard(bend, "M4_HOOK_DEF_SIZE", total_states + 1);
	bend->filter_define_name(bend, "M4_HOOK_DEF_BODY", true);
	bend->newline(bend);

	yydef_tbl = calloc(1, sizeof (struct yytbl_data));
	yytbl_data_init (yydef_tbl, YYTD_ID_DEF);
	yydef_tbl->td_lolen = (flex_uint32_t)(total_states + 1);
	yydef_tbl->td_data = yydef_data =
	    calloc(yydef_tbl->td_lolen, sizeof (flex_int32_t));

	for (i = 1; i <= total_states; ++i) {
		mkdata (def[i]);
		yydef_data[i] = def[i];
	}

	dataend (false);
	bend->filter_define_close(bend, NULL); /* End DEF_BODY */
	bend->newline(bend);
	footprint += (total_states + 1) * ptype->width;

	if (tablesext) {
		yytbl_data_compress (yydef_tbl);
		if (yytbl_data_fwrite (&tableswr, yydef_tbl) < 0)
			flexerror (_("Could not write yydef_tbl"));
	}
	yytbl_data_destroy (yydef_tbl);
	yydef_tbl = NULL;
	/* End generating yy_def */


	ptype = optimize_pack(tblend + 1);
	/* Note: Used when !ctrl.fulltbl && !ctrl.fullspd).
	 * (Alternately defined when ctrl.fullspd)
	 */
	bend->filter_define_vars(bend, "M4_HOOK_YYNXT_TYPE", ptype->name);
	bend->filter_define_vard(bend, "M4_HOOK_YYNXT_SIZE", tblend + 1);
	bend->filter_define_name(bend, "M4_HOOK_YYNXT_BODY", true);
	bend->newline(bend);

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

	dataend (false);
	bend->filter_define_close(bend, NULL); /* End YYNXT_BODY */
	bend->newline(bend);
	footprint += ptype->width * (tblend + 1);

	if (tablesext) {
		yytbl_data_compress (yynxt_tbl);
		if (yytbl_data_fwrite (&tableswr, yynxt_tbl) < 0)
			flexerror (_("Could not write yynxt_tbl"));
	}
	yytbl_data_destroy (yynxt_tbl);
	yynxt_tbl = NULL;
	/* End generating yy_nxt */

	/* Begin generating yy_chk */
	ptype = optimize_pack(tblend + 1);
	bend->filter_define_vars(bend, "M4_HOOK_CHK_TYPE", ptype->name);
	bend->filter_define_vard(bend, "M4_HOOK_CHK_SIZE", tblend + 1);
	bend->filter_define_name(bend, "M4_HOOK_CHK_BODY", true);
	bend->newline(bend);
	
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

	dataend (false);
	bend->filter_define_close(bend, NULL); /* End CHK_BODY */
	bend->newline(bend);
	footprint += ptype->width * (tblend + 1);

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


void visible_define (const char *symname)
{
	const struct flex_backend_t *bend = get_backend();

	bend->filter_define_name(bend, symname, false);
	bend->comment(bend, symname);
	bend->newline(bend);
}

void visible_define_str (const char *symname, const char *val)
{
	char buf[128];
	const struct flex_backend_t *bend = get_backend();

	bend->filter_define_vars(bend, symname, val);
	snprintf(buf, sizeof(buf), "%s = %s", symname, val);
	bend->comment(bend, buf);
	bend->newline(bend);
}

void visible_define_int (const char *symname, const int val)
{
	char buf[128];
	const struct flex_backend_t *bend = get_backend();

	bend->filter_define_vard(bend, symname, val);
	snprintf(buf, sizeof(buf), "%s = %d", symname, val);
	bend->comment(bend, buf);
	bend->newline(bend);
}

/* make_tables - generate transition tables
 */

void make_tables (void)
{
	char buf[128];
	int i;
	struct yytbl_data *yynultrans_tbl = NULL;
	const struct flex_backend_t *bend = get_backend();

	/* This is where we REALLY begin generating the tables. */

	if (ctrl.fullspd) {
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

			if (ctrl.useecs) {
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
	else if (ctrl.fulltbl) {
		genftbl ();
		if (tablesext) {
			struct yytbl_data *tbl;

			/* Alternately defined if !ctrl.ffullspd && !ctrl.fulltbl */
			struct packtype_t *ptype;
			tbl = mkftbl ();
			yytbl_data_compress (tbl);
			ptype = optimize_pack(tbl->td_lolen);
			out_str ("m4_define([[M4_HOOK_ACCEPT_TYPE]], [[%s]])", ptype->name);
			if (yytbl_data_fwrite (&tableswr, tbl) < 0)
				flexerror (_("Could not write ftbl"));
			yytbl_data_destroy (tbl);
			tbl = 0;

			if (ctrl.useecs) {
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

	snprintf(buf, sizeof(buf), "footprint: %ld bytes", footprint);
	bend->comment(bend, buf);
	bend->newline(bend);
	snprintf(buf, sizeof(buf), "tblend: %d", tblend);
	bend->comment(bend, buf);
	bend->newline(bend);
	snprintf(buf, sizeof(buf), "numecs: %d", numecs);
	bend->comment(bend, buf);
	bend->newline(bend);
	snprintf(buf, sizeof(buf), "num_rules: %d", num_rules);
	bend->comment(bend, buf);
	bend->newline(bend);
	snprintf(buf, sizeof(buf), "lastdfa: %d", lastdfa);
	bend->comment(bend, buf);
	bend->newline(bend);
	bend->newline(bend);
	
	// Only at this point do we know if the automaton has backups.
	// Some m4 conditionals require this information.

	bend->comment(bend, "m4 controls begin");
	bend->newline(bend);

	if (num_backing_up > 0)
		visible_define ( "M4_MODE_HAS_BACKING_UP");

	// These are used for NUL transitions
	if ((num_backing_up > 0 && !reject) && (!nultrans || ctrl.fullspd || ctrl.fulltbl))
		visible_define ( "M4_MODE_NEED_YY_CP");
	if ((num_backing_up > 0 && !reject) && (ctrl.fullspd || ctrl.fulltbl))
		visible_define ( "M4_MODE_NULTRANS_WRAP");

	bend->comment(bend, "m4 controls end");
	bend->newline(bend);

	if (ctrl.do_yylineno) {

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
		bend->filter_define_vars(bend, "M4_HOOK_NULTRANS_TYPE", bend->get_state_type(bend));
		bend->filter_define_vard(bend, "M4_HOOK_NULTRANS_SIZE", lastdfa + 1);
		bend->filter_define_name(bend, "M4_HOOK_NULTRANS_BODY", true);
		bend->newline(bend);

		yynultrans_tbl = calloc(1, sizeof (struct yytbl_data));
		yytbl_data_init (yynultrans_tbl, YYTD_ID_NUL_TRANS);
		// Performance kludge for C. Gives a small improvement
		// in table loading time.
		if (ctrl.fullspd && ctrl.have_state_entry_format)
			yynultrans_tbl->td_flags |= YYTD_PTRANS;
		yynultrans_tbl->td_lolen = (flex_uint32_t) (lastdfa + 1);
		yynultrans_tbl->td_data = yynultrans_data =
		    calloc(yynultrans_tbl->td_lolen,
			   sizeof (flex_int32_t));

		for (i = 1; i <= lastdfa; ++i) {
			if ((yynultrans_tbl->td_flags & YYTD_PTRANS) != 0) {
				// Only works in very C-like languages  
				out_dec ("    &yy_transition[%d],\n",
					 base[i]);
				yynultrans_data[i] = base[i];
			}
			else {
				// This will work anywhere
				mkdata (nultrans[i]);
				yynultrans_data[i] = nultrans[i];
			}
		}

		dataend (false);
		bend->filter_define_close(bend, NULL); /* End NULTRANS_BODY */
		bend->newline(bend);
		/* footprint is only used to print an estimated table size in a user-requested summary.
		   There's a C assumtption in the calculation here, but it doesn't affect the scanner.
		*/
		footprint += (lastdfa + 1) * (ctrl.fullspd ? sizeof(struct yy_trans_info *) : sizeof(int32_t));
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

	if (ctrl.ddebug) {		/* Spit out table mapping rules to line numbers. */
		/* Policy choice: we don't include this space
		 * in the table metering.
		 */
		struct packtype_t *ptype = optimize_pack(num_rules);
		bend->filter_define_vars(bend, "M4_HOOK_DEBUGTABLE_TYPE", ptype->name);
		bend->filter_define_vard(bend, "M4_HOOK_DEBUGTABLE_SIZE", num_rules);
		bend->filter_define_name(bend, "M4_HOOK_DEBUGTABLE_BODY", true);
		bend->newline(bend);

		for (i = 1; i < num_rules; ++i)
			mkdata (rule_linenum[i]);
		dataend (false);
		bend->filter_define_close(bend, NULL); /* End DEBUGTABLE_BODY */
		bend->newline(bend);
	}
}
