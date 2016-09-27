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
/* A template scanner file to build "scanner.c". */
#include <stdio.h>
#include <stdlib.h>
#include "config.h"

%}

%option 8bit prefix="test"
%option nounput nomain noyywrap noinput
%option warn debug reentrant

%%
.+   { }
\n   { }
%%

int main(void);

int main (void)
{
    yyscan_t  lexer;
    testlex_init( &lexer );
    testset_out ( stdout,lexer);
    testset_in  ( stdin, lexer);
    
    /* Just see if the next line compiles. */
    testset_debug (testget_debug(lexer), lexer);
    
    while( testlex(lexer) )
    {
    }
    testlex_destroy( lexer );
    printf("TEST RETURNING OK.\n");
    return 0;
}
