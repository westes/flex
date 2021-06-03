/*
 * eof_rules.lex : An example of using multiple buffers
 *                 EOF rules, and start states
 */

%{
                              
#define MAX_NEST 10                   

YY_BUFFER_STATE include_stack[MAX_NEST];
int             include_count = -1;

%}


%x INCLUDE
%x COMMENT


%%

"{"                          yybegin(COMMENT);

<COMMENT>"}"                 yybegin(INITIAL); 
<COMMENT>"$include"[ \t]*"(" yybegin(INCLUDE);
<COMMENT>[ \t]*              /* skip whitespace */

<INCLUDE>")"                 yybegin(COMMENT); 
<INCLUDE>[ \t]*              /* skip whitespace */
<INCLUDE>[^ \t\n() ]+ {      /* get the include file name */
          if ( include_count >= MAX_NEST){
             fprintf( stderr, "Too many include files" );
             exit( 1 );
          }

          include_stack[++include_count] = yy_current_buffer();

          yyin = fopen( yytext, "r" );
          if ( ! yyin ){
             fprintf( stderr, "Unable to open %s",yytext);
             exit( 1 );
          }

          yy_switch_to_buffer(yy_create_buffer(yyin,YY_BUF_SIZE));

          yybegin(INITIAL);
        }
<INCLUDE><<EOF>> 
        {
            fprintf( stderr, "EOF in include" );
            yyterminate();
        }
<COMMENT><<EOF>> 
        {
            fprintf( stderr, "EOF in comment" );
            yyterminate();
        }
<<EOF>> {
          if ( include_count <= 0 ){
            yyterminate();
          } else {
            yy_delete_buffer(include_stack[include_count--] );
            yy_switch_to_buffer(include_stack[include_count] );
            yybegin(INCLUDE);
          }
        }
[a-z]+               yyecho();
.|\n                 yyecho();










