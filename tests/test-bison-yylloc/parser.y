/* 
   How to compile:
   bison --defines --output-file="parser.c" --name-prefix="test" parser.y
 */
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

#define YYERROR_VERBOSE 1
#define YYPARSE_PARAM scanner
#define YYLEX_PARAM   scanner

int yyerror(char* msg);

/* A dummy function. A check against seg-faults in yylval->str. */
int process_text(char* s) {
    int total =0;
    while(*s) {
        total += (int) *s;
        ++s;
    }
    return total;
}


%}

%pure_parser

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
            yyerror("Parse failed: Line numbers do not match.");
            YYABORT;
        }

        /* Recreate the line to stdout. */
        printf ( "%04d: %s=%s\n", @1.first_line, $4, $6);
    }
    ;

%%

int yyerror(char* msg) {
    fprintf(stderr,"%s\n",msg);
    return 0;
}

