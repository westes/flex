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

/* TEST scanner.
   Basic non-reentrant scanner.
   Compile with:  flex scanner.l

   Sample Input:
       # this is a comment
       foo = true
       bar = "string value"
       integer = 43
*/
%{
#include "config.h"
%}

%option prefix="test" outfile="scanner.c" 
%option nounput noyywrap noyylineno warn nodefault

IDENT [[:alnum:]_-]
WS    [[:blank:]]
%%

^{IDENT}+{WS}*={WS}*(true|false){WS}*\r?\n    { return 100;}
^{IDENT}+{WS}*={WS}*\"[^\"\n\r]*\"{WS}*\r?\n  { return 101;}
^{IDENT}+{WS}*={WS}*[[:digit:]]+{WS}*\r?\n    { return 102;}
^{WS}*#.*\r?\n     { }
^{WS}*\r?\n        { }
.|\n  { fprintf(stderr,"Invalid line.\n"); exit(-1);}

%%

int main(void);

int main ()
{
    yyin = stdin;
    yyout = stdout;
    while( yylex() )
    {
    }
    printf("TEST RETURNING OK.\n");
    return 0;
}

