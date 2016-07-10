/*  tblcmp.h - table compression routines
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

/* Build table entries for dfa state. */
void bldtbl(int[], int, int, int, int);

void cmptmps();        /* compress template table entries */

/* Finds a space in the table for a state to be placed. */
int find_table_space(int *, int);
void inittbl(); /* initialize transition tables */

/* Make the default, "jam" table entries. */
void mkdeftbl();

/* Create table entries for a state (or state fragment) which has
* only one out-transition.
*/
void mk1tbl(int, int, int, int);

/* Place a state into full speed transition table. */
void place_state(int *, int, int);

/* Save states with only one out-transition to be processed later. */
void stack1(int, int, int, int);
