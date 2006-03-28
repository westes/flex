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
/* A template scanner file to build "scanner.c". */
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
/*#include "parser.h" */

#define err_abort() do{printf("ERROR: flex line %d. input line %d.\n", __LINE__, yylineno); abort();} while(0)
#define a_ok()      do{printf("OK: flex line %d. input line %d.\n", __LINE__, yylineno); return 1;}while(0)
%}

%option 8bit outfile="scanner.c" prefix="test"
%option nounput nomain noyywrap
%option warn


%%

^"^alpha:"[[:^alpha:]]+@alpha@\n        printf("OK: %s", yytext); ++yylineno; return 1;
^"^digit:"[[:^digit:]]+@digit@\n        printf("OK: %s", yytext); ++yylineno; return 1;
^"^alnum:"[[:^alnum:]]+@alnum@\n        printf("OK: %s", yytext); ++yylineno; return 1;
^"^upper:"[[:^upper:]]+@upper@\n        printf("OK: %s", yytext); ++yylineno; return 1;
^"^lower:"[[:^lower:]]+@lower@\n        printf("OK: %s", yytext); ++yylineno; return 1;
^"^space:"[[:^space:]]+@space@\n        printf("OK: %s", yytext); ++yylineno; return 1;
^"^blank:"[[:^blank:]]+@blank@\n        printf("OK: %s", yytext); ++yylineno; return 1;
^"^punct:"[[:^punct:]]+@punct@\n        printf("OK: %s", yytext); ++yylineno; return 1;
^"^cntrl:"[[:^cntrl:]]+@cntrl@\n        printf("OK: %s", yytext); ++yylineno; return 1;
^"^xdigit:"[[:^xdigit:]]+@xdigit@\n      printf("OK: %s", yytext); ++yylineno; return 1;

^"a-d:"[[:alpha:]]{-}[[:digit:]]+@a-d@\n  printf("OK: %s", yytext); ++yylineno; return 1;
^"l-xyz:"([[:lower:]]{-}[xyz])+@l-xyz@\n    printf("OK: %s", yytext); ++yylineno; return 1;
^"abcd-bc:"([abcd]{-}[bc])+@abcd-bc@\n          printf("OK: %s", yytext); ++yylineno; return 1;
^"abcde-b-c:"([abcde]{-}[b]{-}[c])+@abcde-b-c@\n    printf("OK: %s", yytext); ++yylineno; return 1;
^"^XY-^XYZ:"([^XY]{-}[^XYZ])+@^XY-^XYZ@\n    printf("OK: %s", yytext); ++yylineno; return 1;

^"a+d:"([[:alpha:]]{+}[[:digit:]])+"@a+d@"\n    a_ok();
^"a-u+Q:"([[:alpha:]]{-}[[:upper:]]{+}[Q])+"@a-u+Q@"\n    a_ok();

^"ia:"(?i:a)+@ia@\n                          printf("OK: %s", yytext); ++yylineno; return 1;
^"iabc:"(?i:abc)+@iabc@\n                    printf("OK: %s", yytext); ++yylineno; return 1;
^"ia-c:"(?i:[a-c]+)@ia-c@\n                             printf("OK: %s", yytext); ++yylineno; return 1;

    /* We don't want this one to match. */
^"check-a:"(?i:(?-i:A))@\n               err_abort();
^"check-a:"(?i:(?-i:(?i:A)))@\n          printf("OK: %s", yytext); ++yylineno; return 1;

    /* We don't want this one to match. */
^"dot-all-1:"(?-s:XXX.*)@dot-all-1@\n    err_abort(); 
^"dot-all-1:"(?s:XXX.*)@dot-all-1@\n    a_ok();

^"x1:"(?x:   a | b  )+@x1@\n              a_ok();
^"x2:"(?x:   a |
        (?# Comment )
    b
    )+@x2@\n              a_ok();


.|\n                       { err_abort(); }
%%

int main(void);

int
main ()
{
    yyin = stdin;
    yyout = stdout;
    while (yylex())
        ;
    printf("TEST RETURNING OK.\n");
    return 0;
}
