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
#include <assert.h>
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

%option 8bit prefix="test"
%option nounput nomain noyywrap noinput
%option warn


%%
  /* indented code    [ 1 ] */
  /* indented code   [[ 2 ]] */
  /* indented code  [[[ 3 ]]] */
  /* indented code [[[[ 4 ]]]] */
  /* indented code ]] unmatched [[ */
%{
// non-indented code    [ 1 ]
// non-indented code   [[ 2 ]]
// non-indented code  [[[ 3 ]]]
// non-indented code [[[[ 4 ]]]]
%}

a       /* action comment    [ 1 ]          */ ;
b       /* action comment   [[ 2 ]]         */ ;
c       /* action comment  [[[ 3 ]]]        */ ;
d       /* action comment [[[[ 4 ]]]]       */ ;
e       /* action comment ]] unmatched [[   */ ;
f       return 1+foo(a[b[c[0]]]);
.|\n    {

#if 0
               action code     [ 1 ]        TEST_XXX
               action code    [[ 2 ]]       TEST_XXX
               action code   [[[ 3 ]]]      TEST_XXX
               action code  [[[[ 4 ]]]]     TEST_XXX
               action code  ]] unmatched [[ TEST_XXX
#endif
            /* action block    [ 1 ]        TEST_XXX */
            /* action block   [[ 2 ]]       TEST_XXX */
            /* action block  [[[ 3 ]]]      TEST_XXX */
            /* action block [[[[ 4 ]]]]     TEST_XXX */
            /* action block ]] unmatched [[ TEST_XXX */
            assert(!strcmp("[[ 2 ]]", "[""[ 2 ]""]"));
            assert(!strcmp("[[[ 3 ]]]", "[""[""[ 3 ]""]""]"));
            assert(!strcmp("[[[[ 4 ]]]]", "[""[""[""[ 4 ]""]""]""]"));
            assert(!strcmp("]] unmatched [[", "]""] unmatched [""["));
            assert(!strcmp("]]m4_define(alpha, beta)[[",
             "]""]m4_""define(alpha, beta)[""["));
            return 1+foo(a[b[c[0]]]);  /*   TEST_XXX */
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

#define CONCAT_IDENTS(a, b) a##b
int
main (void)
{
    /* m4_m4exit(100) */
    FILE *M4_YY_NOT_IN_HEADER = stdin;
    yyin = CONCAT_IDENTS(M4_, YY_NOT_IN_HEADER);
    yyout = stdout;
    while (yylex())
        ;
    assert(!strcmp("YY_G( alpha)", "Y""Y_G( alpha)"));
    printf("TEST RETURNING OK.\n");
    return bar(0);
}

