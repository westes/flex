%{

#include <ctype.h>

void user_action(void);

#define YY_USER_ACTION user_action();

%}

%%

.*         ECHO;
\n         ECHO;

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




