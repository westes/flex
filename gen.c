/* gen - actual generation (writing) of flex scanners */

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

#include "flexdef.h"

#ifndef lint
static char rcsid[] =
    "@(#) $Header$ (LBL)";
#endif


/* genctbl - generates full speed compressed transition table
 *
 * synopsis
 *     genctbl();
 */

genctbl()

    {
    register int i;

    /* table of verify for transition and offset to next state */
    printf( "static struct yy_trans_info yy_transition[%d] =\n",
	    tblend + numecs + 1 );
    printf( "    {\n" );
    
    /* We want the transition to be represented as the offset to the
     * next state, not the actual state number, which is what it currently is.
     * The offset is base[nxt[i]] - base[chk[i]].  That's just the
     * difference between the starting points of the two involved states
     * (to - from).
     *
     * first, though, we need to find some way to put in our end-of-buffer
     * flags and states.  We do this by making a state with absolutely no
     * transitions.  We put it at the end of the table.
     */
    /* at this point, we're guaranteed that there's enough room in nxt[]
     * and chk[] to hold tblend + numecs entries.  We need just two slots.
     * One for the action and one for the end-of-buffer transition.  We
     * now *assume* that we're guaranteed the only character we'll try to
     * index this nxt/chk pair with is EOB, i.e., 0, so we don't have to
     * make sure there's room for jam entries for other characters.
     */

    base[lastdfa + 1] = tblend + 2;
    nxt[tblend + 1] = END_OF_BUFFER_ACTION;
    chk[tblend + 1] = numecs + 1;
    chk[tblend + 2] = 1; /* anything but EOB */
    nxt[tblend + 2] = 0; /* so that "make test" won't show arb. differences */

    /* make sure every state has a end-of-buffer transition and an action # */
    for ( i = 0; i <= lastdfa; ++i )
	{
	register int anum = dfaacc[i].dfaacc_state;

	chk[base[i]] = EOB_POSITION;
	chk[base[i] - 1] = ACTION_POSITION;
	nxt[base[i] - 1] = anum ? anum : accnum + 1;	/* action number */
	}

    dataline = 0;
    datapos = 0;

    for ( i = 0; i <= tblend; ++i )
	{
	if ( chk[i] == EOB_POSITION )
	    transition_struct_out( 0, base[lastdfa + 1] - i );

	else if ( chk[i] == ACTION_POSITION )
	    transition_struct_out( 0, nxt[i] );

	else if ( chk[i] > numecs || chk[i] == 0 )
	    transition_struct_out( 0, 0 );		/* unused slot */

	else	/* verify, transition */
	    transition_struct_out( chk[i], base[nxt[i]] - (i - chk[i]) );
	}


    /* here's the final, end-of-buffer state */
    transition_struct_out( chk[tblend + 1], nxt[tblend + 1] );
    transition_struct_out( chk[tblend + 2], nxt[tblend + 2] );

    printf( "    };\n" );
    printf( "\n" );

    /* table of pointers to start states */
    printf( "static struct yy_trans_info *yy_state_ptr[%d] =\n",
	lastsc * 2 + 1 );
    printf( "    {\n" );

    for ( i = 0; i <= lastsc * 2; ++i )
	printf( "    &yy_transition[%d],\n", base[i] );

    printf( "    };\n" );

    if ( useecs )
	genecs();
    }


/* genftbl - generates full transition table
 *
 * synopsis
 *     genftbl();
 */

