/*
 * yymore.lex: An example of using yymore()
 *             to good effect.
 */

%{
#include <memory.h>

void yyerror(char *message)
{
  printf("Error: %s\n",message);
}

%}

%x STRING

%%
\"   BEGIN(STRING);

<STRING>[^\\\n"]*  yymore();
<STRING><<EOF>>    yyerror("EOF in string.");       BEGIN(INITIAL);
<STRING>\n         yyerror("Unterminated string."); BEGIN(INITIAL);
<STRING>\\\n      {
                     bcopy(yytext,yytext+2,yyleng-2);
                     yytext += 2; yyleng -= 2;
                     yymore();
                  }
<STRING>\"        {
                     yyleng -= 1; yytext[yyleng] = '\0';
                     printf("string = \"%s\"",yytext); BEGIN(INITIAL);
                  }
%%
