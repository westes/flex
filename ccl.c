/* ccl - routines for character classes */

/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Vern Paxson.
 * 
 * The United States Government has rights in this work pursuant to
 * contract no. DE-AC03-76SF00098 between the United States Department of
 * Energy and the University of California.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint

static char copyright[] =
    "@(#) Copyright (c) 1989 The Regents of the University of California.\n";
static char CR_continuation[] = "@(#) All rights reserved.\n";

static char rcsid[] =
    "@(#) $Header$ (LBL)";

#endif

#include "flexdef.h"

/* ccladd - add a single character to a ccl
 *
 * synopsis
 *    int cclp;
 *    char ch;
 *    ccladd( cclp, ch );
 */

ccladd( cclp, ch )
int cclp;
char ch;

    {
    int ind, len, newpos, i;

    len = ccllen[cclp];
    ind = cclmap[cclp];

    /* check to see if the character is already in the ccl */

    for ( i = 0; i < len; ++i )
	if ( ccltbl[ind + i] == ch )
	    return;

    newpos = ind + len;

    if ( newpos >= current_max_ccl_tbl_size )
	{
	current_max_ccl_tbl_size += MAX_CCL_TBL_SIZE_INCREMENT;

	++num_reallocs;

	ccltbl = reallocate_character_array( ccltbl, current_max_ccl_tbl_size );
	}

    ccllen[cclp] = len + 1;
    ccltbl[newpos] = ch;
    }


/* cclinit - make an empty ccl
 *
 * synopsis
 *    int cclinit();
 *    new_ccl = cclinit();
 */

int cclinit()

    {
    if ( ++lastccl >= current_maxccls )
	{
	current_maxccls += MAX_CCLS_INCREMENT;

	++num_reallocs;

	cclmap = reallocate_integer_array( cclmap, current_maxccls );
	ccllen = reallocate_integer_array( ccllen, current_maxccls );
	cclng = reallocate_integer_array( cclng, current_maxccls );
	}

    if ( lastccl == 1 )
	/* we're making the first ccl */
	cclmap[lastccl] = 0;

    else
	/* the new pointer is just past the end of the last ccl.  Since
	 * the cclmap points to the \first/ character of a ccl, adding the
	 * length of the ccl to the cclmap pointer will produce a cursor
	 * to the first free space
	 */
	cclmap[lastccl] = cclmap[lastccl - 1] + ccllen[lastccl - 1];

    ccllen[lastccl] = 0;
    cclng[lastccl] = 0;	/* ccl's start out life un-negated */

    return ( lastccl );
    }


/* cclnegate - negate a ccl
 *
 * synopsis
 *    int cclp;
 *    cclnegate( ccl );
 */

cclnegate( cclp )
int cclp;

    {
    cclng[cclp] = 1;
    }


/* list_character_set - list the members of a set of characters in CCL form
 *
 * synopsis
 *     int cset[CSIZE + 1];
 *     FILE *file;
 *     list_character_set( cset );
 *
 * writes to the given file a character-class representation of those
 * characters present in the given set.  A character is present if it
 * has a non-zero value in the set array.
 */

list_character_set( file, cset )
FILE *file;
int cset[];

    {
    register int i;
    char *readable_form();

    putc( '[', file );

    for ( i = 1; i <= CSIZE; ++i )
	{
	if ( cset[i] )
	    {
	    register int start_char = i;

	    putc( ' ', file );

	    fputs( readable_form( i ), file );

	    while ( ++i <= CSIZE && cset[i] )
		;

	    if ( i - 1 > start_char )
		/* this was a run */
		fprintf( file, "-%s", readable_form( i - 1 ) );

	    putc( ' ', file );
	    }
	}

    putc( ']', file );
    }
