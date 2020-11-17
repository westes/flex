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
/* The goal of this test is to verify that yyinput() sets correct byte to null
 * and also when called again it resets the correct byte to it's original value.
 */
#include <stdio.h>
#include <stdlib.h>

#define TESTBUF_SIZE 6

static char testBuffers[][TESTBUF_SIZE] = {
    "flex\0\0",
    "FLEX\0\0"
};

/* C lexer defines input() instead of yyinput() */
#ifndef __cplusplus
#   define yyinput input
#endif

%}

%option 8bit prefix="test"
%option nounput nomain noyywrap
%option warn posix-compat


%%

"fle" {
    yyinput();
}

"F" {
    yyinput();
    yyinput();
    yyinput();
}

%%

int testYYInput(char* testBuf)
{
    YY_BUFFER_STATE state;

    const char* origTestStr = strdup(testBuf);
    state = test_scan_buffer(testBuf, TESTBUF_SIZE);
    testlex();
    yy_delete_buffer(state);
    if (strcmp(origTestStr, testBuf) != 0)
    {
        printf("FAILED: Expected %s but left with %s\n", origTestStr, testBuf);
        return 1;
    }

    free((void*) origTestStr);
    printf("OK\n");
    return 0;
}

int main (void)
{
    int result = 0;
    /* Run the tests */
    printf("Testing yyinput()\n");
    result += testYYInput(testBuffers[0]);
    result += testYYInput(testBuffers[1]);
    
    if (result > 0)
    {
        printf("TEST FAILED.\n");
        exit(1);
    }
    
    printf("TEST RETURNING OK.\n");
    return 0;
}

