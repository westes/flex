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
/* A file to build "scanner.c". */
/* This tests that we can use "yyextra". 
   We buffer all input into a growable array, then print it.
   We run diff on the input and output.
*/

#include <stdio.h>
#include <stdlib.h>
#include "config.h"


/* We'll store the entire input in this buffer, growing as necessary. */
struct Buffer {
    int  curr_len;
    int  max_len;
    int  grow_len;
    char * data;
};
#define YY_EXTRA_TYPE struct Buffer *

/* Save char into junk array at next position. */
static void append_char (char c,  yyscan_t  scanner );

%}

%option 8bit outfile="scanner.c" prefix="test"
%option nounput nomain noyywrap nodefault
%option warn
%option reentrant


%%

.|\r|\n  { append_char (yytext[0],yyscanner); }

%%

int main(void);

int
main ()
{   
    yyscan_t  scanner;
    struct Buffer * buf;
    int i;

    buf = (struct Buffer*) malloc(sizeof(struct Buffer));
    buf->curr_len =0;
    buf->max_len = 4; 
    buf->grow_len = 100;
    buf->data = (char*)malloc(buf->max_len);
 
    testlex_init(&scanner);
    testset_in( stdin, scanner);
    testset_out( stdout, scanner);
    testset_extra( buf, scanner );
    testlex(scanner);

        buf = testget_extra(scanner);
        for(i=0; i < buf->curr_len; i++)
            fputc( buf->data[i], stdout );
        free( buf->data);
        free( buf);

    testlex_destroy(scanner);
    return 0;
}

/* Save char into junk array at next position. */
static void append_char (char c,  yyscan_t  scanner )
{
    struct Buffer *buf, *new_buf;
    buf = testget_extra(scanner);

    /* Grow buffer if necessary. */

    if( buf->curr_len >= buf->max_len )
    {
        new_buf = (struct Buffer*) malloc(sizeof(struct Buffer));
        new_buf->max_len = buf->max_len + buf->grow_len;
        new_buf->grow_len = buf->grow_len;
        new_buf->data = (char*)malloc(new_buf->max_len);
        for( new_buf->curr_len = 0;
             new_buf->curr_len < buf->curr_len;
             new_buf->curr_len++ )
        {
            new_buf->data[ new_buf->curr_len] = buf->data [ new_buf->curr_len];
        }
        free( buf->data );
        free( buf );
        buf = new_buf;
        testset_extra( buf, scanner );
    }

    
    buf->data[ buf->curr_len++ ] = c;
}
