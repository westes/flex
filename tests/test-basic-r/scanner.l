/* A reentrant scanner.
   This file will not compile under flex version <= 2.5.4.
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
%option reentrant

IDENT [[:alnum:]_-]
WS    [[:blank:]]
%%

^{IDENT}+{WS}*={WS}*(true|false){WS}*\r?\n    { return 100;}
^{IDENT}+{WS}*={WS}*\"[^\"\n\r]*\"{WS}*\r?\n  { return 101;}
^{IDENT}+{WS}*={WS}*[[:digit:]]+{WS}*\r?\n    { return 102;}
^{WS}*#.*\r?\n     { }
^{WS}*\r?\n        { }
.|\r|\n  { fprintf(stderr,"Invalid line.\n"); exit(-1);}

%%

int main (int argc, char** argv )
{
    void * lexer;
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

