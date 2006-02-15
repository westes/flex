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
%}

%option 8bit outfile="scanner.c" prefix="test"
%option nounput nomain noyywrap
%option warn stack never-interactive
%x STATE_1

%%

<INITIAL>{
0              yy_push_state (STATE_1);
.|\n           return 1;
}
<STATE_1>{
1              yy_pop_state();
.|\n           return yy_top_state() + 1;
}

%%

int
main (int argc, char* const argv[])
{
    FILE* fp;
    int i;

    if ((fp = fopen(argv[1],"r")) == NULL){
        perror("Failed to open input file.");
        return 1;
    }

    yyset_out ( stdout);

    for (i=0; i <  4; ++i){
        rewind(fp);
        yyset_in  ( fp);
        while( yylex() )
            ;
        yylex_destroy();
    }
    printf("TEST RETURNING OK.\n");
    return 0;
}
