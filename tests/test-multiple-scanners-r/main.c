#include "scanner-1.h"
#include "scanner-2.h"

int
main ( int argc, char** argv )
{
    int S1_ok=1, S2_ok=1;
    YY_BUFFER_STATE buff1, buff2;
    yyscan_t scan1, scan2;
    
    S1_lex_init(&scan1);
    S2_lex_init(&scan2);

    S1_set_out(stdout,scan1);
    S2_set_out(S1_get_out(scan1),scan2);
    
    buff1 = S1__scan_string("foo on bar off", scan1);
    buff2 = S2__scan_string("on blah blah off foo on bar off", scan2);

    /* scan simultaneously. */
    while(S1_ok || S2_ok)
    {
        if (S1_ok)
            S1_ok = S1_lex(scan1);
        if (S2_ok)
            S2_ok = S2_lex(scan2);
    }
    S1__delete_buffer(buff1, scan1);
    S2__delete_buffer(buff2, scan2);

    S1_lex_destroy(scan1);
    S2_lex_destroy(scan2);
    printf("TEST RETURNING OK.\n");
    return 0;
}


/* vim:set tabstop=8 softtabstop=4 shiftwidth=4: */
