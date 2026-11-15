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
/* Test that the c99 back end passes a call whose text is longer than
   BUFSIZ through to the scanner whole when it adds the yyscanner
   argument. */
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
%}

%option emit="c99"
%option 8bit nomain nodefault noyywrap noinput nounput
%option warn

%%

x      {
    yypanic("this rule never matches: "
        "message line 000 of a call longer than BUFSIZ ........................"
        "message line 001 of a call longer than BUFSIZ ........................"
        "message line 002 of a call longer than BUFSIZ ........................"
        "message line 003 of a call longer than BUFSIZ ........................"
        "message line 004 of a call longer than BUFSIZ ........................"
        "message line 005 of a call longer than BUFSIZ ........................"
        "message line 006 of a call longer than BUFSIZ ........................"
        "message line 007 of a call longer than BUFSIZ ........................"
        "message line 008 of a call longer than BUFSIZ ........................"
        "message line 009 of a call longer than BUFSIZ ........................"
        "message line 010 of a call longer than BUFSIZ ........................"
        "message line 011 of a call longer than BUFSIZ ........................"
        "message line 012 of a call longer than BUFSIZ ........................"
        "message line 013 of a call longer than BUFSIZ ........................"
        "message line 014 of a call longer than BUFSIZ ........................"
        "message line 015 of a call longer than BUFSIZ ........................"
        "message line 016 of a call longer than BUFSIZ ........................"
        "message line 017 of a call longer than BUFSIZ ........................"
        "message line 018 of a call longer than BUFSIZ ........................"
        "message line 019 of a call longer than BUFSIZ ........................"
        "message line 020 of a call longer than BUFSIZ ........................"
        "message line 021 of a call longer than BUFSIZ ........................"
        "message line 022 of a call longer than BUFSIZ ........................"
        "message line 023 of a call longer than BUFSIZ ........................"
        "message line 024 of a call longer than BUFSIZ ........................"
        "message line 025 of a call longer than BUFSIZ ........................"
        "message line 026 of a call longer than BUFSIZ ........................"
        "message line 027 of a call longer than BUFSIZ ........................"
        "message line 028 of a call longer than BUFSIZ ........................"
        "message line 029 of a call longer than BUFSIZ ........................"
        "message line 030 of a call longer than BUFSIZ ........................"
        "message line 031 of a call longer than BUFSIZ ........................"
        "message line 032 of a call longer than BUFSIZ ........................"
        "message line 033 of a call longer than BUFSIZ ........................"
        "message line 034 of a call longer than BUFSIZ ........................"
        "message line 035 of a call longer than BUFSIZ ........................"
        "message line 036 of a call longer than BUFSIZ ........................"
        "message line 037 of a call longer than BUFSIZ ........................"
        "message line 038 of a call longer than BUFSIZ ........................"
        "message line 039 of a call longer than BUFSIZ ........................"
        "message line 040 of a call longer than BUFSIZ ........................"
        "message line 041 of a call longer than BUFSIZ ........................"
        "message line 042 of a call longer than BUFSIZ ........................"
        "message line 043 of a call longer than BUFSIZ ........................"
        "message line 044 of a call longer than BUFSIZ ........................"
        "message line 045 of a call longer than BUFSIZ ........................"
        "message line 046 of a call longer than BUFSIZ ........................"
        "message line 047 of a call longer than BUFSIZ ........................"
        "message line 048 of a call longer than BUFSIZ ........................"
        "message line 049 of a call longer than BUFSIZ ........................"
        "message line 050 of a call longer than BUFSIZ ........................"
        "message line 051 of a call longer than BUFSIZ ........................"
        "message line 052 of a call longer than BUFSIZ ........................"
        "message line 053 of a call longer than BUFSIZ ........................"
        "message line 054 of a call longer than BUFSIZ ........................"
        "message line 055 of a call longer than BUFSIZ ........................"
        "message line 056 of a call longer than BUFSIZ ........................"
        "message line 057 of a call longer than BUFSIZ ........................"
        "message line 058 of a call longer than BUFSIZ ........................"
        "message line 059 of a call longer than BUFSIZ ........................"
        "message line 060 of a call longer than BUFSIZ ........................"
        "message line 061 of a call longer than BUFSIZ ........................"
        "message line 062 of a call longer than BUFSIZ ........................"
        "message line 063 of a call longer than BUFSIZ ........................"
        "message line 064 of a call longer than BUFSIZ ........................"
        "message line 065 of a call longer than BUFSIZ ........................"
        "message line 066 of a call longer than BUFSIZ ........................"
        "message line 067 of a call longer than BUFSIZ ........................"
        "message line 068 of a call longer than BUFSIZ ........................"
        "message line 069 of a call longer than BUFSIZ ........................"
        "message line 070 of a call longer than BUFSIZ ........................"
        "message line 071 of a call longer than BUFSIZ ........................"
        "message line 072 of a call longer than BUFSIZ ........................"
        "message line 073 of a call longer than BUFSIZ ........................"
        "message line 074 of a call longer than BUFSIZ ........................"
        "message line 075 of a call longer than BUFSIZ ........................"
        "message line 076 of a call longer than BUFSIZ ........................"
        "message line 077 of a call longer than BUFSIZ ........................"
        "message line 078 of a call longer than BUFSIZ ........................"
        "message line 079 of a call longer than BUFSIZ ........................"
        "message line 080 of a call longer than BUFSIZ ........................"
        "message line 081 of a call longer than BUFSIZ ........................"
        "message line 082 of a call longer than BUFSIZ ........................"
        "message line 083 of a call longer than BUFSIZ ........................"
        "message line 084 of a call longer than BUFSIZ ........................"
        "message line 085 of a call longer than BUFSIZ ........................"
        "message line 086 of a call longer than BUFSIZ ........................"
        "message line 087 of a call longer than BUFSIZ ........................"
        "message line 088 of a call longer than BUFSIZ ........................"
        "message line 089 of a call longer than BUFSIZ ........................"
        "message line 090 of a call longer than BUFSIZ ........................"
        "message line 091 of a call longer than BUFSIZ ........................"
        "message line 092 of a call longer than BUFSIZ ........................"
        "message line 093 of a call longer than BUFSIZ ........................"
        "message line 094 of a call longer than BUFSIZ ........................"
        "message line 095 of a call longer than BUFSIZ ........................"
        "message line 096 of a call longer than BUFSIZ ........................"
        "message line 097 of a call longer than BUFSIZ ........................"
        "message line 098 of a call longer than BUFSIZ ........................"
        "message line 099 of a call longer than BUFSIZ ........................"
        "message line 100 of a call longer than BUFSIZ ........................"
        "message line 101 of a call longer than BUFSIZ ........................"
        "message line 102 of a call longer than BUFSIZ ........................"
        "message line 103 of a call longer than BUFSIZ ........................"
        "message line 104 of a call longer than BUFSIZ ........................"
        "message line 105 of a call longer than BUFSIZ ........................"
        "message line 106 of a call longer than BUFSIZ ........................"
        "message line 107 of a call longer than BUFSIZ ........................"
        "message line 108 of a call longer than BUFSIZ ........................"
        "message line 109 of a call longer than BUFSIZ ........................"
        "message line 110 of a call longer than BUFSIZ ........................"
        "message line 111 of a call longer than BUFSIZ ........................"
        "message line 112 of a call longer than BUFSIZ ........................"
        "message line 113 of a call longer than BUFSIZ ........................"
        "message line 114 of a call longer than BUFSIZ ........................"
        "message line 115 of a call longer than BUFSIZ ........................"
        "message line 116 of a call longer than BUFSIZ ........................"
        "message line 117 of a call longer than BUFSIZ ........................"
        "message line 118 of a call longer than BUFSIZ ........................"
        "message line 119 of a call longer than BUFSIZ ........................"
        "message line 120 of a call longer than BUFSIZ ........................"
        "message line 121 of a call longer than BUFSIZ ........................"
        "message line 122 of a call longer than BUFSIZ ........................"
        "message line 123 of a call longer than BUFSIZ ........................"
        "message line 124 of a call longer than BUFSIZ ........................"
        "message line 125 of a call longer than BUFSIZ ........................"
        "message line 126 of a call longer than BUFSIZ ........................"
        "message line 127 of a call longer than BUFSIZ ........................"
        "message line 128 of a call longer than BUFSIZ ........................"
        "message line 129 of a call longer than BUFSIZ ........................"
    );
    }
.|\n   { }

%%

int main(void);

int
main (void)
{
    yyscan_t scanner = NULL;
    yybuffer state;

    yylex_init(&scanner);
    state = yy_scan_string("abc\n", scanner);
    yylex(scanner);
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    printf("TEST RETURNING OK.\n");
    return 0;
}
