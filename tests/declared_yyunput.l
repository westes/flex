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
/* Test that a program can declare yyunput() itself, as iproute does to
   silence an unused-function warning, and that yyunput(c) in an action
   still pushes a character back. */
#include <stdio.h>
#include <stdlib.h>
#include "config.h"

static void yyunput (int c, char *buf_ptr);

static int saw_b = 0;
%}

%option 8bit nomain nodefault noyywrap

%%

a      { yyunput('b'); }
b      { saw_b = 1; }
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
    fputs("a", yyin);
    rewind(yyin);
    yylex();
    fclose(yyin);
    if (!saw_b) {
        printf("TEST FAILED: the pushed-back character was not read.\n");
        return 1;
    }
    printf("TEST RETURNING OK.\n");
    return 0;
}
