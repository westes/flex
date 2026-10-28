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
/* A scanner that uses REJECT, so the scanner keeps a per-character state
   buffer. yyrestart() and yy_switch_to_buffer() grow that state buffer to
   match the active input buffer, but yypush_buffer_state() used not to. This
   test pushes a buffer larger than the default YY_BUF_SIZE and then scans a
   long token, which overflowed the under-sized state buffer. It is the
   yypush_buffer_state() sibling of the CVE-2006-0459 case in state_buf.direct.
 */
#include <stdio.h>
#include <stdlib.h>
%}

%option nounput nomain noyywrap noinput

%%

a+        REJECT;
.|\n      { }

%%

int
main ( int argc, char** argv )
{
    FILE *fp;

    if ( argc != 2 ) {
        fprintf(stderr, "*** Error: Must specify one filename.\n");
        exit(-1);
    }

    /* First scan on a small buffer. This allocates the reject state buffer
     * at YY_BUF_SIZE. It reads a string, because the default input is
     * stdin and the test must not wait on it.
     */
    yy_scan_string( "aaa" );
    yylex();

    /* Now push a buffer much larger than YY_BUF_SIZE and scan a long token
     * out of it. Before the fix the state buffer was left at its old size.
     */
    if ( (fp = fopen(argv[1], "r")) == NULL ) {
        fprintf(stderr, "*** Error: fopen(%s) failed.\n", argv[1]);
        exit(-1);
    }
    yypush_buffer_state( yy_create_buffer( fp, 1 << 20 ) );
    yylex();

    fclose(fp);
    printf("TEST RETURNING OK.\n");
    return 0;
}
