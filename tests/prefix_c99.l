/*
 * This file is part of flex.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * Neither the name of the University nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE.
 */

%{
/* Builds "scanner.c". */
/* The scanner itself is a no-op. A successful compilation is all we want. */
#include <stdio.h>
#include <stdlib.h>
#include "config.h"

%}
%option emit="c99"
%option 8bit prefix="FOO"
%option nounput nomain noyywrap noinput
%option warn


%%


.|\n|\r  { 

    /* Compile, but do not execute the following code. */
    if( 0 ) {
        FOO_create_buffer( (FILE*)0, 0, yyscanner);
        FOO_delete_buffer( (yybuffer)0, yyscanner);
        FOO_flush_buffer( (yybuffer)0, yyscanner);
        FOO_init_buffer( (yybuffer)0, (FILE*)0, yyscanner);
        FOO_load_buffer_state( yyscanner);
        FOO_scan_buffer( (char*)0, (size_t)0, yyscanner);
        FOO_scan_bytes( (const char*)0, 0, yyscanner);
        FOO_scan_string( (const char*)0, yyscanner);
        FOO_switch_to_buffer( (yybuffer)0, yyscanner);
        FOOrestart( (FILE*)0, (yyscan_t )0);

	/* Commented out because the C99 back end is more
	 * careful about not compiling in things it doesn't
	 * need.
	 */
        //FOOget_extra( (yyscan_t )0 );
        FOOget_in( (yyscan_t )0 );
        FOOget_leng( (yyscan_t )0 );
        FOOget_out( (yyscan_t )0 );
        FOOget_text( (yyscan_t )0 );
        FOOlex( (yyscan_t )0 );
        //FOOlex_destroy( (yyscan_t )0 );
        //FOOlex_init( (yyscan_t *)0 );
        //FOOset_extra( (void *)0, (yyscan_t )0 );
        FOOset_in( (FILE*)0, (yyscan_t )0 );
        FOOset_out( (FILE*)0, (yyscan_t )0 );
    }
  }
%%

int main(void);

int
main (void)
{
    yyscan_t  scanner;
    FOOlex_init( &scanner);
    FOOlex( scanner);
    FOOlex_destroy( scanner);
    printf( "TEST RETURNING OK.\n");
    return 0;
}

