#include "scanner.h"

/* The scanner itself is not important here.
 * We simply try to use all the functions that are exported in the
 * header, to see if we get any compiler warnings.
 */
int
main ( int argc, char** argv )
{
    yyscan_t  scanner;
    FILE *fp;
    char * extra = "EXTRA";
    
    testlex_init(&scanner);
    testset_in(stdin,scanner);
    testset_out(stdout,scanner);    
    testset_extra(extra,scanner);
    
    fp = testget_in(scanner);
    fp = testget_out(scanner);

    while(testlex(scanner)) {
        char * text;
        int line;
        line = testget_lineno(scanner);
        text = testget_text(scanner);
        
        if( (char*)testget_extra(scanner) != extra)
            break;
        
        if ( !text || line < 0)
            continue;
    }
    testlex_destroy(scanner);
    printf("TEST RETURNING OK.\n");
    return 0;
}


/* vim:set tabstop=8 softtabstop=4 shiftwidth=4: */
