#include "scanner-1.h"
#include "scanner-2.h"

int
main ( int argc, char** argv )
{
    int S1_ok=1, S2_ok=1;
    YY_BUFFER_STATE buff1, buff2;
    S1_out = S2_out = stdout;
    buff1 = S1__scan_string("foo on bar off");
    buff2 = S2__scan_string("on blah blah off foo on bar off");

    /* scan simultaneously. */
    while(S1_ok || S2_ok)
    {
        if (S1_ok)
            S1_ok = S1_lex();
        if (S2_ok)
            S2_ok = S2_lex();
    }
    S1__delete_buffer(buff1);
    S2__delete_buffer(buff2);
    printf("TEST RETURNING OK.\n");
    return 0;
}


/* vim:set tabstop=8 softtabstop=4 shiftwidth=4: */
