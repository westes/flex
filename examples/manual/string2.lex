/*
 * string2.lex: An example of using scanning strings
 *              by using start states.
 */

%{
#include <ctype.h>
#include <malloc.h>

#define isodigit(x) ((x) >= '0' && (x) <= '7') 
#define hextoint(x) (isdigit((x)) ? (x) - '0' : ((x) - 'A') + 10)  

char *buffer      = NULL;
int  buffer_size  = 0;

void yyerror(char *message)
{
  printf("\nError: %s\n",message);
}

%}

%x STRING

hex (x|X)[0-9a-fA-F]{1,2}
oct [0-7]{1,3}

%%

\"                 { 
                     buffer      = malloc(1); 
                     buffer_size = 1; strcpy(buffer,"");
                     BEGIN(STRING);
                   }
<STRING>\n         {
                      yyerror("Unterminated string");       
                      free(buffer);
                      BEGIN(INITIAL);
                   }
<STRING><<EOF>>    {
                      yyerror("EOF in string");       
                      free(buffer);
                      BEGIN(INITIAL);
                   }
<STRING>[^\\\n"]   {
                     buffer = realloc(buffer,buffer_size+yyleng+1);
                     buffer_size += yyleng;
                     strcat(buffer,yytext);
                   }
<STRING>\\\n       /* ignore this */
<STRING>\\{hex}    {
                     int temp =0,loop = 0;
                     for(loop=yyleng-2; loop>0; loop--){
                       temp  <<= 4;
                       temp  += hextoint(toupper(yytext[yyleng-loop]));
                     } 
                     buffer = realloc(buffer,buffer_size+1);
                     buffer[buffer_size-1] = temp;
                     buffer[buffer_size]   = '\0';
                     buffer_size += 1;
                   }
<STRING>\\{oct}    {
                     int temp =0,loop = 0;
                     for(loop=yyleng-1; loop>0; loop--){
                       temp  <<= 3;
                       temp  += (yytext[yyleng-loop] - '0');
                     } 
                     buffer = realloc(buffer,buffer_size+1);
                     buffer[buffer_size-1] = temp;
                     buffer[buffer_size]   = '\0';
                     buffer_size += 1;
                   }
<STRING>\\[^\n]    {
                     buffer = realloc(buffer,buffer_size+1);
                     switch(yytext[yyleng-1]){
                     case 'b' : buffer[buffer_size-1] = '\b';  break;
                     case 't' : buffer[buffer_size-1] = '\t';  break;
                     case 'n' : buffer[buffer_size-1] = '\n';  break;
                     case 'v' : buffer[buffer_size-1] = '\v';  break;
                     case 'f' : buffer[buffer_size-1] = '\f';  break;
                     case 'r' : buffer[buffer_size-1] = '\r';  break;
                     default  : buffer[buffer_size-1] = yytext[yyleng-1];
                     }
                     buffer[buffer_size] = '\0';
                     buffer_size += 1;
                   }
<STRING>\"         {
                     printf("string = \"%s\"",buffer); 
                     free(buffer);
                     BEGIN(INITIAL);
                   }
%%


