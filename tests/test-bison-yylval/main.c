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

#include "parser.h"
#include "scanner.h"

extern int testparse(yyscan_t);

int main ( int argc, char** argv )
{
    yyscan_t scanner;
    /*yydebug =1;*/
    testlex_init ( &scanner );
    testset_in(stdin,scanner);
    testparse ( scanner );
    testlex_destroy ( scanner );
    return 0;
}


/* vim:set tabstop=8 softtabstop=4 shiftwidth=4: */
