/*
 * Copyright (c) 2001, John W. Millaway <john43@astro.temple.edu>
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *     Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 
 *     Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SCANOPT_H
#define SCANOPT_H

#include "flexdef.h"


#ifndef NO_SCANOPT_USAGE
/* Used by scanopt_usage for pretty-printing. */
#ifdef HAVE_NCURSES_H
#include <ncurses.h>
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PROTO
#define PROTO(args) args
#endif

/* Error codes. */
enum scanopt_err_t {
    SCANOPT_ERR_OPT_UNRECOGNIZED = -1, /* Unrecognized option. */
    SCANOPT_ERR_OPT_AMBIGUOUS = -2, /* It matched more than one option name. */
    SCANOPT_ERR_ARG_NOT_FOUND= -3, /* The required arg was not found.*/
    SCANOPT_ERR_ARG_NOT_ALLOWED = -4 /* Option does not take an argument. */
};


/* flags passed to scanopt_init */
enum scanopt_flag_t {
    SCANOPT_NO_ERR_MSG  = 0x01 /* Suppress printing to stderr. */
};

/* Specification for a single option. */
struct optspec_t
{
    const char * opt_fmt;  /* e.g., "--foo=FILE", "-f FILE", "-n [NUM]" */
    int r_val; /* Value to be returned by scanopt_ex(). */
    const char* desc; /* Brief description of this option, or NULL. */
};
typedef struct optspec_t optspec_t;


/* Used internally by scanopt() to maintain state. */
/* Never modify these value directly. */
typedef void * scanopt_t;


/* Initializes scanner and checks option list for errors.
 * Parameters:
 *   options - Array of options.
 *   argc    - Same as passed to main().
 *   argv    - Same as passed to main(). First element is skipped.
 *   flags   - Control behavior.
 * Return:  A malloc'd pointer .
 */
scanopt_t* scanopt_init PROTO(( const optspec_t* options,
                                int argc, char** argv, int flags ));

/* Frees memory used by scanner.
 * Always returns 0. */
int scanopt_destroy PROTO((scanopt_t* scanner));

#ifndef NO_SCANOPT_USAGE
/* Prints a usage message based on contents of optlist.
 * Parameters:
 *   scanner  - The scanner, already initialized with scanopt_init().
 *   fp       - The file stream to write to.
 *   usage    - Text to be prepended to option list. May be NULL.
 * Return:  Always returns 0 (zero).
 */
int scanopt_usage PROTO(( scanopt_t* scanner, FILE* fp, const char* usage));
#endif

/* Scans command-line options in argv[].
 * Parameters:
 *   scanner  - The scanner, already initialized with scanopt_init().
 *   optarg   - Return argument, may be NULL.
 *              On success, it points to start of an argument.
 *   optindex - Return argument, may be NULL.
 *              On success or failure, it is the index of this option.
 *              If return is zero, then optindex is the NEXT valid option index.
 *
 * Return:  > 0 on success. Return value is from optspec_t->rval.
 *         == 0 if at end of options.
 *          < 0 on error (return value is an error code).
 *
 */
int scanopt PROTO(( scanopt_t * scanner, char ** optarg, int * optindex));

#ifdef __cplusplus
}
#endif
#endif

/* vim:set tabstop=8 softtabstop=4 shiftwidth=4: */
