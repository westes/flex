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

%option 8bit outfile="scanner.c" prefix="FOO"
%option nounput nomain noyywrap 
%option warn


%%

.|\n|\r  {

    /* Compile, but do not execute the following code. */
    if( 0) {
        FOO_create_buffer((FILE*)0,0);
        FOO_delete_buffer((YY_BUFFER_STATE)0);
        FOO_flush_buffer((YY_BUFFER_STATE)0);
        FOO_init_buffer((YY_BUFFER_STATE)0,(FILE*)0);
        FOO_load_buffer_state();
        FOO_scan_buffer((char*)0,(yy_size_t)0);
        FOO_scan_bytes((yyconst char*)0, 0);
        FOO_scan_string((yyconst char*)0);
        FOO_switch_to_buffer((YY_BUFFER_STATE)0);
        yyin   = (FILE*)0;
        yyout  = (FILE*)0;
        yyleng = 0;
        yylex();
        yyrestart((FILE*)0);
        yytext = (char*)0;
    }
 }
%%


int main(void);

int
main ()
{
    yyin = stdin;
    yyout = stdout;
    FOOlex();
    printf("TEST RETURNING OK.\n");
    return 0;
}

