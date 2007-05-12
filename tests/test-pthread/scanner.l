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
/* A scanner file to build "scanner.c".
   Input language is any text made of spaces, newlines, and alphanumerics.

   We create N_THREADS number of threads. Each thread has it's own scanner.
   Each thread selects one of the files specified in ARGV, scans it, then
   closes it. This is repeated N_SCANS numebr of times for each thread.

   The idea is to press the scanner to break under threads.
   If we see  "Scanner Jammed", then we know

*/
#include <stdio.h>
#include <stdlib.h>
#include <config.h>

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif

/* A naive test for segfaults when accessing yytext. */
static int process_text(char* s, yyscan_t  scanner);

%}

%option 8bit outfile="scanner.c" prefix="test"
%option nounput nomain nodefault
%option yywrap
%option reentrant
%option warn

    /* Arbitrary states.*/
%x STATE_1
%x STATE_2

%%

    #define NUMBER 200
    #define WORD   201

<INITIAL>[[:digit:]]+ { BEGIN(STATE_1); process_text(yytext,yyscanner); return NUMBER; }
<INITIAL>[[:alpha:]]+ { BEGIN(STATE_2); process_text(yytext,yyscanner); return WORD; }

<STATE_1>[[:alpha:]]+ { BEGIN(0); process_text(yytext,yyscanner); return WORD; }
<STATE_1>[[:digit:]]+ { BEGIN(0); process_text(yytext,yyscanner); return NUMBER; }

<STATE_2>[[:alpha:]]+ { BEGIN(0); process_text(yytext,yyscanner); return WORD; }
<STATE_2>[[:digit:]]+ { BEGIN(0); process_text(yytext,yyscanner); return NUMBER; }

<INITIAL,STATE_1,STATE_2>" "|\t|\r|\n { process_text(yytext,yyscanner); }
<INITIAL,STATE_1,STATE_2>[^[:alnum:][:space:]\t\r\n] {
        /*fprintf(stderr,"*** Error: bad input char '%c'.\n", yytext[0]); */
        yyterminate();
    }
<INITIAL,STATE_1,STATE_2>[[:space:]\r\n]+  { }
%%

int yywrap( yyscan_t  scanner) { return 1; }
static int process_text(char* s, yyscan_t  scanner)
{
    return (int)(*s) + (int) *(s + yyget_leng(scanner)-1);
}

int main(int ARGC, char *ARGV[]);

#ifndef HAVE_LIBPTHREAD
  int main (int ARGC, char *ARGV[]) {
    printf(
       "TEST ABORTED because pthread library not available \n"
       "-- This is expected on some systems. It is not a flex error.\n" );
    return 0;
  }
#else

#define N_THREADS 4
#define N_SCANS   20
#define INPUT_FILE "test.input"

/* Each thread selects the next file to scan in round-robin fashion.
   If there are less files than threads, some threads may block. */

static pthread_mutex_t next_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t go_ahead  = PTHREAD_MUTEX_INITIALIZER;
static int n_files, next_file;

static pthread_mutex_t *file_locks;
static char **filenames;


void * thread_func ( void* arg )
{
    int i;

    /* Wait for go-ahead. */
    pthread_mutex_lock( &go_ahead);
    pthread_mutex_unlock(&go_ahead);

    for( i =0 ; i < N_SCANS ; i++ )
    {
        int main(int ARGC, char *ARGV[]);

        int next;
        yyscan_t  scanner;
        FILE * fp;

        pthread_mutex_lock ( &next_lock );
        next = (next_file++) % n_files;
        pthread_mutex_unlock ( &next_lock );

        pthread_mutex_lock ( &file_locks[ next ] );

        yylex_init( &scanner );
        /*printf("Scanning file %s  #%d\n",filenames[next],i); fflush(stdout); */
        if((fp = fopen(filenames[next],"r"))==NULL) {
            perror("fopen");
            return NULL;
        }
        yyset_in(fp,scanner);

        while( yylex( scanner) != 0)
        {
        }
        fclose(fp);
        yylex_destroy(scanner);
        pthread_mutex_unlock ( &file_locks[ next ] );
    }
    return NULL;
}

int main (int ARGC, char *ARGV[])
{
    int i;
    pthread_t threads[N_THREADS];

    if( ARGC < 2 ) {
        fprintf(stderr,"*** Error: No filenames specified.\n");
        exit(-1);
    }

    /* Allocate and initialize the locks. One for each filename in ARGV. */
    file_locks = (pthread_mutex_t*)malloc( (ARGC-1) * sizeof(pthread_mutex_t));
    for( i = 0; i < ARGC-1; i++)
        pthread_mutex_init( &file_locks[i], NULL );

    n_files = ARGC -1;
    filenames = ARGV + 1;
    next_file = 0;

    /* prevent threads from starting until all threads have been created. */
    pthread_mutex_lock(&go_ahead);

    /* Create N threads then wait for them. */
    for(i =0; i < N_THREADS ; i++ ) {
        if( pthread_create( &threads[i], NULL, thread_func, NULL) != 0)
        {
            fprintf(stderr, "*** Error: pthread_create failed.\n");
            exit(-1);
        }
        printf("Created thread %d.\n",i); fflush(stdout);
    }

    /* Tell threads to begin. */
    pthread_mutex_unlock(&go_ahead);

    for(i =0; i < N_THREADS ; i++ ) {
        pthread_join ( threads[i], NULL );
        printf("Thread %d done.\n", i ); fflush(stdout);
    }

    for( i = 0; i < ARGC-1; i++)
        pthread_mutex_destroy( &file_locks[i] );

    pthread_mutex_destroy( &next_lock );
    pthread_mutex_destroy( &go_ahead );
    free( file_locks );
    printf("TEST RETURNING OK.\n");
    return 0;
}

#endif /* HAVE_LIBPTHREAD */

