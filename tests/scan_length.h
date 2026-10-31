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


/* Helpers shared by the scan_length tests: a record of the last fatal error,
   a check that it was the expected one, and a string of 4294967295 bytes
   (which an int sees as -1) built by mapping the same 1 MiB of 'a' over
   4 GiB of address space, so it uses about 1 MiB of memory.
 */

#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(_POSIX_MAPPED_FILES) && _POSIX_MAPPED_FILES > 0
#include <sys/mman.h>
#if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#define MAP_ANONYMOUS MAP_ANON
#endif
#endif

static jmp_buf fatal_env;
static const char *fatal_msg;

/* The fatal error handler: return to main() instead of exiting. */
static void
scan_length_panic ( const char *msg )
{
    fatal_msg = msg;
    longjmp(fatal_env, 1);
}

/* Report a failure unless the last step stopped with the fatal error WANT. */
static int
check_fatal ( const char *what, const char *want, int returned )
{
    if ( returned || fatal_msg == NULL || strcmp(fatal_msg, want) != 0 ) {
        fprintf(stderr, "*** Error: %s: expected the error \"%s\", got %s%s%s.\n",
                what, want,
                fatal_msg ? "\"" : "no error",
                fatal_msg ? fatal_msg : "", fatal_msg ? "\"" : "");
        return 1;
    }
    return 0;
}

#if defined(_POSIX_MAPPED_FILES) && _POSIX_MAPPED_FILES > 0 \
    && defined(MAP_ANONYMOUS) && SIZE_MAX > 0xffffffffUL
#define CHUNK ((size_t) 1 << 20)
#define TOTAL ((size_t) 1 << 32)

/* Return a string of TOTAL - 1 bytes, or NULL if this system cannot map one.
 * The file holds a chunk of 'a' and then a chunk of 'a' that ends in NUL.
 * The first is mapped over everything but the last chunk of the region.
 */
static char *
map_long_string ( void )
{
    FILE *fp;
    char *base, *chunk;
    size_t i;

    if ( (fp = tmpfile()) == NULL || (chunk = malloc(CHUNK)) == NULL ) {
        return NULL;
    }
    memset(chunk, 'a', CHUNK);
    if ( fwrite(chunk, 1, CHUNK, fp) != CHUNK ) {
        return NULL;
    }
    chunk[CHUNK - 1] = '\0';
    if ( fwrite(chunk, 1, CHUNK, fp) != CHUNK || fflush(fp) != 0 ) {
        return NULL;
    }
    free(chunk);

    base = mmap(NULL, TOTAL, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if ( base == MAP_FAILED ) {
        return NULL;
    }
    for ( i = 0; i < TOTAL - CHUNK; i += CHUNK ) {
        if ( mmap(base + i, CHUNK, PROT_READ, MAP_PRIVATE | MAP_FIXED,
                  fileno(fp), 0) == MAP_FAILED ) {
            return NULL;
        }
    }
    if ( mmap(base + i, CHUNK, PROT_READ, MAP_PRIVATE | MAP_FIXED,
              fileno(fp), (off_t) CHUNK) == MAP_FAILED ) {
        return NULL;
    }
    return base;
}
#else
static char *
map_long_string ( void )
{
    return NULL;
}
#endif
