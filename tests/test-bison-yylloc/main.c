#include "parser.h"
#include "scanner.h"

extern int testparse(yyscan_t);

int main ( int argc, char** argv )
{
    yyscan_t scanner;
    /*yydebug =1;*/
    testlex_init ( &scanner );
    testset_in(stdin,scanner);
    testparse ( scanner );
    testlex_destroy ( scanner );
    return 0;
}



/* vim:set tabstop=8 softtabstop=4 shiftwidth=4: */
