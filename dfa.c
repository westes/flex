/* dfa - DFA construction routines */

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

/* epsclosure - construct the epsilon closure of a set of ndfa states
 *
 * synopsis
 *    int t[current_max_dfa_size], numstates, accset[accnum + 1], nacc;
 *    int hashval;
 *    int *epsclosure();
 *    t = epsclosure( t, &numstates, accset, &nacc, &hashval );
 *
 * NOTES
 *    the epsilon closure is the set of all states reachable by an arbitrary
 *  number of epsilon transitions which themselves do not have epsilon
 *  transitions going out, unioned with the set of states which have non-null
 *  accepting numbers.  t is an array of size numstates of nfa state numbers.
 *  Upon return, t holds the epsilon closure and numstates is updated.  accset
 *  holds a list of the accepting numbers, and the size of accset is given
 *  by nacc.  t may be subjected to reallocation if it is not large enough
 *  to hold the epsilon closure.
 *
 *    hashval is the hash value for the dfa corresponding to the state set
 */

int *epsclosure( t, ns_addr, accset, nacc_addr, hv_addr )
int *t, *ns_addr, accset[], *nacc_addr, *hv_addr;

    {
    register int stkpos, ns, tsp;
    int numstates = *ns_addr, nacc, hashval, transsym, nfaccnum;
    int stkend, nstate;
    static int did_stk_init = false, *stk; 

#define MARK_STATE(state) \
	trans1[state] = trans1[state] - MARKER_DIFFERENCE;

#define IS_MARKED(state) (trans1[state] < 0)

#define UNMARK_STATE(state) \
	trans1[state] = trans1[state] + MARKER_DIFFERENCE;

#define CHECK_ACCEPT(state) \
	{ \
	nfaccnum = accptnum[state]; \
	if ( nfaccnum != NIL ) \
	    accset[++nacc] = nfaccnum; \
	}

#define DO_REALLOCATION \
	{ \
	current_max_dfa_size += MAX_DFA_SIZE_INCREMENT; \
	++num_reallocs; \
	t = reallocate_integer_array( t, current_max_dfa_size ); \
	stk = reallocate_integer_array( stk, current_max_dfa_size ); \
	} \

#define PUT_ON_STACK(state) \
	{ \
	if ( ++stkend >= current_max_dfa_size ) \
	    DO_REALLOCATION \
	stk[stkend] = state; \
	MARK_STATE(state) \
	}

#define ADD_STATE(state) \
	{ \
	if ( ++numstates >= current_max_dfa_size ) \
	    DO_REALLOCATION \
	t[numstates] = state; \
	hashval = hashval + state; \
	}

#define STACK_STATE(state) \
	{ \
	PUT_ON_STACK(state) \
	CHECK_ACCEPT(state) \
	if ( nfaccnum != NIL || transchar[state] != SYM_EPSILON ) \
	    ADD_STATE(state) \
	}

    if ( ! did_stk_init )
	{
	stk = allocate_integer_array( current_max_dfa_size );
	did_stk_init = true;
	}

    nacc = stkend = hashval = 0;

    for ( nstate = 1; nstate <= numstates; ++nstate )
	{
	ns = t[nstate];

	/* the state could be marked if we've already pushed it onto
	 * the stack
	 */
	if ( ! IS_MARKED(ns) )
	    PUT_ON_STACK(ns)

	CHECK_ACCEPT(ns)
	hashval = hashval + ns;
	}

    for ( stkpos = 1; stkpos <= stkend; ++stkpos )
	{
	ns = stk[stkpos];
	transsym = transchar[ns];

	if ( transsym == SYM_EPSILON )
	    {
	    tsp = trans1[ns] + MARKER_DIFFERENCE;

	    if ( tsp != NO_TRANSITION )
		{
		if ( ! IS_MARKED(tsp) )
		    STACK_STATE(tsp)

		tsp = trans2[ns];

		if ( tsp != NO_TRANSITION )
		    if ( ! IS_MARKED(tsp) )
			STACK_STATE(tsp)
		}
	    }
	}

    /* clear out "visit" markers */

    for ( stkpos = 1; stkpos <= stkend; ++stkpos )
	{
	if ( IS_MARKED(stk[stkpos]) )
	    {
	    UNMARK_STATE(stk[stkpos])
	    }
	else
	    flexfatal( "consistency check failed in epsclosure()" );
	}

    *ns_addr = numstates;
    *hv_addr = hashval;
    *nacc_addr = nacc;

    return ( t );
    }



