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

#include <stdio.h>

#undef yyFlexLexer
#define yyFlexLexer S1_FlexLexer
#include <FlexLexer.h>

#undef yyFlexLexer
#define yyFlexLexer S2_FlexLexer
#include <FlexLexer.h>

int
main ( int argc, char** argv )
{
    int S1_ok=1, S2_ok=1;
    S1_FlexLexer* S1 = new S1_FlexLexer;
    S2_FlexLexer* S2 = new S2_FlexLexer;

    // scan simultaneously. 
    while(S1_ok || S2_ok)
    {
        if (S1_ok)
            S1_ok = S1->yylex();
        if (S2_ok)
            S2_ok = S2->yylex();
    }
    printf("TEST RETURNING OK.\n");
    delete S1;
    delete S2;
    return 0;
}


/* vim:set tabstop=8 softtabstop=4 shiftwidth=4: */
