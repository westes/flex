/*
 * cat.lex: A demonstration of YY_NEW_FILE.
 */

%{
#include <stdio.h>

char **names = NULL;
int  current = 1;
%}

%%
<<EOF>> {
           current += 1;
           if(names[current] != NULL){
              yyin = fopen(names[current],"r");
              if(yyin == NULL){
                fprintf(stderr,"cat: unable to open %s\n",
                        names[current]);
                yyterminate();
              }
              YY_NEW_FILE;
           } else {
             yyterminate();
           }
        }
%%

int main(int argc, char **argv)
{
    if(argc < 2){
       fprintf(stderr,"Usage: cat files....\n");
       exit(1);
    }
    names = argv;

    yyin = fopen(names[current],"r");
    if(yyin == NULL){
      fprintf(stderr,"cat: unable to open %s\n",
              names[current]);
      yyterminate();
    }

    yylex();
}
