/* scanflags - flags used by scanning. */

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

//scanflags_t* _sf_stk = NULL;
//size_t _sf_top_ix=0, _sf_max=0;

void
sf_push (FlexState* gv)
{
    if (gv->_sf_top_ix + 1 >= gv->_sf_max) {
        gv->_sf_max += 32;
        gv->_sf_stk = realloc(gv->_sf_stk, sizeof(scanflags_t) * gv->_sf_max);
    }

    // copy the top element
    gv->_sf_stk[gv->_sf_top_ix + 1] = gv->_sf_stk[gv->_sf_top_ix];
    ++gv->_sf_top_ix;
}

void
sf_pop (FlexState* gv)
{
    assert(gv->_sf_top_ix > 0);
    --gv->_sf_top_ix;
}

/* one-time initialization. Should be called before any sf_ functions. */
void
sf_init (FlexState* gv)
{
    assert(gv->_sf_stk == NULL);
    gv->_sf_max = 32;
    gv->_sf_stk = malloc(sizeof(scanflags_t) * gv->_sf_max);
    if (!gv->_sf_stk)
        lerr_fatal(gv, _("Unable to allocate %zu of stack"), sizeof(scanflags_t));
    gv->_sf_stk[gv->_sf_top_ix] = 0;
}

/* vim:set expandtab cindent tabstop=4 softtabstop=4 shiftwidth=4 textwidth=0: */
