%{
/* Builds "scanner.c". */
/* The scanner itself is a no-op. A successful compilation is all we want. */
#include <stdio.h>
#include <stdlib.h>
#include "config.h"

%}
%option reentrant
%option 8bit outfile="scanner.c" prefix="FOO"
%option nounput nomain noyywrap 
%option warn


%%


.|\n|\r  { 

    /* Compile, but do not execute the following code. */
    if( 0 ) {
        FOO_create_buffer( (FILE*)0, 0, yy_globals);
        FOO_delete_buffer( (YY_BUFFER_STATE)0, yy_globals);
        FOO_flush_buffer( (YY_BUFFER_STATE)0, yy_globals);
        FOO_init_buffer( (YY_BUFFER_STATE)0, (FILE*)0, yy_globals);
        FOO_load_buffer_state( yy_globals);
        FOO_scan_buffer( (char*)0, (yy_size_t)0, yy_globals);
        FOO_scan_bytes( (yyconst char*)0, 0, yy_globals);
        FOO_scan_string( (yyconst char*)0, yy_globals);
        FOO_switch_to_buffer( (YY_BUFFER_STATE)0, yy_globals);
        FOOrestart( (FILE*)0, (yyscan_t )0);

        FOOget_extra( (yyscan_t )0 );
        FOOget_in( (yyscan_t )0 );
        FOOget_leng( (yyscan_t )0 );
        FOOget_out( (yyscan_t )0 );
        FOOget_text( (yyscan_t )0 );
        FOOlex( (yyscan_t )0 );
        FOOlex_destroy( (yyscan_t )0 );
        FOOlex_init( (yyscan_t *)0 );
        FOOset_extra( (void *)0, (yyscan_t )0 );
        FOOset_in( (FILE*)0, (yyscan_t )0 );
        FOOset_out( (FILE*)0, (yyscan_t )0 );
    }
  }
%%

int
main ( int argc, char** argv )
{
    yyscan_t  scanner;
    FOOlex_init( &scanner);
    FOOlex( scanner);
    FOOlex_destroy( scanner);
    printf( "TEST RETURNING OK.\n");
    return 0;
}

