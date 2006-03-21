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

/* The point of this test is to be sure our M4 madness does not
 * interfere with user code. I particular, we are looking
 * for instances of M4 quotes, [[ and ]], in here to make it through the flex
 * machinery unscathed.
 */

/* sect 1     [ 1 ]           TEST_XXX */
/* sect 1    [[ 2 ]]          TEST_XXX */
/* sect 1   [[[ 3 ]]]         TEST_XXX */
/* sect 1  [[[[ 4 ]]]]        TEST_XXX */
/* sect 1  ]] unmatched [[    TEST_XXX */

%{
/* A template scanner file to build "scanner.c". */
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
/*#include "parser.h" */

/* sect 1 block    [ 1 ]        TEST_XXX */
/* sect 1 block   [[ 2 ]]       TEST_XXX */
/* sect 1 block  [[[ 3 ]]]      TEST_XXX */
/* sect 1 block [[[[ 4 ]]]]     TEST_XXX */
/* sect 1 block ]] unmatched [[ TEST_XXX */

static int a[1] = {0};
static int b[1] = {0};
static int c[1] = {0};

static int foo (int i){
    return a[b[c[i]]]; /* sect 1 code  TEST_XXX */
}
%}

%option 8bit outfile="scanner.c" prefix="test"
%option nounput nomain noyywrap 
%option warn


%%

a       /* action comment    [ 1 ]          */ ;
b       /* action comment   [[ 2 ]]         */ ;
c       /* action comment  [[[ 3 ]]]        */ ;
d       /* action comment [[[[ 4 ]]]]       */ ;
e       /* action comment ]] unmatched [[   */ ;
f       return 1+foo(a[b[c[0]]]);
.|\n    { 
            /* action block    [ 1 ]        TEST_XXX */
            /* action block   [[ 2 ]]       TEST_XXX */
            /* action block  [[[ 3 ]]]      TEST_XXX */
            /* action block [[[[ 4 ]]]]     TEST_XXX */
            /* action block ]] unmatched [[ TEST_XXX */
            return 1+foo(a[b[c[0]]]);  //   TEST_XXX
         }
%%

/* sect 3     [ 1 ]        TEST_XXX */
/* sect 3    [[ 2 ]]       TEST_XXX */
/* sect 3   [[[ 3 ]]]      TEST_XXX */
/* sect 3  [[[[ 4 ]]]]     TEST_XXX */
/* sect 3  ]] unmatched [[ TEST_XXX */
static int bar (int i){
    return c[b[a[i]]]; /* sect 3 code TEST_XXX */
}
int main(void);

int
main ()
{
    yyin = stdin;
    yyout = stdout;
    while (yylex())
        ;
    printf("TEST RETURNING OK.\n");
    return bar(0);
}

