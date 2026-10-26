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

/* Helpers for the tests that give damaged tables files to
 * yytables_fload().  Include this after the first "%%" of the scanner,
 * and in the first section define
 *
 *     #define YY_FATAL_ERROR(msg) tb_fatal(msg)
 *
 * after declaring tb_fatal() with the prototype below.
 *
 * What the helpers provide:
 *
 * - An allocator, for the scanner's yyalloc and yyfree, that puts each
 *   block directly in front of a page the system will not let us read or
 *   write.  A loader that goes past the end of its buffer then crashes
 *   the test.  Without mmap the blocks come from malloc and only the
 *   size limit below is checked.
 * - A limit on the size of any one block, so that a table size taken
 *   from the file cannot make the loader ask for a huge allocation.
 * - A way to record a call to YY_FATAL_ERROR and carry on, so that a file
 *   the loader should refuse with -1 but instead stops on counts as a
 *   failure.
 */

#include <setjmp.h>
#include <string.h>
#if defined(__unix__) || defined(__APPLE__)
#include <unistd.h>
#include <sys/mman.h>
#if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#define MAP_ANONYMOUS MAP_ANON
#endif
#endif

#define TB_MAX_BLOCK ((size_t) 16 * 1024 * 1024)

#define TB_MAGIC     0xF13C57B1ul
#define TB_ID_TRANSITION       0x0B
#define TB_ID_START_STATE_LIST 0x0A
#define TB_DATA32    0x04
#define TB_PTRANS    0x08
#define TB_STRUCT    0x10

struct tb_block {
    char *base;         /* start of the whole mapping, NULL if from malloc */
    size_t total;       /* length of the whole mapping */
};

static jmp_buf tb_jmp;
static int tb_armed = 0;
static int tb_fatal_seen = 0;
static int tb_too_big = 0;
static int tb_failures = 0;

void tb_fatal (const char *msg);

void tb_fatal (const char *msg)
{
    fprintf (stderr, "    (fatal error: %s)\n", msg);
    tb_fatal_seen = 1;
    if (tb_armed) {
        longjmp (tb_jmp, 1);
    }
    exit (2);
}

void *yyalloc (yy_size_t size)
{
    size_t rounded = (size + 7) & ~(size_t) 7;
    struct tb_block *b;
    char *p;

    if (size > TB_MAX_BLOCK) {
        tb_too_big = 1;
        return NULL;
    }
#ifdef MAP_ANONYMOUS
    {
        size_t ps = (size_t) sysconf (_SC_PAGESIZE);
        size_t pages = (sizeof (struct tb_block) + rounded + ps - 1) / ps;
        size_t total = (pages + 1) * ps;
        char *base = (char *) mmap (NULL, total, PROT_READ | PROT_WRITE,
                                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        if (base == (char *) MAP_FAILED) {
            return NULL;
        }
        if (mprotect (base + pages * ps, ps, PROT_NONE) != 0) {
            munmap (base, total);
            return NULL;
        }
        p = base + pages * ps - rounded;
        b = (struct tb_block *) p - 1;
        b->base = base;
        b->total = total;
    }
#else
    b = (struct tb_block *) malloc (sizeof (*b) + rounded);
    if (b == NULL) {
        return NULL;
    }
    b->base = NULL;
    b->total = 0;
    p = (char *) (b + 1);
#endif
    /* Not zero, so that a string search that runs off the end keeps going. */
    memset (p, 'X', rounded);
    return p;
}

void *yyrealloc (void *ptr, yy_size_t size)
{
    (void) ptr;
    (void) size;
    tb_fatal ("the test does not expect a reallocation");
    return NULL;
}

void yyfree (void *ptr)
{
    struct tb_block *b;

    if (ptr == NULL) {
        return;
    }
    b = (struct tb_block *) ptr - 1;
#ifdef MAP_ANONYMOUS
    munmap (b->base, b->total);
#else
    free (b);
#endif
}

static void tb_put16 (FILE *fp, unsigned v)
{
    fputc ((v >> 8) & 0xff, fp);
    fputc (v & 0xff, fp);
}

static void tb_put32 (FILE *fp, unsigned long v)
{
    tb_put16 (fp, (unsigned) ((v >> 16) & 0xffff));
    tb_put16 (fp, (unsigned) (v & 0xffff));
}

/* A header whose version and name area is exactly LEN bytes.  A real
 * one is padded to a multiple of eight, so LEN is 18 for ours.
 */
static void tb_header (FILE *fp, const char *area, unsigned len,
                       unsigned long ssize)
{
    tb_put32 (fp, TB_MAGIC);
    tb_put32 (fp, 14 + len);
    tb_put32 (fp, ssize);
    tb_put16 (fp, 0);
    fwrite (area, 1, len, fp);
}

/* The version and name of the tables a scanner without a prefix wants. */
static const char tb_names[18] = "2.6.4\0yytables";

static void tb_table (FILE *fp, unsigned id, unsigned flags,
                      unsigned long hilen, unsigned long lolen)
{
    tb_put16 (fp, id);
    tb_put16 (fp, flags);
    tb_put32 (fp, hilen);
    tb_put32 (fp, lolen);
}

static FILE *tb_scratch (void)
{
    FILE *fp = tmpfile ();

    if (fp == NULL) {
        fprintf (stderr, "*** Error: cannot create a temporary file.\n");
        exit (1);
    }
    return fp;
}

/* Load FP and check the outcome.  WANT is 0 for a file that should load
 * and -1 for one that should be refused without any fatal error.
 */
static void tb_check (const char *what, FILE *fp, int want)
{
    int got = 12345;

    tb_fatal_seen = 0;
    tb_too_big = 0;
    rewind (fp);
    tb_armed = 1;
    if (setjmp (tb_jmp) == 0) {
        got = yytables_fload (fp);
    }
    tb_armed = 0;
    if (tb_fatal_seen || tb_too_big || got != want) {
        fprintf (stderr, "*** Error: %s: wanted %d, got %d%s%s.\n", what,
                 want, got, tb_fatal_seen ? ", with a fatal error" : "",
                 tb_too_big ? ", after asking for too much memory" : "");
        tb_failures++;
    }
    fclose (fp);
}