genftbl()

    {
    register int i;

    /* *everything* is done in terms of arrays starting at 1, so provide
     * a null entry for the zero element of all C arrays
     */
    static char C_short_decl[] = "static short int %c[%d] =\n    {   0,\n";
    static char C_char_decl[] = "static char %c[%d] =\n    {   0,\n";

#ifdef UNSIGNED_CHAR
    printf( C_short_decl, ALIST, lastdfa + 1 );
#else
    printf( accnum > 127 ? C_short_decl : C_char_decl, ALIST, lastdfa + 1 );
#endif

    for ( i = 1; i <= lastdfa; ++i )
	{
	register int anum = dfaacc[i].dfaacc_state;

	if ( i == end_of_buffer_state )
	    mkdata( END_OF_BUFFER_ACTION );

	else
	    mkdata( anum ? anum : accnum + 1 );

	if ( trace && anum )
	    fprintf( stderr, "state # %d accepts: [%d]\n", i, anum );
	}

    dataend();

    if ( useecs )
	genecs();

    /* don't have to dump the actual full table entries - they were created
     * on-the-fly
     */
    }


/* gentabs - generate data statements for the transition tables
 *
 * synopsis
 *    gentabs();
 */

gentabs()

    {
    int i, j, k, *accset, nacc, *acc_array, total_states;

    /* *everything* is done in terms of arrays starting at 1, so provide
     * a null entry for the zero element of all C arrays
     */
    static char C_long_decl[] = "static long int %c[%d] =\n    {   0,\n";
    static char C_short_decl[] = "static short int %c[%d] =\n    {   0,\n";
    static char C_char_decl[] = "static char %c[%d] =\n    {   0,\n";

    acc_array = allocate_integer_array( current_max_dfas );
    nummt = 0;

    printf( "#define YY_JAM %d\n", jamstate );
    printf( "#define YY_JAM_BASE %d\n", jambase );

    if ( usemecs )
	printf( "#define YY_TEMPLATE %d\n", lastdfa + 2 );

    if ( reject )
	{
	/* write out accepting list and pointer list
	 * first we generate the ACCEPT array.  In the process, we compute
	 * the indices that will go into the ALIST array, and save the
	 * indices in the dfaacc array
	 */

	printf( accnum > 127 ? C_short_decl : C_char_decl,
		ACCEPT, max( numas, 1 ) + 1 );

	j = 1;	/* index into ACCEPT array */

	for ( i = 1; i <= lastdfa; ++i )
	    {
	    acc_array[i] = j;

	    if ( accsiz[i] != 0 )
		{
		accset = dfaacc[i].dfaacc_set;
		nacc = accsiz[i];

		if ( trace )
		    fprintf( stderr, "state # %d accepts: ", i );

		for ( k = 1; k <= nacc; ++k )
		    {
		    ++j;
		    mkdata( accset[k] );

		    if ( trace )
			{
			fprintf( stderr, "[%d]", accset[k] );

			if ( k < nacc )
			    fputs( ", ", stderr );
			else
			    putc( '\n', stderr );
			}
		    }
		}
	    }

	/* add accepting number for the "jam" state */
	acc_array[i] = j;

	dataend();
	}

    else
	{
	for ( i = 1; i <= lastdfa; ++i )
	    acc_array[i] = dfaacc[i].dfaacc_state;

	/* add accepting number for jam state */
	acc_array[i] = 0;
	}

    /* spit out ALIST array.  If we're doing "reject", it'll be pointers
     * into the ACCEPT array.  Otherwise it's actual accepting numbers.
     * In either case, we just dump the numbers.
     */

    /* "lastdfa + 2" is the size of ALIST; includes room for C arrays
     * beginning at 0 and for "jam" state
     */
    k = lastdfa + 2;

    if ( reject )
	/* we put a "cap" on the table associating lists of accepting
	 * numbers with state numbers.  This is needed because we tell
	 * where the end of an accepting list is by looking at where
	 * the list for the next state starts.
	 */
	++k;

#ifdef UNSIGNED_CHAR
    printf( C_short_decl, ALIST, k );
#else
    printf( ((reject && numas > 126) || accnum > 127) ?
	    C_short_decl : C_char_decl, ALIST, k );
#endif

    for ( i = 1; i <= lastdfa; ++i )
	{
	mkdata( acc_array[i] );

	if ( ! reject && trace && acc_array[i] )
	    fprintf( stderr, "state # %d accepts: [%d]\n", i, acc_array[i] );
	}

    /* add entry for "jam" state */
    mkdata( acc_array[i] );

    if ( reject )
	/* add "cap" for the list */
	mkdata( acc_array[i] );

    dataend();

    if ( useecs )
	genecs();

    if ( usemecs )
	{
	/* write out meta-equivalence classes (used to index templates with) */

	if ( trace )
	    fputs( "\n\nMeta-Equivalence Classes:\n", stderr );

	printf( C_char_decl, MATCHARRAY, numecs + 1 );

	for ( i = 1; i <= numecs; ++i )
	    {
	    if ( trace )
		fprintf( stderr, "%d = %d\n", i, abs( tecbck[i] ) );

	    mkdata( abs( tecbck[i] ) );
	    }

	dataend();
	}

    total_states = lastdfa + numtemps;

    printf( tblend > MAX_SHORT ? C_long_decl : C_short_decl,
	    BASEARRAY, total_states + 1 );

    for ( i = 1; i <= lastdfa; ++i )
	{
	register int d = def[i];

	if ( base[i] == JAMSTATE )
	    base[i] = jambase;

	if ( d == JAMSTATE )
	    def[i] = jamstate;

	else if ( d < 0 )
	    {
	    /* template reference */
	    ++tmpuses;
	    def[i] = lastdfa - d + 1;
	    }

	mkdata( base[i] );
	}

    /* generate jam state's base index */
    mkdata( base[i] );

    for ( ++i /* skip jam state */; i <= total_states; ++i )
	{
	mkdata( base[i] );
	def[i] = jamstate;
	}

    dataend();

    printf( tblend > MAX_SHORT ? C_long_decl : C_short_decl,
	    DEFARRAY, total_states + 1 );

    for ( i = 1; i <= total_states; ++i )
	mkdata( def[i] );

    dataend();

    printf( lastdfa > MAX_SHORT ? C_long_decl : C_short_decl,
	    NEXTARRAY, tblend + 1 );

    for ( i = 1; i <= tblend; ++i )
	{
	if ( nxt[i] == 0 || chk[i] == 0 )
	    nxt[i] = jamstate;	/* new state is the JAM state */

	mkdata( nxt[i] );
	}

    dataend();

    printf( lastdfa > MAX_SHORT ? C_long_decl : C_short_decl,
	    CHECKARRAY, tblend + 1 );

    for ( i = 1; i <= tblend; ++i )
	{
	if ( chk[i] == 0 )
	    ++nummt;

	mkdata( chk[i] );
	}

    dataend();
    }


