%{
/* The scanner expects to link to bison yylval . */
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "config.h"
static char* STRDUP(char* s1);
#define YY_EXTRA_TYPE int
%}

%option 8bit outfile="scanner.c" prefix="test"
%option reentrant-bison yylineno
%option nomain nounput noyy_top_state noyywrap nodefault warn
%option prefix="test"


%%

    if ( !yyextra)
        yyextra = 1;

^[[:digit:]]+  { 
        yylval->lineno = yyextra++;
        yylloc->first_line = (int)strtol(yytext,NULL,10);
        return LINENO;
    }
":"  { return COLON; }
" "  { return SPACE; }
"="  { return EQUAL; }
[[:alnum:]_]+ {  yylval->str = STRDUP(yytext); return IDENT;}

\r|\n { }
.     { yyterminate();}
%%


static char* STRDUP(char* s1)
{
    char* s2 = (char*)malloc(strlen(s1)+1);
    sprintf(s2,"%s",s1);
    return s2;
}
