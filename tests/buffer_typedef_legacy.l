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
/* Test that a program which declares YY_BUFFER_STATE itself, guarded by
   YY_TYPEDEF_YY_BUFFER_STATE as flex 2.6.4 allowed, still gets the
   yybuffer type that the scanner uses. PostgreSQL does this. */
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
%}

%top{
#ifndef YY_TYPEDEF_YY_BUFFER_STATE
#define YY_TYPEDEF_YY_BUFFER_STATE
typedef struct yy_buffer_state *YY_BUFFER_STATE;
#endif
}

%option 8bit prefix="test"
%option nomain nodefault noyywrap
%option warn


%%

.|\n   { }

%%

int main(void);

int
main (void)
{
    YY_BUFFER_STATE old_name;
    yybuffer new_name;

    old_name = new_name = test_scan_string("text");
    (void) old_name;
    testlex();
    test_delete_buffer(new_name);

    printf("TEST RETURNING OK.\n");
    return 0;
}
