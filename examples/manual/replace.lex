/*
 * replace.lex : A simple filter for renaming
 *               parts of flex of bison generated
 *               scanners or parsers.
 */

%{
#include <stdio.h>

char lower_replace[1024];
char upper_replace[1024];

%}

%%

"yy"   printf("%s",lower_replace); 
"YY"   printf("%s",upper_replace);
,      ECHO;

%%

int main(int argc, char *argv[])
{
   if(argc < 2){
     printf("Usage %s lower UPPER\n",argv[0]);
     exit(1);
   }
   strcpy(lower_replace,argv[1]);
   strcpy(upper_replace,argv[2]);
   yylex();
   return(0);
}
