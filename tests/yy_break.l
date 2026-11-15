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
/* Test that a program can define YY_BREAK, the code that ends each rule's
   action, as it could with flex 2.6.4. The program counts the rules the
   scanner matches by counting how often the code runs. */
#include <stdio.h>
#include <stdlib.h>
#include "config.h"

static int breaks;

#define YY_BREAK breaks++; break;
%}

%option 8bit nomain nodefault noyywrap

%%

a      { }
.|\n   { }

%%

int main(void);

int
main (void)
{
    yyin = tmpfile();
    if (yyin == NULL) {
        printf("TEST FAILED: no temporary file.\n");
        return 1;
    }
    fputs("aab", yyin);
    rewind(yyin);
    yylex();
    fclose(yyin);
    if (breaks != 3) {
        printf("TEST FAILED: YY_BREAK ran %d times, not 3.\n", breaks);
        return 1;
    }
    printf("TEST RETURNING OK.\n");
    return 0;
}
