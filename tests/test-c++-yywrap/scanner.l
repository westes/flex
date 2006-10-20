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

#include "config.h"
#include <fstream>

%}

%option 8bit outfile="scanner.cpp" prefix="test"
%option nounput nomain
%option warn c++


%%

.              { }

%%

#define MAX_FILES 10

char *files[MAX_FILES] = { 0 };
int filecounter = 0;

int testFlexLexer::yywrap()
{
    if (filecounter-- > 0) {
        std::cout << "NOW WRAPPING TO READ " << files[filecounter] << std::endl;
        std::ifstream *ifs = new std::ifstream(files[filecounter]);
        switch_streams(ifs);
        return 0;
    }
    return 1;
}

int
main (int argc, char *argv[])
{
    for (int i = 1; i < argc && i <= MAX_FILES; i++) {
        files[i-1] = argv[i];
	filecounter++;
    }
    testFlexLexer* f = new testFlexLexer;
    f->yywrap();
    f->yylex();
    std::cout << "TEST RETURNING OK." << std::endl;
    return 0;
}
