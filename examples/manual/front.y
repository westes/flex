/* C code supplied at the beginning of the file.  */

%{

#include <stdio.h>                          
#include <string.h>

extern int  yylexlinenum;                   /* these are in YYlex      */
extern char *yytext;                        /* current token           */


%}

/* Keywords and reserved words begin here.  */

%union{                                     /* this is the data union  */
    char   name[128];                       /* names                   */
}

/*-------------------- the reserved words -----------------------------*/

%token PERIOD
%token NEWLINE
%token POSITIONAL

%token VERB
%token ADVERB

%token PROPER_NOUN
%token NOUN

%token DECLARATIVE
%token CONDITIONAL


%type  <name> declarative
%type  <name> verb_phrase
%type  <name> noun_phrase
%type  <name> position_phrase
%type  <name> adverb

%type  <name> POSITIONAL VERB ADVERB PROPER_NOUN 
%type  <name> NOUN DECLARATIVE CONDITIONAL

%%

sentence_list : sentence
              | sentence_list NEWLINE sentence
              ;


sentence : verb_phrase noun_phrase position_phrase adverb period 
           {
             printf("I understand that sentence.\n");
             printf("VP = %s \n",$1);
             printf("NP = %s \n",$2);
             printf("PP = %s \n",$3);
             printf("AD = %s \n",$4);
           }
         | { yyerror("That's a strange sentence !!");  }
         ;

position_phrase : POSITIONAL  declarative PROPER_NOUN 
                  {
                    sprintf($$,"%s %s %s",$1,$2,$3);
                  }   
                | /* empty */ { strcpy($$,""); }
                ;
               

verb_phrase : VERB { strcpy($$,$1); strcat($$," "); }
            | adverb VERB  
              {
                sprintf($$,"%s %s",$1,$2);
              }
            ;

adverb : ADVERB      { strcpy($$,$1); }
       | /* empty */ { strcpy($$,""); }
       ;

noun_phrase : DECLARATIVE NOUN 
              {
                sprintf($$,"%s %s",$1,$2);
              }
            | CONDITIONAL declarative NOUN 
                  {
                    sprintf($$,"%s %s %s",$1,$2,$3);
                  }   
            | NOUN { strcpy($$,$1); strcat($$," "); }
            ;

declarative : DECLARATIVE { strcpy($$,$1); }
            | /* empty */ { strcpy($$,""); }
            ;

period : /* empty */
       | PERIOD
       ;


%%

/* Supplied main() and yyerror() functions.  */

int main(int argc, char *argv[])
{
  yyparse();   /* parse the file          */
  return(0);
}

int yyerror(char *message)
{
  extern FILE *yyout;

  fprintf(yyout,"\nError at line %5d. (%s) \n",
                     yylexlinenum,message);
}