/* increase_max_dfas - increase the maximum number of DFAs */

increase_max_dfas()

    {
    int old_max = current_max_dfas;

    current_max_dfas += MAX_DFAS_INCREMENT;

    ++num_reallocs;

    base = reallocate_integer_array( base, current_max_dfas );
    def = reallocate_integer_array( def, current_max_dfas );
    dfasiz = reallocate_integer_array( dfasiz, current_max_dfas );
    accsiz = reallocate_integer_array( accsiz, current_max_dfas );
    dhash = reallocate_integer_array( dhash, current_max_dfas );
    todo = reallocate_integer_array( todo, current_max_dfas );
    dss = reallocate_integer_pointer_array( dss, current_max_dfas );
    dfaacc = reallocate_dfaacc_union( dfaacc, current_max_dfas );

    /* fix up todo queue */
    if ( todo_next < todo_head )
	{ /* queue was wrapped around the end */
	register int i;

	for ( i = 0; i < todo_next; ++i )
	    todo[old_max + i] = todo[i];
	
	todo_next += old_max;
	}
    }


/* snstods - converts a set of ndfa states into a dfa state
 *
 * synopsis
 *    int sns[numstates], numstates, newds, accset[accnum + 1], nacc, hashval;
 *    int snstods();
 *    is_new_state = snstods( sns, numstates, accset, nacc, hashval, &newds );
 *
 * on return, the dfa state number is in newds.
 */

int snstods( sns, numstates, accset, nacc, hashval, newds_addr )
int sns[], numstates, accset[], nacc, hashval, *newds_addr;

    {
    int didsort = 0;
    register int i, j;
    int newds, *oldsns;
    char *malloc();

    for ( i = 1; i <= lastdfa; ++i )
	if ( hashval == dhash[i] )
	    {
	    if ( numstates == dfasiz[i] )
		{
		oldsns = dss[i];

		if ( ! didsort )
		    {
		    /* we sort the states in sns so we can compare it to
		     * oldsns quickly.  we use bubble because there probably
		     * aren't very many states
		     */
		    bubble( sns, numstates );
		    didsort = 1;
		    }

		for ( j = 1; j <= numstates; ++j )
		    if ( sns[j] != oldsns[j] )
			break;

		if ( j > numstates )
		    {
		    ++dfaeql;
		    *newds_addr = i;
		    return ( 0 );
		    }

		++hshcol;
		}

	    else
		++hshsave;
	    }

    /* make a new dfa */

    if ( ++lastdfa >= current_max_dfas )
	increase_max_dfas();

    newds = lastdfa;

    if ( ! (dss[newds] = (int *) malloc( (unsigned) ((numstates + 1) * sizeof( int )) )) )
	flexfatal( "dynamic memory failure in snstods()" );

    /* if we haven't already sorted the states in sns, we do so now, so that
     * future comparisons with it can be made quickly
     */

    if ( ! didsort )
	bubble( sns, numstates );

    for ( i = 1; i <= numstates; ++i )
	dss[newds][i] = sns[i];

    dfasiz[newds] = numstates;
    dhash[newds] = hashval;

    if ( nacc == 0 )
	{
	dfaacc[newds].dfaacc_state = 0;
	accsiz[newds] = 0;
	}

    else if ( reject )
	{
	/* we sort the accepting set in increasing order so the disambiguating
	 * rule that the first rule listed is considered match in the event of
	 * ties will work.  We use a bubble sort since the list is probably
	 * quite small.
	 */

	bubble( accset, nacc );

	dfaacc[newds].dfaacc_state =
	    (int) malloc( (unsigned) ((nacc + 1) * sizeof( int )) );

	if ( ! dfaacc[newds].dfaacc_state )
	    flexfatal( "dynamic memory failure in snstods()" );

	/* save the accepting set for later */
	for ( i = 1; i <= nacc; ++i )
	    dfaacc[newds].dfaacc_set[i] = accset[i];

	accsiz[newds] = nacc;
	}

    else
	{ /* find lowest numbered rule so the disambiguating rule will work */
	j = accnum + 1;

	for ( i = 1; i <= nacc; ++i )
	    if ( accset[i] < j )
		j = accset[i];

	dfaacc[newds].dfaacc_state = j;
	}

    *newds_addr = newds;

    return ( 1 );
    }


