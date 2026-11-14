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
/* Test that yyinput() leaves the input buffer as it found it. The scanner
   reads a buffer supplied by the program, so any byte it overwrites with a
   NUL and does not put back is visible to the program afterwards. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

static int reads;
static int lengths_wrong;

static void
check_length(int expected)
{
    if (yyleng != expected)
        lengths_wrong++;
}
%}

%option 8bit nomain noyywrap nounput nodefault

%%

fle    { if (input() == 'x') { reads++; check_length(3); } }
F      { if (input() == 'L') reads++;
         if (input() == 'E') reads++;
         if (input() == 'X') reads++;
         check_length(1); }
.|\n   { }

%%

int main(void);

static int
try_buffer(const char *text)
{
    char buf[16];
    yybuffer b;

    memset(buf, 0, sizeof buf);
    strcpy(buf, text);
    b = yy_scan_buffer(buf, strlen(text) + 2);
    yylex();
    yy_delete_buffer(b);
    if (strcmp(buf, text) != 0) {
        printf("TEST FAILED: %s came back as %s.\n", text, buf);
        return 1;
    }
    return 0;
}

int
main (void)
{
    if (try_buffer("flex") || try_buffer("FLEX"))
        return 1;
    if (reads != 4) {
        printf("TEST FAILED: expected 4 characters read, got %d.\n", reads);
        return 1;
    }
    if (lengths_wrong) {
        printf("TEST FAILED: yyleng changed when input() read characters.\n");
        return 1;
    }
    printf("TEST RETURNING OK.\n");
    return 0;
}
