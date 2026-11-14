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
/* Test that YY_BUF_SIZE and YY_READ_BUF_SIZE can be used in the first
   section, as with flex 2.6.4, where they were defined before it. */
#include <stdio.h>
#include <stdlib.h>
#include "config.h"

static size_t buffer_size(void) { return YY_BUF_SIZE; }
static size_t read_size(void) { return YY_READ_BUF_SIZE; }
%}

%option 8bit nomain nodefault noyywrap

%%

.|\n   { }

%%

int main(void);

int
main (void)
{
    if (buffer_size() < read_size() || read_size() == 0) {
        printf("TEST FAILED: odd buffer sizes.\n");
        return 1;
    }
    printf("TEST RETURNING OK.\n");
    return 0;
}
