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
%option warn reentrant


%%

.              { }

%%

int
main ( int argc, char** argv )
{
    yyscan_t  lexer;
    yylex_init( &lexer );
    yyset_out ( stdout,lexer);
    yyset_in  ( stdin, lexer);
    while( yylex(lexer) )
    {
    }
    yylex_destroy( lexer );
    printf("TEST RETURNING OK.\n");
    return 0;
}
