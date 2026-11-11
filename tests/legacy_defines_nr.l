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

%top{
#include <stdint.h>
/* Definitions a program made for flex 2.6.4. YY_READ_BUF_SIZE sets the
   amount read at a time. Defining FLEXINT_H made the scanner skip its
   own integer types, so the program supplies them; flex_int32_t is long
   here so that a second definition by the scanner would conflict. */
#define YY_READ_BUF_SIZE 1024
#define FLEXINT_H
typedef int8_t flex_int8_t;
typedef uint8_t flex_uint8_t;
typedef int16_t flex_int16_t;
typedef uint16_t flex_uint16_t;
typedef long flex_int32_t;
typedef unsigned long flex_uint32_t;
}

%{
#include <stdio.h>
#include "config.h"
%}

%option 8bit nomain nodefault noyywrap

%%

.|\n   { }

%%

#if YY_READ_BUF_SIZE != 1024
#error YY_READ_BUF_SIZE was redefined
#endif

int main(void);

int
main (void)
{
    yybuffer buffer = yy_scan_string("text");

    yylex();
    yy_delete_buffer(buffer);
    printf("TEST RETURNING OK.\n");
    return 0;
}
