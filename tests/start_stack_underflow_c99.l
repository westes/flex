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
/* The c99 version of start_stack_underflow: test that yy_pop_state() on
   an empty start-condition stack leaves the stack usable when the program
   recovers from the fatal error, here from yypanic(), with longjmp(). The
   scanner's memory comes from yyalloc() below, which puts guard bytes in
   front of each block, so a push that writes in front of the stack is seen
   when the block is freed. */
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

enum { guard = 16 };

static int errors;
static jmp_buf fatal_env;
static const char *fatal_msg;
static yyscan_t scanner;

static void
fatal(const char *msg)
{
    fatal_msg = msg;
    longjmp(fatal_env, 1);
}

#define yypanic(msg, yyscanner) fatal(msg)

void *yyalloc(size_t, yyscan_t);
void *yyrealloc(void *, size_t, yyscan_t);
void yyfree(void *, yyscan_t);
%}

%option emit="c99"
%option 8bit nomain noyywrap nodefault nounput noinput noyypanic stack noyy_top_state
%option noyyalloc noyyrealloc noyyfree

%x INNER

%%

<INITIAL,INNER>"("   yy_push_state(INNER, yyscanner);
<INITIAL,INNER>")"   yy_pop_state(yyscanner);
<INITIAL,INNER>.|\n  ;

%%

int main(void);

static yybuffer buffer;

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
yyalloc(size_t n, yyscan_t yyscanner)
{
    unsigned char *p = malloc(n + guard);

    (void) yyscanner;
    if (p == NULL)
        return NULL;
    memset(p, 0xa5, guard);
    return p + guard;
}

void *
yyrealloc(void *ptr, size_t n, yyscan_t yyscanner)
{
    unsigned char *p;

    if (ptr == NULL)
        return yyalloc(n, yyscanner);
    check_guard((unsigned char *) ptr - guard);
    p = realloc((unsigned char *) ptr - guard, n + guard);
    return p == NULL ? NULL : p + guard;
}

void
yyfree(void *ptr, yyscan_t yyscanner)
{
    (void) yyscanner;
    if (ptr == NULL)
        return;
    check_guard((unsigned char *) ptr - guard);
    free((unsigned char *) ptr - guard);
}

/* Scan TEXT and check that it stops with the fatal error WANT, or with no
   error if WANT is NULL. */
static void
scan(const char *text, const char *want)
{
    fatal_msg = NULL;
    if (setjmp(fatal_env) == 0) {
        buffer = yy_scan_string(text, scanner);
        yylex(scanner);
    }
    yy_delete_buffer(buffer, scanner);
    if (want == NULL ? fatal_msg != NULL
        : fatal_msg == NULL || strcmp(fatal_msg, want) != 0) {
        printf("TEST FAILED: scanning \"%s\" gave %s%s%s, not %s%s%s.\n",
               text,
               fatal_msg ? "\"" : "", fatal_msg ? fatal_msg : "no error",
               fatal_msg ? "\"" : "",
               want ? "\"" : "", want ? want : "no error", want ? "\"" : "");
        errors++;
    }
}

int
main (void)
{
    yylex_init(&scanner);
    scan("()", NULL);
    scan(")", "start-condition stack underflow");
    scan("(())", NULL);
    if (yystart(scanner) != INITIAL) {
        printf("TEST FAILED: start condition is %d, not INITIAL.\n",
               yystart(scanner));
        errors++;
    }
    yylex_destroy(scanner);
    if (errors)
        return 1;
    printf("TEST RETURNING OK.\n");
    return 0;
}
