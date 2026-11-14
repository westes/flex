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
/* Test that the hook macros and calls a flex 2.6.4 program uses still work:
   YY_INPUT, YY_USER_ACTION, YY_USER_INIT, YY_FATAL_ERROR, the start
   condition stack, YY_START, yymore, yyless, REJECT, input and yy_set_bol.
   The program defines each hook the way 2.6.4 expects. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

static const char *text = "aab\nbcab\nc\n";
static int pos;
static int nact;
static int hits;

#define YY_INPUT(buf,result,max_size) \
    do { \
        if (text[pos] == '\0') { (result) = YY_NULL; } \
        else { (buf)[0] = text[pos++]; (result) = 1; } \
    } while (0)
#define YY_USER_ACTION nact++;
#define YY_USER_INIT   nact = 0;
#define YY_FATAL_ERROR(msg) do { printf("fatal: %s\n", (msg)); exit(1); } while (0)
%}

%option 8bit nomain noyywrap nounput stack
%x X

%%

aa      { yy_push_state(X); hits++; }
<X>b    { if (YY_START == X && yy_top_state() == INITIAL) hits++; yy_pop_state(); }
bc      { hits++; REJECT; }
ca      { yyless(1); hits++; }
c       { yy_set_bol(1); hits++; }
a       { yymore(); }
<<EOF>> { yy_delete_buffer(YY_CURRENT_BUFFER); return 0; }
.|\n    { }

%%

int main(void);

int
main (void)
{
    yylex();
    if (hits != 5 || nact == 0) {
        printf("TEST FAILED: hits %d, actions %d.\n", hits, nact);
        return 1;
    }
    printf("TEST RETURNING OK.\n");
    return 0;
}