/* generate equivalence-class tables */

genecs()

    {
    register int i, j;
    static char C_char_decl[] = "static char %c[%d] =\n    {   0,\n";
    int numrows;
    char clower();

    printf( C_char_decl, ECARRAY, CSIZE + 1 );

    for ( i = 1; i <= CSIZE; ++i )
	{
	if ( caseins && (i >= 'A') && (i <= 'Z') )
	    ecgroup[i] = ecgroup[clower( i )];

	ecgroup[i] = abs( ecgroup[i] );
	mkdata( ecgroup[i] );
	}

    dataend();

    if ( trace )
	{
	fputs( "\n\nEquivalence Classes:\n\n", stderr );

	numrows = (CSIZE + 1) / 8;

	for ( j = 1; j <= numrows; ++j )
	    {
	    for ( i = j; i <= CSIZE; i = i + numrows )
		{
		char *readable_form();

		fprintf( stderr, "%4s = %-2d",
			 readable_form( i ), ecgroup[i] );

		putc( ' ', stderr );
		}

	    putc( '\n', stderr );
	    }
	}
    }


/* generate the code to find the next state */

gen_next_state()

    {
    char *char_map = useecs ? "e[*yy_cp]" : "*yy_cp";
    
    if ( fulltbl )
	{
	printf( "while ( (yy_current_state = n[yy_current_state][%s]) > 0 )\n",
		char_map );

	puts( "\t{" );

	if ( num_backtracking > 0 )
	    {
	    puts( "\tif ( l[yy_current_state] )" );
	    puts( "\t\t{" );
	    puts( "\t\tyy_last_accepting_state = yy_current_state;" );
	    puts( "\t\tyy_last_accepting_cpos = yy_cp;" );
	    puts( "\t\t}" );
	    }

	puts( "yy_cp++;" );
	puts( "\t}" );

	puts( "yy_current_state = -yy_current_state;" );
	}

    else if ( fullspd )
	{
	puts( "{" );
	puts( "register char yy_c;" );
	printf( "\nfor ( yy_c = %s;\n", char_map );
	puts( "      (yy_trans_info = &yy_current_state[yy_c])->v == yy_c;" );
	printf( "      yy_c = %s )\n", char_map );

	puts( "\t{" );

	puts( "\tyy_current_state += yy_trans_info->n;" );

	if ( num_backtracking > 0 )
	    {
	    puts( "\tif ( yy_current_state[-1].n )" );
	    puts( "\t\t{" );
	    puts( "yy_last_accepting_state = yy_current_state;" );
	    puts( "yy_last_accepting_cpos = yy_c_buf_p;" );
	    puts( "\t\t}" );
	    }

	puts( "\t}" );
	puts( "}" );
	}

    else
	{ /* compressed */
	puts( "do" );

	puts( "\t{" );
	printf( "\tregister char yy_c = %s;\n", char_map );
	puts(
	"\twhile ( c[b[yy_current_state] + yy_sym] != yy_current_state )" );
	puts( "\t\t{" );
	puts( "yy_current_state = d[yy_current_state];" );

	if ( usemecs )
	    {
	    /* we've arrange it so that templates are never chained
             * to one another.  This means we can afford make a
             * very simple test to see if we need to convert to
             * yy_c's meta-equivalence class without worrying
             * about erroneously looking up the meta-equivalence
             * class twice
             */
	    puts( "\t\tif ( yy_current_state >= YY_TEMPLATE )" );
	    puts( "\t\t\tyy_c = m[yy_c];" );
	    }

	puts( "\t\t}" );

	puts( "\tyy_current_state = n[b[yy_current_state] + yy_c];" );

	if ( reject )
	    puts( "\t*yy_state_ptr++ = yy_current_state;" );

	puts( "\t}" );

	if ( interactive )
	    puts( "while ( b[yy_current_state] != YY_JAM_BASE );" );
	else
	    puts( "while ( yy_current_state != YY_JAM );" );
	}
    }


