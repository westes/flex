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
/* Test yyless() after input() has had to grow the input buffer. An
   interactive scanner can end a token at the end of the buffer, so when
   the token fills the buffer, input() reads more of the file into a
   larger one. yyrealloc() below keeps each block it replaces, filled
   with a known byte, so a write into the old buffer is seen. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

enum { max_old = 8, fill = 0xa5 };

static int errors;
static int next_char;
static int string_length;
static char log_text[64];
static unsigned char *old_block[max_old];
static size_t old_size[max_old];
static int old_count;

static void
note(const char *text)
{
    strcat(log_text, text);
    strcat(log_text, "|");
}

static void
check_old_blocks(void)
{
    int i;
    size_t j;

    for (i = 0; i < old_count; i++) {
        for (j = 0; j < old_size[i]; j++) {
            if (old_block[i][j] != fill) {
                printf("TEST FAILED: byte %lu of a replaced buffer was changed.\n",
                       (unsigned long) j);
                errors++;
                return;
            }
        }
    }
}
%}

%option 8bit nomain noyywrap nounput nodefault interactive
%option noyyalloc noyyrealloc noyyfree

%%

\"[^"]*\"  { next_char = input();
             string_length = yyleng;
             yyless(string_length);
             check_old_blocks();
             if (errors)
                 yyterminate(); }
.|\n       { note(yytext); }

%%

int main(void);

/* Each block has its size in front of it, so yyrealloc() can copy it. */
void *
yyalloc(yy_size_t n)
{
    size_t *p = malloc(2 * sizeof(size_t) + n);

    if (p == NULL)
        return NULL;
    p[0] = n;
    return p + 2;
}

void *
yyrealloc(void *ptr, yy_size_t n)
{
    void *p;
    size_t old;

    if (ptr == NULL)
        return yyalloc(n);
    p = yyalloc(n);
    if (p == NULL)
        return NULL;
    old = ((size_t *) ptr)[-2];
    memcpy(p, ptr, old < n ? old : n);
    memset(ptr, fill, old);
    if (old_count < max_old) {
        old_block[old_count] = ptr;
        old_size[old_count++] = old;
    } else {
        yyfree(ptr);
    }
    return p;
}

void
yyfree(void *ptr)
{
    if (ptr != NULL)
        free((size_t *) ptr - 2);
}

int
main (void)
{
    FILE *f = tmpfile();
    int i;

    if (f == NULL) {
        printf("TEST FAILED: no temporary file.\n");
        return 1;
    }
    /* A string that fills the buffer, so input() has to grow it. */
    fputc('"', f);
    for (i = 0; i < YY_BUF_SIZE - 3; i++)
        fputc('a', f);
    fputs("\"xy", f);
    rewind(f);
    yyin = f;
    yylex();
    yylex_destroy();
    fclose(f);
    for (i = 0; i < old_count; i++)
        yyfree(old_block[i]);
    if (errors)
        return 1;
    if (old_count == 0) {
        printf("TEST FAILED: input() did not grow the buffer.\n");
        errors++;
    }
    if (string_length != YY_BUF_SIZE - 1 || next_char != 'x') {
        printf("TEST FAILED: string of %d characters followed by %d, not %d and %d.\n",
               string_length, next_char, YY_BUF_SIZE - 1, 'x');
        errors++;
    }
    if (strcmp(log_text, "x|y|") != 0) {
        printf("TEST FAILED: scanned \"%s\", not \"x|y|\".\n", log_text);
        errors++;
    }
    if (errors)
        return 1;
    printf("TEST RETURNING OK.\n");
    return 0;
}
