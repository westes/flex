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
/* Test that a program can define its own functions with the names of the
   flex 2.6.5 macros, as GCC does with yybegin(), and that the new spellings
   still work inside actions. */
#include <stdio.h>
#include <stdlib.h>
#include "config.h"

void yybegin(const char *fname);
int yystart(void);
void yyecho(int c);
int yyatbol(void);
void yysetbol(int at_bol);

static int begins = 0;
%}

%option 8bit nomain nodefault noyywrap
%x SPECIAL

%%

a      { yybegin(SPECIAL); }
<SPECIAL>b   { if (yystart() == SPECIAL) begins++; yybegin(INITIAL); }
c      { yyecho(); yysetbol(1); if (yyatbol()) begins++; }
.|\n   { }

%%

/* the program's own functions; in actions the names above mean the flex ones */
void (yybegin)(const char *fname) { (void) fname; }
int (yystart)(void) { return 0; }
void (yyecho)(int c) { (void) c; }
int (yyatbol)(void) { return 0; }
void (yysetbol)(int at_bol) { (void) at_bol; }

int main(void);

int
main (void)
{
    yybuffer buffer = yy_scan_string("abc");

    yybegin("x");
    yylex();
    yy_delete_buffer(buffer);
    if (begins != 2) {
        printf("TEST FAILED: expected 2, got %d.\n", begins);
        return 1;
    }
    printf("\nTEST RETURNING OK.\n");
    return 0;
}
