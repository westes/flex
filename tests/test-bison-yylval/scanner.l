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

enum yesno_t { no=0, yes=1 };
#define YY_EXTRA_TYPE  enum yesno_t
%}

%option 8bit outfile="scanner.c" prefix="test"
%option reentrant bison-bridge
%option noyywrap nomain nounput noyy_top_state noyywrap nodefault warn
%option prefix="test" header="scanner.h"
%option stack


%x IN_TAG
%x DISCARD_THRU_GT
%%

    #define YY_USER_INIT yyextra = no;
    #define NEED_TAG_NAME yyextra
        

<INITIAL>{
"</"       { NEED_TAG_NAME= yes; yy_push_state( IN_TAG, yyscanner); return LTSLASH;}
"<"[^[:alpha:]]  { yy_push_state(DISCARD_THRU_GT,yyscanner); }
"<"        { NEED_TAG_NAME= yes; yy_push_state( IN_TAG, yyscanner); return  LT; }
[^<]{1,512} { yyget_lval(yyscanner)->str = STRDUP(yytext); return TEXT;}
}
<IN_TAG>{
">"  { yy_pop_state( yyscanner ); return GT; }
[[:alpha:]][[:alnum:]]* {
        if( NEED_TAG_NAME == yes){
            NEED_TAG_NAME=no;
            yylval->str = STRDUP(yytext);
            return TAGNAME;
        }
    }

.|\r|\n { }
}
<DISCARD_THRU_GT>{
[^>]{1,512} { }
">"        { yy_pop_state(yyscanner);}
}
%%


static char* STRDUP(char* s1)
{
    char* s2 = (char*)malloc(strlen(s1)+1);
    sprintf(s2,"%s",s1);
    return s2;
}
