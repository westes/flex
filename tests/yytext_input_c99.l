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
/* Test, in the c99 back end, what yytext holds after the program has called yyinput(yyscanner). As with
   flex 2.6.4, yytext as a string ends where the token ends, however many
   characters yyinput(yyscanner) has read since, and yyleng does not change. Also check
   that yyless() after yyinput(yyscanner) gives back the right text and leaves a buffer
   owned by the program as it was, and that all this still holds when yyinput(yyscanner)
   has to read more of the file. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

static int errors;
static char log_text[64];
static size_t long_comment_length;

static void
expect(const char *what, const char *got, const char *wanted)
{
    if (strcmp(got, wanted) != 0) {
        printf("TEST FAILED: %s was \"%s\", not \"%s\".\n", what, got, wanted);
        errors++;
    }
}

static void
note(const char *text)
{
    strcat(log_text, text);
    strcat(log_text, "|");
}
%}

%option emit="c99"
%option 8bit nomain noyywrap nounput nodefault

%%

"/*"   { int c1 = yyinput(yyscanner);
         int c2 = yyinput(yyscanner);
         size_t length = 2;

         expect("yytext after two reads", yytext, "/*");
         if (yyleng != 2) {
             printf("TEST FAILED: yyleng was %d, not 2.\n", yyleng);
             errors++;
         }
         while (c2 != 0 && !(c1 == '*' && c2 == '/')) {
             c1 = c2;
             c2 = yyinput(yyscanner);
             length++;
         }
         long_comment_length = length;
         expect("yytext after the whole comment", yytext, "/*");
         note(yytext); }
ab     { yyinput(yyscanner);
         yyinput(yyscanner);
         yyless(1, yyscanner);
         note(yytext); }
.|\n   { note(yytext); }

%%

int main(void);

static int
try_buffer(const char *text, const char *wanted)
{
    char buf[16];
    yyscan_t scanner;
    yybuffer b;

    memset(buf, 0, sizeof buf);
    strcpy(buf, text);
    log_text[0] = '\0';
    yylex_init(&scanner);
    b = yy_scan_buffer(buf, strlen(text) + 2, scanner);
    yylex(scanner);
    yy_delete_buffer(b, scanner);
    yylex_destroy(scanner);
    if (strcmp(buf, text) != 0) {
        printf("TEST FAILED: %s came back as %s.\n", text, buf);
        errors++;
    }
    expect(text, log_text, wanted);
    return 0;
}

static int
try_file(void)
{
    enum { body = 40000 };
    FILE *f = tmpfile();
    yyscan_t scanner;
    int i;

    if (f == NULL) {
        printf("TEST FAILED: no temporary file.\n");
        return 1;
    }
    fputs("/**", f);
    for (i = 0; i < body; i++)
        fputc(i % 50 == 49 ? '\n' : 'x', f);
    fputs("*/ y", f);
    rewind(f);
    log_text[0] = '\0';
    yylex_init(&scanner);
    yyrestart(f, scanner);
    yylex(scanner);
    yylex_destroy(scanner);
    fclose(f);
    expect("text after a comment longer than the buffer", log_text, "/*| |y|");
    if (long_comment_length != body + 3) {
        printf("TEST FAILED: input() read %lu characters of the comment, not %d.\n",
               (unsigned long) long_comment_length, body + 3);
        errors++;
    }
    return 0;
}

int
main (void)
{
    try_buffer("abcd", "a|b|c|d|");
    try_buffer("/**\n*/", "/*|");
    try_file();
    if (errors)
        return 1;
    printf("TEST RETURNING OK.\n");
    return 0;
}
