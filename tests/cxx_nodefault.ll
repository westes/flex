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
/* Test that a C++ scanner with no default rule compiles, and that
   input no rule matches reaches LexerError() as it did in flex 2.6.4. */
#include <iostream>
#include <sstream>
#include "config.h"

static int saw_error = 0;
%}

%option 8bit prefix="test"
%option warn c++
%option nomain nodefault noinput nounput noyywrap

%%

a              { }
\n             { }

%%

class MyLexer : public yyFlexLexer {
public:
    MyLexer(std::istream* in, std::ostream* out) : yyFlexLexer(in, out) {}
    void LexerError(const char*) override { saw_error = 1; throw 1; }
};

int main(void);

int
main (void)
{
    std::istringstream in("ab");
    std::ostringstream out;
    MyLexer f(&in, &out);

    try {
        f.yylex();
    } catch (int) {
    }
    if (!saw_error) {
        std::cout << "TEST FAILED: LexerError was not called." << std::endl;
        return 1;
    }
    std::cout << "TEST RETURNING OK." << std::endl;
    return 0;
}
