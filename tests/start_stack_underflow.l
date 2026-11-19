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
/* Test that yy_pop_state() on an empty start-condition stack leaves the
   stack usable when the program recovers from the fatal error with
   longjmp(). The scanner's memory comes from yyalloc() below, which puts
   guard bytes in front of each block, so a push that writes in front of
   the stack is seen when the block is freed. */
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

enum { guard = 16 };

static int errors;
static jmp_buf fatal_env;
static const char *fatal_msg;
static YY_BUFFER_STATE buffer;

static void
fatal(const char *msg)
{
    fatal_msg = msg;
    longjmp(fatal_env, 1);
}

#define YY_FATAL_ERROR(msg) fatal(msg)
%}

%option 8bit nomain noyywrap nodefault nounput noinput stack noyy_top_state
%option noyyalloc noyyrealloc noyyfree

%x INNER

%%

<INITIAL,INNER>"("   yy_push_state(INNER);
<INITIAL,INNER>")"   yy_pop_state();
<INITIAL,INNER>.|\n  ;

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

/* Scan TEXT and check that it stops with the fatal error WANT, or with no
   error if WANT is NULL. */
static void
scan(const char *text, const char *want)
{
    fatal_msg = NULL;
    if (setjmp(fatal_env) == 0) {
        buffer = yy_scan_string(text);
        yylex();
    }
    yy_delete_buffer(buffer);
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
    scan("()", NULL);
    scan(")", "start-condition stack underflow");
    scan("(())", NULL);
    if (YY_START != INITIAL) {
        printf("TEST FAILED: start condition is %d, not INITIAL.\n",
               YY_START);
        errors++;
    }
    yylex_destroy();
    if (errors)
        return 1;
    printf("TEST RETURNING OK.\n");
    return 0;
}
