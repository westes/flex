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
/* yy_scan_bytes() and yy_scan_string() size their copy of the input from an
   int. A negative length, or a string too long for an int, used to make
   yy_scan_bytes() allocate a tiny buffer and write the two end-of-buffer
   characters outside it. Both now stop with a fatal error. This test
   replaces the fatal error handler with one that returns to main(), so it
   can check which error was reported and that ordinary input still scans.

   The string of 4294967295 bytes (which an int sees as -1) is built by
   mapping the same 1 MiB of 'a' over 4 GiB of address space, so it uses
   about 1 MiB of memory. A system without mmap() or a 64-bit size_t skips
   that part.
 */
#include "scan_length.h"

static int count;

#define YY_FATAL_ERROR(msg) scan_length_panic(msg)
%}

%option nounput nomain noyywrap noinput

%%

.|\n      { ++count; }

%%

int
main ( int argc, char** argv )
{
    int failed = 0;
    int returned;
    char *huge;

    (void) argc;
    (void) argv;

    /* A negative length. */
    returned = 0;
    if ( setjmp(fatal_env) == 0 ) {
        yy_scan_bytes("abc", -1);
        returned = 1;
    }
    failed |= check_fatal("yy_scan_bytes(-1)",
                          "negative length in yy_scan_bytes()", returned);

    /* Ordinary input still scans, through both entry points. */
    yy_scan_bytes("abc", 3);
    yylex();
    yy_scan_string("de");
    yylex();
    if ( count != 5 ) {
        fprintf(stderr, "*** Error: scanned %d characters, expected 5.\n",
                count);
        failed = 1;
    }

    /* A string too long for an int. */
    if ( (huge = map_long_string()) == NULL ) {
        printf("Skipping the long string: this system cannot map one.\n");
    } else {
        fatal_msg = NULL;
        returned = 0;
        if ( setjmp(fatal_env) == 0 ) {
            yy_scan_string(huge);
            returned = 1;
        }
        failed |= check_fatal("yy_scan_string(4 GiB - 1)",
                              "string too long for yy_scan_string()", returned);
    }

    if ( failed ) {
        return 1;
    }
    printf("TEST RETURNING OK.\n");
    return 0;
}
