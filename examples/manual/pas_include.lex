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

"{"                          BEGIN(COMMENT);

<COMMENT>"}"                 BEGIN(INITIAL); 
<COMMENT>"$include"[ \t]*"(" BEGIN(INCLUDE);
<COMMENT>[ \t]*              /* skip whitespace */

<INCLUDE>")"                 BEGIN(COMMENT); 
<INCLUDE>[ \t]*              /* skip whitespace */
<INCLUDE>[^ \t\n() ]+ {      /* get the include file name */
          if ( include_count >= MAX_NEST){
             fprintf( stderr, "Too many include files" );
             exit( 1 );
          }

          include_stack[++include_count] = YY_CURRENT_BUFFER;

          yyin = fopen( yytext, "r" );
          if ( ! yyin ){
             fprintf( stderr, "Unable to open %s",yytext);
             exit( 1 );
          }

          yy_switch_to_buffer(yy_create_buffer(yyin,YY_BUF_SIZE));

          BEGIN(INITIAL);
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
            BEGIN(INCLUDE);
          }
        }
[a-z]+               ECHO;
.|\n                 ECHO;