/* make_tables - generate transition tables
 *
 * synopsis
 *     make_tables();
 *
 * Generates transition tables and finishes generating output file
 */

make_tables()

    {
    if ( fullspd )
	{ /* need to define YY_TRANS_OFFSET_TYPE as a size large
	   * enough to hold the biggest offset
	   */
	int total_table_size = tblend + numecs + 1;

	printf( "#define YY_TRANS_OFFSET_TYPE %s\n",
		total_table_size > MAX_SHORT ? "long" : "short" );
	}

    skelout();
    
    if ( fullspd || fulltbl )
	{
	if ( num_backtracking > 0 )
	    {
	    printf( "#define FLEX_USES_BACKTRACKING\n" );
	    printf( "#define YY_BACK_TRACK %d\n", accnum + 1 );
	    }

	if ( fullspd )
	    genctbl();
	else
	    genftbl();
	}

    else
	gentabs();

    skelout();

    (void) fclose( temp_action_file );
    temp_action_file = fopen( action_file_name, "r" );

    /* copy prolog from action_file to output file */
    action_out();

    skelout();

    /* copy actions from action_file to output file */
    action_out();

    skelout();

    /* generate code to find next state */
    gen_next_state();

    skelout();

    /* copy remainder of input to output */

    line_directive_out( stdout );
    (void) flexscan(); /* copy remainder of input to output */
    }
