/*
 * expr.y : A simple yacc expression parser
 *          Based on the Bison manual example. 
 */

%{
#include <stdio.h>
#include <math.h>

%}

%union {
   float val;
}

%token NUMBER
%token PLUS MINUS MULT DIV EXPON
%token EOL
%token LB RB

%left  MINUS PLUS
%left  MULT DIV
%right EXPON

%type  <val> exp NUMBER

%%
input   :
        | input line
        ;

line    : EOL
        | exp EOL { printf("%g\n",$1);}

exp     : NUMBER                 { $$ = $1;        }
        | exp PLUS  exp          { $$ = $1 + $3;   }
        | exp MINUS exp          { $$ = $1 - $3;   }
        | exp MULT  exp          { $$ = $1 * $3;   }
        | exp DIV   exp          { $$ = $1 / $3;   }
        | MINUS  exp %prec MINUS { $$ = -$2;       }
        | exp EXPON exp          { $$ = pow($1,$3);}
        | LB exp RB                      { $$ = $2;        }
        ;

%%

yyerror(char *message)
{
  printf("%s\n",message);
}

int main(int argc, char *argv[])
{
  yyparse();
  return(0);
}








