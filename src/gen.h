/*  gen.h - actual generation (writing) of flex scanners
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

/* Generate the code to keep backing-up information. */
void gen_backing_up(void);

/* Generate the code to perform the backing up. */
void gen_bu_action(void);

/* Generate full speed compressed transition table. */
void genctbl(void);

/* Generate the code to find the action number. */
void gen_find_action(void);

void genftbl(void); /* generate full transition table */

/* Generate the code to find the next compressed-table state. */
void gen_next_compressed_state(char *);

/* Generate the code to find the next match. */
void gen_next_match(void);

/* Generate the code to find the next state. */
void gen_next_state(int);

/* Generate the code to make a NUL transition. */
void gen_NUL_trans(void);

/* Generate the code to find the start state. */
void gen_start_state(void);

/* Generate data statements for the transition tables. */
void gentabs(void);

void make_tables(void); /* generate transition tables */
