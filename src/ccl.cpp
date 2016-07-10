/* ccl - routines for character classes */

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

#include "ccl.h"

#include "flexdef.h"

#include "misc.h"

/* return true if the chr is in the ccl. Takes negation into account. */
bool ccl_contains(const int cclp, const int ch)
{
    auto &t = ccls[cclp].table;
    auto p = t.find(ch);
    if (p != t.end())
        return !ccls[cclp].ng;
    return ccls[cclp].ng;
}

/* ccladd - add a single character to a ccl */
void ccladd(int cclp, int ch)
{
    check_char(ch);

    auto &ccl = ccls[cclp];
    auto &t = ccl.table;

    /* check to see if the character is already in the ccl */
    if (t.find(ch) != t.end())
        return;

    /* mark newlines */
    if (ch == nlch)
        ccl.has_nl = true;

    t.insert(ch);
}

/* dump_cclp - same thing as list_character_set, but for cclps.  */
void dump_cclp(FILE *file, int cclp)
{
    int i;

    putc('[', file);

    for (i = 0; i < csize; ++i)
    {
        if (ccl_contains(cclp, i))
        {
            int start_char = i;

            putc(' ', file);

            fputs(readable_form(i), file);

            while (++i < csize && ccl_contains(cclp, i))
                ;

            if (i - 1 > start_char)
                /* this was a run */
                fprintf(file, "-%s",
                        readable_form(i - 1));

            putc(' ', file);
        }
    }

    putc(']', file);
}

/* ccl_set_diff - create a new ccl as the set difference of the two given ccls. */
int ccl_set_diff(int a, int b)
{
    int d, ch;

    /* create new class  */
    d = cclinit();

    /* In order to handle negation, we spin through all possible chars,
     * addding each char in a that is not in b.
     * (This could be O(n^2), but n is small and bounded.)
     */
    for (ch = 0; ch < csize; ++ch)
        if (ccl_contains(a, ch) && !ccl_contains(b, ch))
            ccladd(d, ch);

    /* debug */
    if (0)
    {
        fprintf(stderr, "ccl_set_diff (");
        fprintf(stderr, "\n    ");
        dump_cclp(stderr, a);
        fprintf(stderr, "\n    ");
        dump_cclp(stderr, b);
        fprintf(stderr, "\n    ");
        dump_cclp(stderr, d);
        fprintf(stderr, "\n)\n");
    }
    return d;
}

/* ccl_set_union - create a new ccl as the set union of the two given ccls. */
int ccl_set_union(int a, int b)
{
    int d;

    /* create new class  */
    d = cclinit();

    /* Add all of a */
    ccls[d].table.insert(ccls[a].table.begin(), ccls[a].table.end());

    /* Add all of b */
    ccls[d].table.insert(ccls[b].table.begin(), ccls[b].table.end());

    /* debug */
    if (0)
    {
        fprintf(stderr, "ccl_set_union (%d + %d = %d", a, b, d);
        fprintf(stderr, "\n    ");
        dump_cclp(stderr, a);
        fprintf(stderr, "\n    ");
        dump_cclp(stderr, b);
        fprintf(stderr, "\n    ");
        dump_cclp(stderr, d);
        fprintf(stderr, "\n)\n");
    }
    return d;
}

/* cclinit - return an empty ccl */
int cclinit(void)
{
    ccls.emplace_back();
    return ccls.size() - 1;
}

/* cclnegate - negate the given ccl */
void cclnegate(int cclp)
{
    ccls[cclp].ng = 1;
    ccls[cclp].has_nl = !ccls[cclp].has_nl;
}

/* list_character_set - list the members of a set of characters in CCL form
 *
 * Writes to the given file a character-class representation of those
 * characters present in the given CCL.  A character is present if it
 * has a non-zero value in the cset array.
 */
void list_character_set(FILE *file, int cset[])
{
    int i;

    putc('[', file);

    for (i = 0; i < csize; ++i)
    {
        if (cset[i])
        {
            int start_char = i;

            putc(' ', file);

            fputs(readable_form(i), file);

            while (++i < csize && cset[i])
                ;

            if (i - 1 > start_char)
                /* this was a run */
                fprintf(file, "-%s", readable_form(i - 1));

            putc(' ', file);
        }
    }

    putc(']', file);
}

/** Determines if the range [c1-c2] is unambiguous in a case-insensitive
 * scanner.  Specifically, if a lowercase or uppercase character, x, is in the
 * range [c1-c2], then we require that UPPERCASE(x) and LOWERCASE(x) must also
 * be in the range. If not, then this range is ambiguous, and the function
 * returns false.  For example, [@-_] spans [a-z] but not [A-Z].  Beware that
 * [a-z] will be labeled ambiguous because it does not include [A-Z].
 *
 * @param c1 the lower end of the range
 * @param c2 the upper end of the range
 * @return true if [c1-c2] is not ambiguous for a caseless scanner.
 */
bool range_covers_case(int c1, int c2)
{
    int i, o;

    for (i = c1; i <= c2; i++)
    {
        if (has_case(i))
        {
            o = reverse_case(i);
            if (o < c1 || c2 < o)
                return false;
        }
    }
    return true;
}

/** Reverse the case of a character, if possible.
 * @return c if case-reversal does not apply.
 */
int reverse_case(int c)
{
    return isupper(c) ? tolower(c) : (islower(c) ? toupper(c) : c);
}

/** Return true if c is uppercase or lowercase. */
bool has_case(int c)
{
    return (isupper(c) || islower(c)) ? true : false;
}
