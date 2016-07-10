/* ecs - equivalence class routines */

/*  Copyright (c) 1990 The Regents of the University of California. */
/*  All rights reserved. */

/*  This code is derived from software contributed to Berkeley by */
/*  Vern Paxson. */

/*  The United States Government has rights in this work pursuant */
/*  to contract no. DE-AC03-76SF00098 between the United States */
/*  Department of Energy and the University of California. */

/* This file is part of flex */

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

#include "ecs.h"

#include "flexdef.h"

/* ccl2ecl - convert character classes to set of equivalence classes */
void ccl2ecl(void)
{
    for (size_t i = 1; i < ccls.size(); ++i)
    {
        /* We loop through each character class, and for each character
		 * in the class, add the character's equivalence class to the
		 * new "character" class we are creating.  Thus when we are all
		 * done, character classes will really consist of collections
		 * of equivalence classes
		 */
        CharacterClass::Table new_table;
        for (auto &c : ccls[i].table)
        {
            int cclmec = ecgroup[c];
            if (cclmec > 0)
                new_table.insert(cclmec);
        }
        ccls[i].table = new_table;
    }
}

/* cre8ecs - associate equivalence class numbers with class members
 *
 * fwd is the forward linked-list of equivalence class members.  bck
 * is the backward linked-list, and num is the number of class members.
 *
 * Returned is the number of classes.
 */
int cre8ecs(int fwd[], int bck[], int num)
{
    int i, j, numcl;

    numcl = 0;

    /* Create equivalence class numbers.  From now on, ABS( bck(x) )
	 * is the equivalence class number for object x.  If bck(x)
	 * is positive, then x is the representative of its equivalence
	 * class.
	 */
    for (i = 1; i <= num; ++i)
        if (bck[i] == NIL)
        {
            bck[i] = ++numcl;
            for (j = fwd[i]; j != NIL; j = fwd[j])
                bck[j] = -numcl;
        }

    return numcl;
}

/* mkeccl - update equivalence classes based on character class xtions
 *
 * synopsis
 *    unsigned char ccls[];
 *    int lenccl, fwd[llsiz], bck[llsiz], llsiz, NUL_mapping;
 *    void mkeccl( unsigned char ccls[], int lenccl, int fwd[llsiz], int bck[llsiz],
 *			int llsiz, int NUL_mapping );
 *
 * ccls contains the elements of the character class, lenccl is the
 * number of elements in the ccl, fwd is the forward link-list of equivalent
 * characters, bck is the backward link-list, and llsiz size of the link-list.
 *
 * NUL_mapping is the value which NUL (0) should be mapped to.
 */
void mkeccl(const CharacterClass::Table &table, int fwd[], int bck[], int llsiz, int NUL_mapping)
{
    static unsigned char cclflags[CSIZE] = { 0 };

    /* Note that it doesn't matter whether or not the character class is
	 * negated.  The same results will be obtained in either case.
	 */
    int it = 0;
    for (auto iter = table.begin(); iter != table.end();)
    {
        auto cclm = *iter;

        if (NUL_mapping && cclm == 0)
            cclm = NUL_mapping;

        int oldec = bck[cclm];
        int newec = cclm;

        auto j = std::next(iter);
        auto jt = it + 1;

        for (size_t i = fwd[cclm]; i != NIL && i <= llsiz; i = fwd[i])
        {
            /* look for the symbol in the character class */
            for (; j != table.end(); ++j, ++jt)
            {
                auto ccl_char = *j;

                if (NUL_mapping && ccl_char == 0)
                    ccl_char = NUL_mapping;

                if (ccl_char > i)
                    break;

                auto d = jt;
                if (ccl_char == i && !cclflags[d])
                {
                    /* We found an old companion of cclm
					 * in the ccl.  Link it into the new
					 * equivalence class and flag it as
					 * having been processed.
					 */
                    bck[i] = newec;
                    fwd[newec] = i;
                    newec = i;
                    /* Set flag so we don't reprocess. */
                    cclflags[d] = 1;

                    /* Get next equivalence class member. */
                    /* continue 2 */
                    goto next_pt;
                }
            }

            /* Symbol isn't in character class.  Put it in the old
			 * equivalence class.
			 */
            bck[i] = oldec;

            if (oldec != NIL)
                fwd[oldec] = i;

            oldec = i;

        next_pt:;
        }

        if (bck[cclm] != NIL || oldec != bck[cclm])
        {
            bck[cclm] = NIL;
            fwd[oldec] = NIL;
        }

        fwd[newec] = NIL;

        /* Find next ccl member to process. */
        for (++iter, ++it; iter != table.end() && cclflags[it]; ++iter, ++it)
        {
            /* Reset "doesn't need processing" flag. */
            cclflags[it] = 0;
        }
        if (iter == table.end())
            break;
    }
}

/* mkechar - create equivalence class for single character */
void mkechar(int tch, int fwd[], int bck[])
{
    /* If until now the character has been a proper subset of
	 * an equivalence class, break it away to create a new ec
	 */
    if (fwd[tch] != NIL)
        bck[fwd[tch]] = bck[tch];

    if (bck[tch] != NIL)
        fwd[bck[tch]] = fwd[tch];

    fwd[tch] = NIL;
    bck[tch] = NIL;
}
