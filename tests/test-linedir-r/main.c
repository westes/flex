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

#include "scanner.h"

int
main ( int argc, char** argv )
{
    yyscan_t  scanner;
    FILE *fp;
    char * extra = "EXTRA";
    
    testlex_init(&scanner);
    testset_in(stdin,scanner);
    testset_out(stdout,scanner);    
    testset_extra(extra,scanner);
    
    fp = testget_in(scanner);
    fp = testget_out(scanner);

    while(testlex(scanner)) {
        char * text;
        int line;
        line = testget_lineno(scanner);
        text = testget_text(scanner);
        
        if( (char*)testget_extra(scanner) != extra)
            break;
        
        if ( !text || line < 0)
            continue;
    }
    testlex_destroy(scanner);
    return 0;
}


/* vim:set tabstop=8 softtabstop=4 shiftwidth=4: */
