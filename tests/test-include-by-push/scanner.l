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
/* A scanner file to build "scanner.c".
   Input language is any text.
   "#include <filename>" causes a buffer switch.
 */
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
%}

%option 8bit outfile="scanner.c" prefix="test"
%option nounput nomain noyywrap
%option warn

%x GET_FILENAME

%%

<INITIAL>{
^"#include"[[:blank:]]+"<"  { BEGIN(GET_FILENAME); }
.|\n      { ECHO; }
}

<GET_FILENAME>{
[[:alnum:]_.-]+>  {
     /* recurse */
    yytext[yyleng-1]='\0';
    if((yyin=fopen(yytext,"r"))==NULL) {
        fprintf(stderr,"*** Error: Could not open include file \"%s\".\n",yytext);
        yyterminate();
    }
    yypush_buffer_state( yy_create_buffer( yyin, YY_BUF_SIZE ));
    BEGIN(0);
    }
.|\n  {
    fprintf(stderr,"Invalid input \"%s\".\n", yytext);
    yyterminate();
   }
}

<<EOF>> {
           fclose(yyin);
           yypop_buffer_state();
           if(!YY_CURRENT_BUFFER)
               yyterminate();
        }

%%

int main(int argc, char** argv);

int
main ( int argc, char** argv )
{
    FILE * fp;
    if( argc != 2 ) {
        fprintf(stderr,"*** Error: Must specifiy one filename.\n");
        exit(-1);
    }
    if((fp=fopen(argv[1],"r"))==NULL) {
        fprintf(stderr,"*** Error: fopen(%s) failed.\n",argv[1]);
        exit(-1);
    }
    yyin = fp;
    yyout = stdout;
    yylex();
    printf("TEST RETURNING OK.\n");
    return 0;
}
