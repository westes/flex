/*  scanflags.h - flags used by scanning
 *
 *  Copyright (c) 1990 The Regents of the University of California.
 *  All rights reserved.
 *
 *  This code is derived from software contributed to Berkeley by
 *  Vern Paxson.
 *
 *  The United States Government has rights in this work pursuant
 *  to contract no. DE-AC03-76SF00098 between the United States
 *  Department of Energy and the University of California.
 *
 *  This file is part of flex.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  Neither the name of the University nor the names of its contributors
 *  may be used to endorse or promote products derived from this software
 *  without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 *  IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE.
 */

#pragma once

#include <stdlib.h>

typedef unsigned int scanflags_t;
extern scanflags_t *_sf_stk;
extern size_t _sf_top_ix, _sf_max; /**< stack of scanner flags. */

#define _SF_CASE_INS ((scanflags_t) 0x0001)
#define _SF_DOT_ALL ((scanflags_t) 0x0002)
#define _SF_SKIP_WS ((scanflags_t) 0x0004)

#define sf_top() (_sf_stk[_sf_top_ix])
#define sf_case_ins() (sf_top() & _SF_CASE_INS)
#define sf_dot_all() (sf_top() & _SF_DOT_ALL)
#define sf_skip_ws() (sf_top() & _SF_SKIP_WS)
#define sf_set_case_ins(X) ((X) ? (sf_top() |= _SF_CASE_INS) : (sf_top() &= ~_SF_CASE_INS))
#define sf_set_dot_all(X) ((X) ? (sf_top() |= _SF_DOT_ALL) : (sf_top() &= ~_SF_DOT_ALL))
#define sf_set_skip_ws(X) ((X) ? (sf_top() |= _SF_SKIP_WS) : (sf_top() &= ~_SF_SKIP_WS))

extern void sf_init(void);
extern void sf_push(void);
extern void sf_pop(void);
