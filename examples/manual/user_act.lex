%{

#include <ctype.h>

void user_action(void);

%}

%option pre-action = "user_action();"
%%

.*         yyecho();
\n         yyecho();

%%

void user_action(void)
{
  int loop;
  
  for(loop=0; loop<yyleng; loop++){
    if(islower(yytext[loop])){
       yytext[loop] = toupper(yytext[loop]);
    }
  }
}




