%{
/* A template scanner file to build "scanner.c". 
   The scanner is not really important -- we only care if
   it compiles under a c++ compiler, and runs. */
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
/*#include "parser.h" */

%}

%option 8bit outfile="scanner.c" prefix="test"
%option nounput nomain noyywrap 
%option warn


%%

.              { }

%%

int
main ( int argc, char** argv )
{
    yyin = stdin;
    yyout = stdout;
    yylex();
    printf("TEST RETURNING OK.\n");
    return 0;
}
