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
/* A batch scanner with compressed tables must back up when it reaches a
   NUL byte that cannot extend the current match. Without the back-up, the
   NUL was swallowed into the previous token, which was one byte too long,
   and the rule for the NUL never ran. */
#include <stdio.h>
#include <stdlib.h>
#include "config.h"

static int as, nuls, others, bad_length;
%}

%option batch nomain noyywrap noinput nounput

%%

"a"     { as++; if (yyleng != 1) bad_length++; }
\0      { nuls++; if (yyleng != 1) bad_length++; }
.|\n    { others++; if (yyleng != 1) bad_length++; }

%%

int main(void);

int
main (void)
{
    static const char input[] = { 'a', '\0', 'b', 'a', '\0' };
    yybuffer b;

    b = yy_scan_bytes(input, sizeof input);
    yylex();
    yy_delete_buffer(b);
    if (as != 2 || nuls != 2 || others != 1 || bad_length != 0) {
        printf("TEST FAILED: %d a, %d NUL, %d others and %d bad lengths, "
               "not 2, 2, 1 and 0.\n", as, nuls, others, bad_length);
        return 1;
    }
    printf("TEST RETURNING OK.\n");
    return 0;
}
