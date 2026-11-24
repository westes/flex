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

#include <iostream>

#include "cxx_c_buffer_state_c.h"

#undef yyFlexLexer
#define yyFlexLexer Cpp_FlexLexer
#include <FlexLexer.h>

/* Overload resolution picks the first form only when both arguments
   have the same type. */
template <class T> char (&same_type (T, T))[1];
template <class T, class U> char (&same_type (T, U))[2];

int
main (void)
{
    Cpp_FlexLexer lexer;
    yy_buffer_state *buffer = lexer.yy_create_buffer (std::cin, 16384);
    lexer.yy_delete_buffer (buffer);

    /* The C scanner's buffer type must differ from the C++ scanner's.
       If they were the same struct, the program would contain two
       definitions of it. The calls are only named, never run. */
    if (sizeof (same_type (C__create_buffer (stdin, 0), buffer)) == 1)
        return 1;

    int result = 0;
    result += lexer.yylex ();
    result += C_lex ();
    result += C_lex_destroy ();

    if (result != 0)
        return 1;

    std::cout << "TEST RETURNING OK." << std::endl;
    return 0;
}
