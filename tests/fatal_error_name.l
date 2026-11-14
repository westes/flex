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
/* Test that a program can use yy_fatal_error(), the flex 2.6.4 name of
   yypanic(), from its own code, including in the first section, as gpc
   does to avoid an unused-function warning. */
#include <stdio.h>
#include <stdlib.h>
#include "config.h"

static void mention(void) { (void) yy_fatal_error; }
%}

%option 8bit nomain nodefault noyywrap

%%

.|\n   { }

%%

static void give_up(const char *msg)
{
    yy_fatal_error(msg);
}

int main(void);

int
main (void)
{
    if (0) {
        mention();
        give_up("not reached");
    }
    printf("TEST RETURNING OK.\n");
    return 0;
}
