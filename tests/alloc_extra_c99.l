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
/* A file to build "scanner.c". */
/* This tests that we can use "yyextra". 
   We buffer all input into a growable array, then print it.
   We run diff on the input and output.
*/

#include <stdio.h>
#include <stdlib.h>
#include "config.h"


/* We'll store the entire input in this buffer, growing as necessary. */
struct Check {
    char foo;
    char *bar;
    char qux;
};

void *yyalloc(size_t, yyscan_t);

/* Save char into junk array at next position. */
static void check_extra ( yyscan_t  scanner );

%}

%option emit="c99"
%option 8bit
%option nounput nomain noyywrap nodefault noinput
%option warn
%option extra-type="struct Check *"
%option noyyalloc


%%

.|\r|\n  { check_extra (yyscanner); }

%%

int main(void);

int
main (void)
{   
    yyscan_t scanner;
    struct Check check;

    check.foo = 'a';
    check.bar = NULL;
    check.qux = 'z';
 
    yylex_init_extra(&check, &scanner);
    yyset_in(stdin, scanner);
    yyset_out(stdout, scanner);

    /* Test to confirm that testalloc was called from
     * testlex_init_extra with the testextra argument.
     */
    check_extra(scanner);

    yylex(scanner);

    yylex_destroy(scanner);
    return 0;
}

/* Replaces the stock yyalloc */
void *yyalloc(size_t size, yyscan_t scanner)
{
    struct Check *check;
    check = yyget_extra(scanner);

    if (!check->bar)
        check->bar = "Hello World";

    check_extra(scanner);

    return malloc(size);
}

/* Save char into junk array at next position. */
static void check_extra(yyscan_t  scanner)
{
    struct Check *check;
    check = yyget_extra(scanner);

    if (check->foo != 'a') {
        fprintf(stderr, "foo is not 'a'\n");
        exit(1);
    }
    if (strcmp(check->bar, "Hello World") != 0) {
        fprintf(stderr, "bar is not Hello World\n");
        exit(1);
    }
    if (check->qux != 'z') {
        fprintf(stderr, "qux is not 'z'\n");
        exit(1);
    }
}
