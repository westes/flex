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
/* The scanner expects to link to bison yylval . */
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "config.h"
static char* STRDUP(char* s1);
#define YY_EXTRA_TYPE int
%}

%option 8bit outfile="scanner.c" prefix="test"
%option reentrant bison-bridge bison-locations yylineno
%option nomain nounput noyy_top_state noyywrap nodefault warn
%option prefix="test" header="scanner.h"


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
