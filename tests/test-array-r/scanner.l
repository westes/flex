%{
/* A template scanner file to build "scanner.c". */
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
/*#include "parser.h" */

%}

%option 8bit outfile="scanner.c" prefix="test"
%option nounput nomain noyywrap 
%option warn array reentrant


%%

.|\n    { }


%%

int
main ( int argc, char** argv )
{
    yyscan_t lexer;
    
	yylex_init(&lexer);
    yyset_in(stdin, lexer);
    yyset_out(stdout, lexer);
	
    yylex( lexer );
	
    yylex_destroy( lexer);
    printf("TEST RETURNING OK.\n");

    return 0;
}
