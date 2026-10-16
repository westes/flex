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
/* The reject state buffer must be managed with yyalloc(), yyrealloc() and
 * yyfree() only. Switching to a buffer larger than YY_BUF_SIZE makes the
 * scanner grow the state buffer, so every pointer handed to yyrealloc()
 * and yyfree() is checked against the ones yyalloc() handed out.
 */
#include <cstdio>
#include <cstdlib>
#include "config.h"

%}

%option 8bit prefix="test"
%option warn c++
%option nounput nomain noinput noyywrap
%option noyyalloc noyyrealloc noyyfree

%%

foo            { REJECT; }
.|\n           { }

%%

#define MAX_PTRS 64
static void *ptrs[MAX_PTRS];

static void track(void *p)
{
    int i;
    for (i = 0; i < MAX_PTRS; i++) {
        if (ptrs[i] == NULL) {
            ptrs[i] = p;
            return;
        }
    }
    fprintf(stderr, "too many allocations\n");
    exit(1);
}

static void untrack(void *p)
{
    int i;
    if (p == NULL)
        return;
    for (i = 0; i < MAX_PTRS; i++) {
        if (ptrs[i] == p) {
            ptrs[i] = NULL;
            return;
        }
    }
    fprintf(stderr, "pointer %p was not allocated by yyalloc\n", p);
    exit(1);
}

void *testalloc(yy_size_t n)
{
    void *p = malloc(n);
    if (p)
        track(p);
    return p;
}

void *testrealloc(void *p, yy_size_t n)
{
    void *q;
    untrack(p);
    q = realloc(p, n);
    if (q)
        track(q);
    return q;
}

void testfree(void *p)
{
    untrack(p);
    free(p);
}

int main(void);

int
main (void)
{
    yyFlexLexer *f = new yyFlexLexer(&std::cin, &std::cout);
    f->yy_switch_to_buffer(f->yy_create_buffer(&std::cin, 4 * YY_BUF_SIZE));
    f->yylex();
    delete f;
    std::cout << "TEST RETURNING OK." << std::endl;
    return 0;
}
