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
/* Test that the legacy macros YY_NO_INPUT and YY_NO_UNPUT, defined in
   section 1 instead of using %option noinput nounput, still keep the
   scanner from defining yyinput() and yyunput_r(). If it defined them,
   the declarations in section 3 would conflict with the static
   functions and this test would not compile. */
#include <stdio.h>
#include <stdlib.h>
#include "config.h"

#define YY_NO_INPUT 1
#define YY_NO_UNPUT 1
%}

%option 8bit prefix="test"
%option nomain nodefault noyywrap
%option warn reentrant


%%

.|\n   { }

%%

extern int yyinput;
extern int yyunput_r;

int main(void);

int
main (void)
{
    yyscan_t scanner;
    yybuffer buffer;

    testlex_init(&scanner);
    buffer = test_scan_string("text", scanner);
    testlex(scanner);
    test_delete_buffer(buffer, scanner);
    testlex_destroy(scanner);

    printf("TEST RETURNING OK.\n");
    return 0;
}
