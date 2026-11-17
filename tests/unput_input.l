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
/* Test input() after unput() has pushed back more characters than the
   token had, when input() then has to read more of the file. The
   scanner's memory comes from yyalloc() below, which puts guard bytes in
   front of each block, so a write in front of the input buffer is seen
   when the block is freed. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

enum { guard = 16 };

static int errors;
static size_t angle_length;
static char log_text[64];

static void
note(const char *text)
{
    strcat(log_text, text);
    strcat(log_text, "|");
}
%}

%option 8bit nomain noyywrap nodefault
%option noyyalloc noyyrealloc noyyfree

%%

"<"    { int c;
         size_t length = 0;

         unput('c');
         unput('b');
         unput('a');
         while ((c = input()) != 0 && c != '>')
             length++;
         angle_length = length; }
.|\n   { note(yytext); }

%%

int main(void);

static void
check_guard(unsigned char *p)
{
    int i;

    for (i = 0; i < guard; i++) {
        if (p[i] != 0xa5) {
            printf("TEST FAILED: byte %d in front of a block was changed.\n",
                   guard - i);
            errors++;
            return;
        }
    }
}

void *
yyalloc(yy_size_t n)
{
    unsigned char *p = malloc(n + guard);

    if (p == NULL)
        return NULL;
    memset(p, 0xa5, guard);
    return p + guard;
}

void *
yyrealloc(void *ptr, yy_size_t n)
{
    unsigned char *p;

    if (ptr == NULL)
        return yyalloc(n);
    check_guard((unsigned char *) ptr - guard);
    p = realloc((unsigned char *) ptr - guard, n + guard);
    return p == NULL ? NULL : p + guard;
}

void
yyfree(void *ptr)
{
    if (ptr == NULL)
        return;
    check_guard((unsigned char *) ptr - guard);
    free((unsigned char *) ptr - guard);
}

int
main (void)
{
    enum { body = 40000 };
    FILE *f = tmpfile();
    int i;

    if (f == NULL) {
        printf("TEST FAILED: no temporary file.\n");
        return 1;
    }
    fputs("zz<", f);
    for (i = 0; i < body; i++)
        fputc(i % 50 == 49 ? '\n' : 'x', f);
    fputs("> y", f);
    rewind(f);
    yyin = f;
    yylex();
    yylex_destroy();
    fclose(f);
    if (strcmp(log_text, "z|z| |y|") != 0) {
        printf("TEST FAILED: scanned \"%s\", not \"z|z| |y|\".\n", log_text);
        errors++;
    }
    if (angle_length != body + 3) {
        printf("TEST FAILED: input() read %lu characters, not %d.\n",
               (unsigned long) angle_length, body + 3);
        errors++;
    }
    if (errors)
        return 1;
    printf("TEST RETURNING OK.\n");
    return 0;
}