/* symfollowset - follow the symbol transitions one step
 *
 * synopsis
 *    int ds[current_max_dfa_size], dsize, transsym;
 *    int nset[current_max_dfa_size], numstates;
 *    numstates = symfollowset( ds, dsize, transsym, nset );
 */

int symfollowset( ds, dsize, transsym, nset )
int ds[], dsize, transsym, nset[];

    {
    int ns, tsp, sym, i, j, lenccl, ch, numstates;
    int ccllist;

    numstates = 0;

    for ( i = 1; i <= dsize; ++i )
	{ /* for each nfa state ns in the state set of ds */
	ns = ds[i];
	sym = transchar[ns];
	tsp = trans1[ns];

	if ( sym < 0 )
	    { /* it's a character class */
	    sym = -sym;
	    ccllist = cclmap[sym];
	    lenccl = ccllen[sym];

	    if ( cclng[sym] )
		{
		for ( j = 0; j < lenccl; ++j )
		    { /* loop through negated character class */
		    ch = ccltbl[ccllist + j];

		    if ( ch > transsym )
			break;	/* transsym isn't in negated ccl */

		    else if ( ch == transsym )
			/* next 2 */ goto bottom;
		    }

		/* didn't find transsym in ccl */
		nset[++numstates] = tsp;
		}

	    else
		for ( j = 0; j < lenccl; ++j )
		    {
		    ch = ccltbl[ccllist + j];

		    if ( ch > transsym )
			break;

		    else if ( ch == transsym )
			{
			nset[++numstates] = tsp;
			break;
			}
		    }
	    }

	else if ( sym >= 'A' && sym <= 'Z' && caseins )
	    flexfatal( "consistency check failed in symfollowset" );

	else if ( sym == SYM_EPSILON )
	    { /* do nothing */
	    }

	else if ( ecgroup[sym] == transsym )
	    nset[++numstates] = tsp;

bottom:
	;
	}

    return ( numstates );
    }


/* sympartition - partition characters with same out-transitions
 *
 * synopsis
 *    integer ds[current_max_dfa_size], numstates, duplist[numecs];
 *    symlist[numecs];
 *    sympartition( ds, numstates, symlist, duplist );
 */

sympartition( ds, numstates, symlist, duplist )
int ds[], numstates, duplist[];
int symlist[];

    {
    int tch, i, j, k, ns, dupfwd[CSIZE + 1], lenccl, cclp, ich;

    /* partitioning is done by creating equivalence classes for those
     * characters which have out-transitions from the given state.  Thus
     * we are really creating equivalence classes of equivalence classes.
     */

    for ( i = 1; i <= numecs; ++i )
	{ /* initialize equivalence class list */
	duplist[i] = i - 1;
	dupfwd[i] = i + 1;
	}

    duplist[1] = NIL;
    dupfwd[numecs] = NIL;

    for ( i = 1; i <= numstates; ++i )
	{
	ns = ds[i];
	tch = transchar[ns];

	if ( tch != SYM_EPSILON )
	    {
	    if ( tch < -lastccl || tch > CSIZE )
		flexfatal( "bad transition character detected in sympartition()" );

	    if ( tch > 0 )
		{ /* character transition */
		mkechar( ecgroup[tch], dupfwd, duplist );
		symlist[ecgroup[tch]] = 1;
		}

	    else
		{ /* character class */
		tch = -tch;

		lenccl = ccllen[tch];
		cclp = cclmap[tch];
		mkeccl( ccltbl + cclp, lenccl, dupfwd, duplist, numecs );

		if ( cclng[tch] )
		    {
		    j = 0;

		    for ( k = 0; k < lenccl; ++k )
			{
			ich = ccltbl[cclp + k];

			for ( ++j; j < ich; ++j )
			    symlist[j] = 1;
			}

		    for ( ++j; j <= numecs; ++j )
			symlist[j] = 1;
		    }

		else
		    for ( k = 0; k < lenccl; ++k )
			{
			ich = ccltbl[cclp + k];
			symlist[ich] = 1;
			}
		}
	    }
	}
    }
