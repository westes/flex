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
/* The c99 version of scan_length.direct. Here the fatal error handler is a
   function, yypanic(), so the scanner is told not to generate its own and
   calls to it are redirected to the handler in scan_length.h.
 */
#include "scan_length.h"

#define yypanic(msg, yyscanner) scan_length_panic(msg)

static int count;
%}

%option emit="c99"
%option 8bit prefix="test"
%option nounput nomain nodefault noyywrap noinput noyypanic
%option warn

%%

.|\n      { ++count; }

%%

int
main ( void )
{
    int failed = 0;
    int returned;
    char *huge;
    yyscan_t scanner = NULL;

    testlex_init(&scanner);

    /* A negative length. */
    returned = 0;
    if ( setjmp(fatal_env) == 0 ) {
        test_scan_bytes("abc", -1, scanner);
        returned = 1;
    }
    failed |= check_fatal("test_scan_bytes(-1)",
                          "negative length in yy_scan_bytes()", returned);

    /* Ordinary input still scans, through both entry points. */
    test_scan_bytes("abc", 3, scanner);
    testlex(scanner);
    test_scan_string("de", scanner);
    testlex(scanner);
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
            test_scan_string(huge, scanner);
            returned = 1;
        }
        failed |= check_fatal("test_scan_string(4 GiB - 1)",
                              "string too long for yy_scan_string()", returned);
    }

    testlex_destroy(scanner);
    if ( failed ) {
        return 1;
    }
    printf("TEST RETURNING OK.\n");
    return 0;
}
