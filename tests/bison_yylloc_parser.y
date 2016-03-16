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

%parse-param { void* scanner }
%lex-param { void* scanner }

/* 
   How to compile:
   bison --defines --output-file="bison_yylloc_parser.c" --name-prefix="test" parser.y
 */
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "bison_yylloc_parser.h"
#include "bison_yylloc_scanner.h"

int yyerror(YYLTYPE *location, void* scanner, const char* msg);

#define YYERROR_VERBOSE 1


/* A dummy function. A check against seg-faults in yylval->str. */
static int process_text(char* s) {
    int total =0;
    while(*s) {
        total += (int) *s;
        ++s;
    }
    return total;
}


%}

%pure-parser

%union  {
    int  lineno;
    char * str;
}
%token <str> IDENT
%token <lineno> LINENO
%token  EQUAL "="
%token  COLON ":"
%token  SPACE " "
%%

file:
     line
  |  file line
  ;

line:
    LINENO COLON SPACE IDENT EQUAL IDENT
    {
        process_text($4);
        process_text($6);
        /* Check lineno. */
        if( $1 != @1.first_line || $1 != testget_lineno(scanner))
        {
            yyerror(0, 0, "Parse failed: Line numbers do not match.");
            YYABORT;
        }

        /* Recreate the line to stdout. */
        printf ( "%04d: %s=%s\n", @1.first_line, $4, $6);
    }
    ;

%%

int yyerror(YYLTYPE *location, void* scanner, const char* msg) {
    (void)location;
    (void)scanner;
    fprintf(stderr,"%s\n",msg);
    return 0;
}

