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
%option nounput noyywrap yylineno warn nodefault

IDENT [[:alnum:]_-]
WS    [[:blank:]]
%%

^{IDENT}+{WS}*={WS}*(true|false){WS}*\r?\n    { return 100;}
^{IDENT}+{WS}*={WS}*\"[^\"\n\r]*\"{WS}*\r?\n  { return 101;}
^{IDENT}+{WS}*={WS}*[[:digit:]]+{WS}*\r?\n    { return 102;}
^{WS}*#.*\r?\n     { }
^{WS}*\r?\n        { }
.  { fprintf(stderr,"Invalid line.\n"); exit(-1);}

%%

int main (int argc, char** argv )
{
    yyin = stdin;
    yyout = stdout;
    while( yylex() )
    {
    }
    printf("TEST RETURNING OK.\n");
    return 0;
}

