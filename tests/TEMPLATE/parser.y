/* A dummy template parser to build "parser.c" and "parser.h".
   How to compile:
   bison --defines --output-file="parser.c" --name-prefix="test" parser.y
 */
%{
#include <stdio.h>
#include <stdlib.h>
#include "config.h"

#define YYERROR_VERBOSE 1       /* For debugging.   */
/* #define YYPARSE_PARAM scanner  */ /* For pure bison parser. */
/* #define YYLEX_PARAM   scanner  */ /* For reentrant flex. */


int yyerror(char* msg);
extern int testlex();

%}

%%

rule:  ;

%%

int yyerror(char* msg) {
    fprintf(stderr,"%s\n",msg);
    return 0;
}

/*
int
main ( int argc, char** argv )
{
    yyparse ();
    return 0;
}
*/
