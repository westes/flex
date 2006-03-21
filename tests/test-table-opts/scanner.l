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
%}

%option 8bit
%option nounput nomain noyywrap 
%option warn yylineno



%%

foo|bar         ;
[[:digit:]]+    ;
[[:blank:]]+    ;
.|\n            ;
%%

int main ( int argc, char** argv )
{
    FILE* fp = NULL;
    void *yyscanner=0;
    M4_YY_DECL_GUTS_VAR();
    
#ifdef TEST_IS_REENTRANT
    yylex_init(&yyscanner);
#endif

#ifdef TEST_HAS_TABLES_EXTERNAL
    if((fp  = fopen(argv[1],"r"))== NULL)
        yy_fatal_error("could not open tables file for reading" M4_YY_CALL_LAST_ARG);

    if(yytables_fload(fp M4_YY_CALL_LAST_ARG) < 0)
        yy_fatal_error("yytables_fload returned < 0" M4_YY_CALL_LAST_ARG);
    if(M4_YY_TABLES_VERIFY)
        exit(0);
#endif
    
    if(argc > 2){
        if((fp  = fopen(argv[2],"r"))== NULL)
            yy_fatal_error("could not open input file for reading" M4_YY_CALL_LAST_ARG);
        yyin = fp;
    }
    while(yylex(M4_YY_CALL_ONLY_ARG) != 0)
        ;
        
#ifdef TEST_HAS_TABLES_EXTERNAL
    yytables_destroy(M4_YY_CALL_ONLY_ARG);
#endif
    yylex_destroy(M4_YY_CALL_ONLY_ARG);

    if(argc < 0) /* silence the compiler */
        yyscanner = (void*)fp;

    return 0;
}
