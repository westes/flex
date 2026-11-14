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
/* Test that a C++ scanner keeps the flex 2.6.4 yyunput(int, char*) member,
   which a subclass or a bundled FlexLexer.h may use or declare. */
#include <iostream>
#include <sstream>
#include "config.h"

static int saw_b = 0;
%}

%option 8bit prefix="test"
%option warn c++
%option nomain noinput noyywrap

%%

a              { unput('b'); }
b              { saw_b = 1; }
.|\n           { }

%%

class MyLexer : public yyFlexLexer {
public:
    MyLexer(std::istream* in, std::ostream* out) : yyFlexLexer(in, out) {}
    void push_back(int c) { yyunput(c, yytext); }
};

int main(void);

int
main (void)
{
    std::istringstream in("a");
    std::ostringstream out;
    MyLexer f(&in, &out);

    f.yylex();
    if (!saw_b) {
        std::cout << "TEST FAILED: the pushed-back character was not read." << std::endl;
        return 1;
    }
    std::cout << "TEST RETURNING OK." << std::endl;
    return 0;
}
