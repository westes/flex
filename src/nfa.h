/*  nfa.h - NFA construction routines
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

/* Add an accepting state to a machine. */
void add_accept(int, int);

/* Make a given number of copies of a singleton machine. */
int copysingl(int, int);

/* Debugging routine to write out an nfa. */
void dumpnfa(int);

/* Finish up the processing for a rule. */
void finish_rule(int, int, int, int, int);

/* Connect two machines together. */
int link_machines(int, int);

/* Mark each "beginning" state in a machine as being a "normal" (i.e.,
* not trailing context associated) state.
*/
void mark_beginning_as_normal(int);

/* Make a machine that branches to two machines. */
int mkbranch(int, int);

int mkclos(int); /* convert a machine into a closure */
int mkopt(int);  /* make a machine optional */

/* Make a machine that matches either one of two machines. */
int mkor(int, int);

/* Convert a machine into a positive closure. */
int mkposcl(int);

int mkrep(int, int, int); /* make a replicated machine */

/* Create a state with a transition on a given symbol. */
int mkstate(int);

void new_rule(void); /* initialize for a new rule */
