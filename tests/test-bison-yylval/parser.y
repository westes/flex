/* Accepts html-like input.
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
    long unused;
    char * str;
}

%token <str> TAGNAME TEXT
%token  LT 
%token  GT
%token  LTSLASH "</"

%%

html:
    TEXT { process_text($1); free($1);}
  | starttag html endtag
  | html TEXT { process_text($2); free($2);}
  | html starttag html endtag  
  ;

starttag:  LT      TAGNAME GT { process_text($2); free($2);} ;
endtag:    LTSLASH TAGNAME GT { process_text($2);free($2);} ;
%%

int yyerror(char* msg) {
    fprintf(stderr,"%s\n",msg);
    return 0;
}
