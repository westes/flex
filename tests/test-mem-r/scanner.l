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
/* A template scanner file to build "scanner.c".
 * The whole idea is to cause memory realloc by
 *  1. pushing a lot on the condition stack, and
 *  2. eating input greater than YY_BUF_SIZE
 */
#include <stdio.h>
#include <stdlib.h>
#include "config.h"

/* Insanely small read buffer. This pretty much guarantees at least one realloc. */
#ifdef YY_BUF_SIZE
#undef YY_BUF_SIZE
#endif
#define YY_BUF_SIZE 8

%}

%option 8bit outfile="scanner.c" prefix="test"
%option nounput nomain noyywrap
%option warn stack nodefault reentrant
%option noyyalloc noyyrealloc noyyfree

%x parens

%%

<INITIAL>{
"("         { printf("yy_push_state(parens)\n"); yy_push_state(parens,yyscanner); }
len=[0-9]+  { printf("About read token where %s\n",yytext); }
0+          { }
.|\n        { }
}

<parens>{
"("         { printf("yy_push_state(parens)\n"); yy_push_state(parens,yyscanner); }
")"         { printf("yy_pop_state()\n");yy_pop_state(yyscanner);}
[^()\n]+     { }
.|\n        { }
}

%%
/* total memory allocated */
static size_t total_mem=0;

/* track the amount of memory for ptr. */
struct memsz {
    void* p;
    size_t sz;
};

static struct memsz * ptrs=0;  /* Array of pairs. */
static int nptrs=0; /* Number of pairs in array. */
static int arrsz=0; /* Capacity of array. */

static void dump_mem(FILE* fp){
    int i;
    fprintf(fp,"\tptrs[%d] = {", nptrs);
    for (i=0; i < arrsz; i++)
        fprintf(fp," {%#lx,%ld},", (long)ptrs[i].p, (long)ptrs[i].sz);
    
    fprintf(fp,"}\n");
}

void * yyalloc(yy_size_t n , void* yyscanner)
{
    void * p;
    int i;

    total_mem += n;
    p = (void*)malloc(n);

    if( nptrs >= arrsz){
        /* increase array size by 1 */
        arrsz++;
        ptrs = (struct memsz*)realloc( ptrs, arrsz * sizeof(struct memsz));
        ptrs[nptrs].p = 0;
        ptrs[nptrs].sz = 0;
    }

    /* find a null slot */
    for(i=0; i < arrsz ; i++)
        if (ptrs[i].p == 0) {
            ptrs[i].p = p;
            ptrs[i].sz = n;
        }

    nptrs++;
    printf("yyflex_alloc(%8ld) total=%8ld return=%#10lx\n",(long)n,(long)total_mem,(long)p);
    dump_mem(stdout);
    return p;
}

void * yyrealloc(void* p, yy_size_t n , void* yyscanner)
{
    int i;
    for (i=0; i < arrsz; i++)
        if ( ptrs[i].p == p){
            total_mem -= ptrs[i].sz;
            total_mem += n;
            ptrs[i].p = (void*)realloc(p,n);
            ptrs[i].sz = n;

            printf("yyflex_realloc(%#10lx,%8ld) total=%8ld return=%8lx\n",
                    (long)p,(long)n,(long)total_mem,(long)ptrs[i].p);
            dump_mem(stdout);
            return ptrs[i].p;
        }

    fprintf(stderr,"ERROR: yyflex_realloc could not locate pointer %#lx.\n",(long)p);
    dump_mem(stdout);
    exit(1);
}

void yyfree(void* p , void* yyscanner)
{
    int i;
    for (i=0; i < arrsz; i++)
        if ( ptrs[i].p == p){
            total_mem -= ptrs[i].sz;
            free(p);
            ptrs[i].p = 0;
            ptrs[i].sz = 0;
            nptrs--;
            printf("yyflex_free(%#10lx) total=%8ld\n",(long)p,(long)total_mem);
            dump_mem(stdout);
            return;
        }

    fprintf(stderr,"ERROR: yyflex_free could not locate pointer %#lx.\n",(long)p);
    dump_mem(stdout);
    exit(1);
}

int main(void);

int
main ()
{
    yyscan_t scanner;
    arrsz = 1;
    ptrs  = (struct memsz*)calloc(1,sizeof(struct memsz));
    nptrs = 0;

    yylex_init(&scanner);
    yyset_in(stdin,scanner);
    yyset_out(stdout,scanner);
    yylex(scanner);
    yylex_destroy(scanner);
    free(ptrs);

    if ( nptrs > 0 || total_mem > 0){
        fprintf(stderr,"Oops. Looks like a memory leak: nptrs=%d, unfreed memory=%ld\n",nptrs,(long)total_mem);
        exit(1);
    }
    printf("TEST RETURNING OK.\n");
    return 0;
}
