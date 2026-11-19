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
/* A -Cf scanner built without -7 or -8 must handle bytes above 127.
   flex used to make such scanners 7-bit, and their tables have no
   entries for those bytes, so the scanner read past them and skipped
   or misread input. */
#include <stdio.h>
#include <stdlib.h>
#include "config.h"

static int letters, others;
%}

%option full nomain nodefault noyywrap noinput nounput

%%

[a-z]   { letters++; }
.|\n    { others++; }

%%

int main(void);

int
main (void)
{
    static const char input[] = "a\303\251b\377\376\200c\n";

    yyin = tmpfile();
    if (yyin == NULL) {
        printf("TEST FAILED: no temporary file.\n");
        return 1;
    }
    fputs(input, yyin);
    rewind(yyin);
    yylex();
    fclose(yyin);
    if (letters != 3 || others != 6) {
        printf("TEST FAILED: %d letters and %d others, not 3 and 6.\n",
               letters, others);
        return 1;
    }
    printf("TEST RETURNING OK.\n");
    return 0;
}
