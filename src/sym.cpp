/* sym - symbol table routines */

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

#include <unordered_map>

#include "sym.h"

#include "flexdef.h"

#include "misc.h"
#include "nfa.h"

/* Variables for symbol tables:
 * sctbl - start-condition symbol table
 * ccltab - character class text symbol table
 */

std::unordered_map<String, int> sctbl;

/* scinstal - make a start condition
 *
 * NOTE
 *    The start condition is "exclusive" if xcluflg is true.
 */
void scinstal(const String& str, int xcluflg)
{
    StartCondition sc;
    sc.name = str;

    auto i = sctbl.find(str);
    if (i != sctbl.end())
        format_pinpoint_message(_("start condition %s declared twice"), str.c_str());
    sctbl[sc.name] = start_conditions.size();

    sc.set = mkstate(SYM_EPSILON);
    sc.bol = mkstate(SYM_EPSILON);
    sc.xclu = xcluflg;
    sc.eof = false;

    start_conditions.push_back(sc);
}

/* sclookup - lookup the number associated with a start condition
 *
 * Returns 0 if no such start condition.
 */
int sclookup(const String &str)
{
    auto i = sctbl.find(str);
    if (i == sctbl.end())
        return 0;
    return i->second;
}
