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
/* Test that a buffer created from a NULL FILE is filled through a
   user-defined YY_INPUT. Such a scanner never reads the FILE, so the
   NULL must not make the buffer look empty. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

static const char *input = "one two three";
static size_t input_pos = 0;

#define YY_INPUT(buf, result, max_size)                         \
    do {                                                        \
        size_t n = strlen (input + input_pos);                  \
        if (n > (size_t) (max_size))                            \
            n = (size_t) (max_size);                            \
        memcpy (buf, input + input_pos, n);                     \
        input_pos += n;                                         \
        result = (int) n;                                       \
    } while (0)

static int words = 0;
%}

%option 8bit prefix="test"
%option nounput nomain nodefault noyywrap noinput
%option warn reentrant


%%

[[:space:]]+   { }
[[:alpha:]]+   { ++words; }
.              {
    fprintf(stderr,"*** Error: Unrecognized character '%c' while scanning.\n",
         yytext[0]);
    yyterminate();
    }

%%

int main(void);

int
main (void)
{
    yyscan_t scanner;
    yybuffer buffer;

    testlex_init(&scanner);
    buffer = test_create_buffer(NULL, 1024, scanner);
    test_switch_to_buffer(buffer, scanner);
    testlex(scanner);
    test_delete_buffer(buffer, scanner);
    testlex_destroy(scanner);

    if (words != 3) {
        fprintf(stderr, "*** Error: scanned %d words, expected 3.\n", words);
        return 1;
    }
    printf("TEST RETURNING OK.\n");
    return 0;
}
