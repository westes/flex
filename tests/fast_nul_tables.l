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
/* The same as fast_nul, for a full-speed scanner whose tables are loaded
   from a tables file. A NUL in the input takes the end-of-buffer
   transition, and the scanner then indexes the end-of-buffer state with
   the class of the character after it. The loaded transition table used
   to end at that state, so that read went past it.

   tables_bad.h hands out each block with its end against an unreadable
   page, so a read past the table stops the program without a sanitizer. */
#include <stdio.h>
#include <stdlib.h>

void tb_fatal (const char *msg);
#define YY_FATAL_ERROR(msg) tb_fatal (msg)

static int as, nuls, others, bad_length;
%}

%option 8bit
%option nounput nomain nodefault noyywrap noinput
%option noyyalloc noyyrealloc noyyfree
%option fast warn tables-file="fast_nul_tables.tables"

%%

"a"     { as++; if (yyleng != 1) bad_length++; }
\0      { nuls++; if (yyleng != 1) bad_length++; }
.|\n    { others++; if (yyleng != 1) bad_length++; }

%%

#include "tables_bad.h"

int main (void);

int
main (void)
{
    static const char input[] = { 'a', '\0', 'b', 'a', '\0' };
    FILE *fp = fopen("fast_nul_tables.tables", "rb");
    yybuffer b;

    if (fp == NULL || yytables_fload(fp) < 0) {
        printf("TEST FAILED: cannot load the tables file.\n");
        return 1;
    }
    fclose(fp);
    b = yy_scan_bytes(input, sizeof input);
    yylex();
    yy_delete_buffer(b);
    yytables_destroy();
    if (as != 2 || nuls != 2 || others != 1 || bad_length != 0) {
        printf("TEST FAILED: %d a, %d NUL, %d others and %d bad lengths, "
               "not 2, 2, 1 and 0.\n", as, nuls, others, bad_length);
        return 1;
    }
    printf("TEST RETURNING OK.\n");
    return 0;
}
