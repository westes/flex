/* flexecs - equivalence class routines */

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

/* ccl2ecl - convert character classes to set of equivalence classes
 *
 * synopsis
 *    ccl2ecl();
 */

ccl2ecl()

    {
    int i, ich, newlen, cclp, ccls, cclmec;

    for ( i = 1; i <= lastccl; ++i )
	{
	/* we loop through each character class, and for each character
	 * in the class, add the character's equivalence class to the
	 * new "character" class we are creating.  Thus when we are all
	 * done, character classes will really consist of collections
	 * of equivalence classes
	 */

	newlen = 0;
	cclp = cclmap[i];

	for ( ccls = 0; ccls < ccllen[i]; ++ccls )
	    {
	    ich = ccltbl[cclp + ccls];
	    cclmec = ecgroup[ich];
	    if ( cclmec > 0 )
		{
		ccltbl[cclp + newlen] = cclmec;
		++newlen;
		}
	    }

	ccllen[i] = newlen;
	}
    }


/* cre8ecs - associate equivalence class numbers with class members
 *
 * synopsis
 *    int cre8ecs();
 *    number of classes = cre8ecs( fwd, bck, num );
 *
 *  fwd is the forward linked-list of equivalence class members.  bck
 *  is the backward linked-list, and num is the number of class members.
 *  Returned is the number of classes.
 */

int cre8ecs( fwd, bck, num )
int fwd[], bck[], num;

    {
    int i, j, numcl;

    numcl = 0;

    /* create equivalence class numbers.  From now on, abs( bck(x) )
     * is the equivalence class number for object x.  If bck(x)
     * is positive, then x is the representative of its equivalence
     * class.
     */

    for ( i = 1; i <= num; ++i )
	if ( bck[i] == NIL )
	    {
	    bck[i] = ++numcl;
	    for ( j = fwd[i]; j != NIL; j = fwd[j] )
		bck[j] = -numcl;
	    }

    return ( numcl );
    }


/* mkeccl - update equivalence classes based on character class xtions
 *
 * synopsis
 *    char ccls[];
 *    int lenccl, fwd[llsiz], bck[llsiz], llsiz;
 *    mkeccl( ccls, lenccl, fwd, bck, llsiz );
 *
 * where ccls contains the elements of the character class, lenccl is the
 * number of elements in the ccl, fwd is the forward link-list of equivalent
 * characters, bck is the backward link-list, and llsiz size of the link-list
 */

mkeccl( ccls, lenccl, fwd, bck, llsiz )
char ccls[];
int lenccl, fwd[], bck[], llsiz;

    {
    int cclp, oldec, newec;
    int cclm, i, j;

    /* note that it doesn't matter whether or not the character class is
     * negated.  The same results will be obtained in either case.
     */

    cclp = 0;

    while ( cclp < lenccl )
	{
	cclm = ccls[cclp];
	oldec = bck[cclm];
	newec = cclm;

	j = cclp + 1;

	for ( i = fwd[cclm]; i != NIL && i <= llsiz; i = fwd[i] )
	    { /* look for the symbol in the character class */
	    for ( ; j < lenccl && ccls[j] <= i; ++j )
		if ( ccls[j] == i )
		    {
		    /* we found an old companion of cclm in the ccl.
		     * link it into the new equivalence class and flag it as
		     * having been processed
		     */

		    bck[i] = newec;
		    fwd[newec] = i;
		    newec = i;
		    ccls[j] = -i;	/* set flag so we don't reprocess */

		    /* get next equivalence class member */
		    /* next 2 */ goto next_pt;
		    }

	    /* symbol isn't in character class.  Put it in the old equivalence
	     * class
	     */

	    bck[i] = oldec;

	    if ( oldec != NIL )
		fwd[oldec] = i;

	    oldec = i;
next_pt:
	    ;
	    }

	if ( bck[cclm] != NIL || oldec != bck[cclm] )
	    {
	    bck[cclm] = NIL;
	    fwd[oldec] = NIL;
	    }

	fwd[newec] = NIL;

	/* find next ccl member to process */

	for ( ++cclp; ccls[cclp] < 0 && cclp < lenccl; ++cclp )
	    {
	    /* reset "doesn't need processing" flag */
	    ccls[cclp] = -ccls[cclp];
	    }
	}
    }


/* mkechar - create equivalence class for single character
 *
 * synopsis
 *    int tch, fwd[], bck[];
 *    mkechar( tch, fwd, bck );
 */

mkechar( tch, fwd, bck )
int tch, fwd[], bck[];

    {
    /* if until now the character has been a proper subset of
     * an equivalence class, break it away to create a new ec
     */

    if ( fwd[tch] != NIL )
	bck[fwd[tch]] = bck[tch];

    if ( bck[tch] != NIL )
	fwd[bck[tch]] = fwd[tch];

    fwd[tch] = NIL;
    bck[tch] = NIL;
    }
