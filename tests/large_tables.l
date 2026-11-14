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
/* Test that a scanner whose tables are long, but whose states are few,
   gets a table of offsets wide enough for its entries. The rule below
   needs about 24000 states and over 49000 table entries, so the offsets
   no longer fit in 16 bits. The test is compiled with -Werror=overflow,
   which rejects a table of offsets that is too narrow, and it checks
   that the scanner still finds the right matches. */
#include <stdio.h>
#include <stdlib.h>
#include "config.h"

static int found = 0;
static int other = 0;
%}

%option 8bit nomain nodefault noyywrap noinput nounput

%%

[ab]*a[ab]{13}b\n    { found++; }
[ab]*\n              { other++; }
.|\n                 { other++; }

%%

int main(void);

int
main (void)
{
    FILE *f = tmpfile();

    if (f == NULL) {
        printf("TEST FAILED: cannot make a temporary file.\n");
        return 1;
    }
    /* the first and third lines have an a, thirteen letters after it,
       and then a b; the second has no a */
    fputs("abbbbbbbbbbbbbb\n", f);
    fputs("bbbbbbbbbbbbbbb\n", f);
    fputs("bbaababababababab\n", f);
    rewind(f);
    yyin = f;
    yylex();
    fclose(f);
    if (found != 2 || other != 1) {
        printf("TEST FAILED: found %d, other %d.\n", found, other);
        return 1;
    }
    printf("TEST RETURNING OK.\n");
    return 0;
}
