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
/* Test that the buffer's yy_at_bol member, which programs set to make ^
   match, keeps its flex 2.6.4 name. */
#include <stdio.h>
#include <stdlib.h>
#include "config.h"

static int matched = 0;
%}

%option 8bit nomain nodefault noyywrap

%%

^a     { matched = 1; }
.|\n   { }

%%

int main(void);

int
main (void)
{
    yybuffer buffer = yy_scan_string("a");

    YY_CURRENT_BUFFER->yy_at_bol = 1;
    yylex();
    yy_delete_buffer(buffer);
    if (!matched) {
        printf("TEST FAILED: ^ did not match.\n");
        return 1;
    }
    printf("TEST RETURNING OK.\n");
    return 0;
}
