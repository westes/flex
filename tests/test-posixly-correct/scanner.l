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
/* The goal of this test is to verify that we are getting the counter-intuitive
 * posix behavior of the repeat operator `{}'.
 *
 *  ab{3} -  In traditional flex, this matches "abbb".
 *           In posix, this matches "ababab".
 */
#include <stdio.h>
#include <stdlib.h>
#include "config.h"

#define NUM_TESTS 1
char * tests[NUM_TESTS] =  { "ababab"};
int main(void);

int tests_ok[NUM_TESTS] =  { 0 };

%}

%option 8bit outfile="scanner.c" prefix="test"
%option nounput nomain noyywrap
%option warn


%%

ab{3}    tests_ok[0] = 1; return 0;
.|\n     return 0;

%%


int main ()
{
    YY_BUFFER_STATE state;
    int i;

    yyin = stdin;
    yyout = stdout;

    /* Run the tests */
    for (i=0; i < NUM_TESTS; i++){
        printf("Testing: yy_scan_string(%s): ", tests[i]);
        state = yy_scan_string(tests[i]);
        yylex();
        yy_delete_buffer(state);
        printf("... %s\n", tests_ok[i] ? "OK" : "FAILED");
    }

    for (i=0; i < NUM_TESTS; i++)
        if (!tests_ok[i])
            exit(1);

    printf("TEST RETURNING OK.\n");
    return 0;
}
