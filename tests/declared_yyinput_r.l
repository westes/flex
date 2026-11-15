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
/* Test that a program can declare yyinput() itself in a reentrant scanner,
   and that yyinput(yyscanner) and input(yyscanner) in an action still read
   the next character. */
#include <stdio.h>
#include <stdlib.h>
#include "config.h"

extern int yyinput(void *scanner);

static int first = -1;
static int second = -1;
%}

%option 8bit nomain nodefault noyywrap reentrant

%%

a      { first = yyinput(yyscanner); second = input(yyscanner); }
.|\n   { }

%%

int main(void);

int
main (void)
{
    yyscan_t scanner;

    if (yylex_init(&scanner) != 0) {
        printf("TEST FAILED: no scanner.\n");
        return 1;
    }
    yyset_in(tmpfile(), scanner);
    if (yyget_in(scanner) == NULL) {
        printf("TEST FAILED: no temporary file.\n");
        return 1;
    }
    fputs("aXY", yyget_in(scanner));
    rewind(yyget_in(scanner));
    yylex(scanner);
    fclose(yyget_in(scanner));
    yylex_destroy(scanner);
    if (first != 'X' || second != 'Y') {
        printf("TEST FAILED: read %d and %d, not X and Y.\n", first, second);
        return 1;
    }
    printf("TEST RETURNING OK.\n");
    return 0;
}
