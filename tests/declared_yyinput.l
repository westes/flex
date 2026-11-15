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
/* Test that a program can declare yyinput() itself, as VirtualBox does in
   a header, and that yyinput() and input() in an action still read the
   next character. A C scanner names its function input(), as flex 2.6.4
   did, so the declaration does not conflict with it. */
#include <stdio.h>
#include <stdlib.h>
#include "config.h"

extern int yyinput(void);

static int first = -1;
static int second = -1;
%}

%option 8bit nomain nodefault noyywrap

%%

a      { first = yyinput(); second = input(); }
.|\n   { }

%%

int main(void);

int
main (void)
{
    yyin = tmpfile();
    if (yyin == NULL) {
        printf("TEST FAILED: no temporary file.\n");
        return 1;
    }
    fputs("aXY", yyin);
    rewind(yyin);
    yylex();
    fclose(yyin);
    if (first != 'X' || second != 'Y') {
        printf("TEST FAILED: read %d and %d, not X and Y.\n", first, second);
        return 1;
    }
    printf("TEST RETURNING OK.\n");
    return 0;
}
