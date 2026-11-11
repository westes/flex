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
#ifndef YY_TYPEDEF_YY_BUFFER_STATE
#define YY_TYPEDEF_YY_BUFFER_STATE
typedef struct yy_buffer_state *YY_BUFFER_STATE;
#endif
}

%{
/* Test that a C++ scanner still gets yybuffer when the program declares
   YY_BUFFER_STATE itself, guarded by YY_TYPEDEF_YY_BUFFER_STATE. */
#include <iostream>
#include <sstream>
#include "config.h"
%}

%option 8bit prefix="test"
%option warn c++
%option nounput nomain noinput noyywrap

%%

.|\n           { }

%%

int main(void);

int
main (void)
{
    std::istringstream in("text");
    yyFlexLexer f;
    YY_BUFFER_STATE old_name;
    yybuffer new_name;

    f.switch_streams(&in, &std::cout);
    new_name = f.yy_create_buffer(&in, 16);
    old_name = new_name;
    (void) old_name;
    f.yy_delete_buffer(new_name);
    std::cout << "TEST RETURNING OK." << std::endl;
    return 0;
}
