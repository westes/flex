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
/* Like tables_fload_bad, but for a scanner built with tables-verify,
 * which checks the file against the scanner's own tables instead of
 * loading it.  The file must not be longer than those tables, or the
 * check reads past them.
 */
#include <stdio.h>
#include <stdlib.h>

void tb_fatal (const char *msg);
#define YY_FATAL_ERROR(msg) tb_fatal (msg)
%}

%option 8bit
%option nounput nomain nodefault noyywrap noinput
%option noyyalloc noyyrealloc noyyfree
%option fast warn tables-verify tables-file="tables_verify_bad.tables"

%%

.|\n   ;

%%

#include "tables_bad.h"

int main (void);

/* A table of N entries made of 0x7f, which matches no real table. */
static void too_long (const char *what, unsigned id, unsigned flags,
                      unsigned long n)
{
    FILE *fp = tb_scratch ();
    unsigned long i;

    tb_header (fp, tb_names, 18, 32 + 12 + 4 * n + 8);
    tb_table (fp, id, flags, 0, n);
    for (i = 0; i < n * ((flags & TB_STRUCT) ? 2 : 1); i++) {
        tb_put32 (fp, 0x7f7f7f7ful);
    }
    tb_check (what, fp, -1);
}

int main (void)
{
    FILE *fp;

    /* The tables file flex wrote for this scanner still verifies. */
    if ((fp = fopen ("tables_verify_bad.tables", "rb")) == NULL) {
        fprintf (stderr, "*** Error: cannot open the generated tables file.\n");
        return 1;
    }
    tb_check ("the generated tables file", fp, 0);

    too_long ("a transition table longer than the scanner's",
              TB_ID_TRANSITION, TB_DATA32 | TB_STRUCT, 100000);
    too_long ("a start state list longer than the scanner's",
              TB_ID_START_STATE_LIST, TB_DATA32 | TB_PTRANS, 5000);

    if (tb_failures == 0) {
        printf ("TEST RETURNING OK.\n");
        return 0;
    }
    return 1;
}
