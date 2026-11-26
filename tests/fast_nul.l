/*
 * This file is part of flex.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * Neither the name of the University nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE.
 */

%{
/* A NUL in the input has the equivalence class of the end-of-buffer
   character, so a full-speed scanner takes the end-of-buffer transition
   when it reads one, and then indexes the end-of-buffer state with the
   class of the character after it.  That state used to be the last entry
   of yy_transition, so the scanner read past the end of the table. */
#include <stdio.h>
#include <stdlib.h>
#include "config.h"

static int as, nuls, others, bad_length;
%}

%option fast nomain nodefault noyywrap noinput nounput

%%

"a"     { as++; if (yyleng != 1) bad_length++; }
\0      { nuls++; if (yyleng != 1) bad_length++; }
.|\n    { others++; if (yyleng != 1) bad_length++; }

%%

int main(void);

/* The entry a start state points at is its end-of-buffer transition, so
   adding that offset gives the end-of-buffer state, and the entry in
   front of that one holds numecs + 1.  The scanner indexes the state with
   every equivalence class, so the table needs that many entries after it. */
static int
room_after_end_of_buffer_state (void)
{
    const struct yy_trans_info *state = yy_start_state_list[0];
    const struct yy_trans_info *end =
        yy_transition + sizeof yy_transition / sizeof *yy_transition;
    int numecs;

    if (state->yy_verify != 0)
        return -1;
    state += state->yy_nxt;
    numecs = state[-1].yy_verify - 1;
    return end - (state + numecs) > 0;
}

int
main (void)
{
    static const char input[] = { 'a', '\0', 'b', 'a', '\0' };
    yybuffer b;
    int room = room_after_end_of_buffer_state();

    b = yy_scan_bytes(input, sizeof input);
    yylex();
    yy_delete_buffer(b);
    if (room != 1) {
        printf("TEST FAILED: no room after the end-of-buffer state (%d).\n",
               room);
        return 1;
    }
    if (as != 2 || nuls != 2 || others != 1 || bad_length != 0) {
        printf("TEST FAILED: %d a, %d NUL, %d others and %d bad lengths, "
               "not 2, 2, 1 and 0.\n", as, nuls, others, bad_length);
        return 1;
    }
    printf("TEST RETURNING OK.\n");
    return 0;
}
