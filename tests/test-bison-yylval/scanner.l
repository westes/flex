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
%option reentrant-bison
%option noyywrap nomain nounput noyy_top_state noyywrap nodefault warn
%option prefix="test"
%option stack


%x IN_TAG
%x DISCARD_THRU_GT
%%

    #define YY_USER_INIT yyextra = no;
    #define NEED_TAG_NAME yyextra
        

<INITIAL>{
"</"       { NEED_TAG_NAME= yes; yy_push_state( IN_TAG, yy_globals); return LTSLASH;}
"<"[^[:alpha:]]  { yy_push_state(DISCARD_THRU_GT,yy_globals); }
"<"        { NEED_TAG_NAME= yes; yy_push_state( IN_TAG, yy_globals); return  LT; }
[^<]{1,512} { yyget_lval(yy_globals)->str = STRDUP(yytext); return TEXT;}
}
<IN_TAG>{
">"  { yy_pop_state( yy_globals ); return GT; }
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
">"        { yy_pop_state(yy_globals);}
}
%%


static char* STRDUP(char* s1)
{
    char* s2 = (char*)malloc(strlen(s1)+1);
    sprintf(s2,"%s",s1);
    return s2;
}
