/*
 * unput.l : An example of what *not*
 *           to do with unput().
 */


%{
#include <stdio.h>

void putback_yytext(void);
%}

%%
foobar   putback_yytext();
raboof   putback_yytext();
%%

void putback_yytext(void)
{
    int   i;
    int   l = strlen(yytext);
    char  buffer[YY_BUF_SIZE];

    strcpy(buffer,yytext);
    printf("Got: %s\n",yytext);
    for(i=0; i<l; i++){
       unput(buffer[i]);
    }
}

 
           
