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
/* Give yytables_fload() tables files that are damaged in ways a real
 * file never is, and check that each is refused with -1.  A loader that
 * trusts the file would read or write outside its buffers, ask for huge
 * amounts of memory, or stop on a fatal error; see tables_bad.h for how
 * the test notices.
 */
#include <stdio.h>
#include <stdlib.h>

void tb_fatal (const char *msg);
#define YY_FATAL_ERROR(msg) tb_fatal (msg)
%}

%option 8bit
%option nounput nomain nodefault noyywrap noinput
%option noyyalloc noyyrealloc noyyfree
%option fast warn tables-file="tables_fload_bad.tables"

%%

.|\n   ;

%%

#include "tables_bad.h"

int main (void);

/* A header followed by a transition table of two entries and then, if
 * START_FLAGS is not zero, a start state list of one entry.
 */
static FILE *two_tables (unsigned start_flags, unsigned long start_value)
{
    FILE *fp = tb_scratch ();
    int i;

    tb_header (fp, tb_names, 18, start_flags ? 80 : 64);
    tb_table (fp, TB_ID_TRANSITION, TB_DATA32 | TB_STRUCT, 0, 2);
    for (i = 0; i < 4; i++) {
        tb_put32 (fp, 0);
    }
    tb_put32 (fp, 0);       /* padding to a multiple of eight */
    if (start_flags) {
        tb_table (fp, TB_ID_START_STATE_LIST, start_flags, 0, 1);
        tb_put32 (fp, start_value);
    }
    return fp;
}

static void bad_dimensions (const char *what, unsigned long hilen,
                            unsigned long lolen)
{
    FILE *fp = tb_scratch ();

    tb_header (fp, tb_names, 18, 32 + 12);
    tb_table (fp, TB_ID_TRANSITION, TB_DATA32 | TB_STRUCT, hilen, lolen);
    tb_check (what, fp, -1);
}

static void bad_table (const char *what, unsigned id, unsigned flags)
{
    FILE *fp = tb_scratch ();
    int i;

    tb_header (fp, tb_names, 18, 32 + 12 + 24);
    tb_table (fp, id, flags, 0, 3);
    for (i = 0; i < 6; i++) {
        tb_put32 (fp, 0);
    }
    tb_check (what, fp, -1);
}

int main (void)
{
    FILE *fp;

    /* The tables file flex wrote for this scanner still loads. */
    if ((fp = fopen ("tables_fload_bad.tables", "rb")) == NULL) {
        fprintf (stderr, "*** Error: cannot open the generated tables file.\n");
        return 1;
    }
    tb_check ("the generated tables file", fp, 0);

    /* Headers whose strings are not both inside the header. */
    fp = tb_scratch ();
    tb_header (fp, "AAAAAA", 6, 40);
    tb_check ("a header with no NUL", fp, -1);

    fp = tb_scratch ();
    tb_header (fp, "ab\0cd", 5, 40);
    tb_check ("a header with no name", fp, -1);

    fp = tb_scratch ();
    tb_header (fp, "abcde", 6, 40);
    tb_check ("a header with no room for a name", fp, -1);

    /* A dataset no longer than its own header holds no tables. */
    fp = tb_scratch ();
    tb_header (fp, tb_names, 18, 32);
    tb_check ("a dataset with no tables", fp, -1);

    /* Sizes that would ask for too much memory or overflow. */
    bad_dimensions ("a table with a huge length", 0, 0xFFFFFFFFul);
    bad_dimensions ("a table whose size overflows", 0x01000000ul, 0x01000000ul);
    bad_dimensions ("a table with two huge dimensions", 0xFFFFFFFFul, 0xFFFFFFFFul);

    /* Flags that do not fit the table they are on. */
    bad_table ("the transition table without structs", TB_ID_TRANSITION, TB_DATA32);
    bad_table ("a transition table of indexes", TB_ID_TRANSITION,
               TB_DATA32 | TB_STRUCT | TB_PTRANS);
    bad_table ("a start state list of structs", TB_ID_START_STATE_LIST,
               TB_DATA32 | TB_STRUCT);
    bad_table ("a start state list without indexes", TB_ID_START_STATE_LIST,
               TB_DATA32);

    /* Indexes into the transition table: the last entry is fine, past it is not. */
    tb_check ("an index inside the transition table",
              two_tables (TB_DATA32 | TB_PTRANS, 1), 0);
    tb_check ("an index past the transition table",
              two_tables (TB_DATA32 | TB_PTRANS, 2), -1);
    tb_check ("a negative index",
              two_tables (TB_DATA32 | TB_PTRANS, 0xFFFFFFFFul), -1);

    if (tb_failures == 0) {
        printf ("TEST RETURNING OK.\n");
        return 0;
    }
    return 1;
}
