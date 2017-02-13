/* This stub will be used when Bison is not available on the user's host. */

/*  This file is part of flex.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  Neither the name of the University nor the names of its contributors
 *  may be used to endorse or promote products derived from this software
 *  without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 *  IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE.
 */
#include <stdio.h>

int main (int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    puts(
        "This test requires Bison. Install Bison and re-run \"configure && make check\"\n"
        "to perform this test. (This file is stub code.)"
    );

    /* Exit status for a skipped test */
    return 77;
}

/* vim:set tabstop=8 softtabstop=4 shiftwidth=4: */
