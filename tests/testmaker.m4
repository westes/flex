dnl Multi-backend test template
dnl
dnl Supply per-backend boilerplate for Flex tests.
dnl See testmaker.sh for how it's used.
dnl
dnl M4_TEST_BACKEND must be set to a recognized back end name.
dnl For purposes of this file, "nr" and "r" are treated as
dnl separate back ends - non-reentrant and reentrant C through
dnl the default cpp skeleton. The c99 backend is treated here
dnl as re-eentrant C, but may have a different skeleton file.
dnl
dnl The purpose of this file is to properly set three macros:
dnl
dnl M4_TEST_PREAMBLE = the contyents of s top secton, if required, and
dnl the emit option required to srt the back end.
dnl
dnl M4_TEST_COMPRESSION = compression option.
dnl
dnl M4_TEST_ECHO = echo the token buffer and continue.
dnl
dnl M4_TEST_FAILMESSAGE = a line of code required to issue dnl a
dnl failure notification to standard error and exit with a failure status.
dnl
dnl M4_TEST_POSTAMBLE = the test main.
dnl
dnl M4_TEST_TABLE_SERIALIZATION = define this to exercise table
dnl serialization.
dnl
dnl M4_TEST_TABLE_VERIFICATION = define this to exercise table
dnl verification and exit.
dnl
dnl These macros are to be expanded by files with a .rules extension
dnl that contain pure flex rulesets and no backend-specific code.
dnl
define(`M4_TEST_FAILMESSAGE', `INVALID BACK END')dnl
dnl
ifelse(M4_TEST_BACKEND, `nr', `dnl
define(`M4_TEST_PREAMBLE', `dnl
%{
#include "config.h"
#include <stdio.h>
%}
')dnl close preamble
define(`M4_TEST_ECHO', `yyecho();')
define(`M4_TEST_FAILMESSAGE', `dnl
fprintf(stderr,"Invalid line.\n"); exit(-1);
')dnl close failmessage
define(`M4_TEST_POSTAMBLE', `dnl
int main (int argc, char **argv)
{
ifdef(`M4_TEST_TABLE_SERIALIZATION', `dnl
    FILE *fp;
')dnl table_serialization
    yyin = stdin;
    yyout = stdout;
ifdef(`M4_TEST_TABLE_SERIALIZATION', `dnl
    if((fp  = fopen(argv[1],"rb"))== NULL)
        YY_FATAL_ERROR("could not open tables file for reading");

    if(yytables_fload(fp) < 0)
        YY_FATAL_ERROR("yytables_fload returned < 0");
    ifdef(`M4_TEST_TABLE_VERIFICATION', `exit(0);')
')dnl table_serialization
    while( yylex() )
    {
    }
ifdef(`M4_TEST_TABLE_SERIALIZATION', `dnl
    yytables_destroy();
')dnl table_serialization
    printf("TEST RETURNING OK.\n");
    return 0;
}
')dnl close postamble
')dnl close nr
dnl
ifelse(M4_TEST_BACKEND, `r', `dnl
define(`M4_TEST_PREAMBLE', `dnl
%{
#include "config.h"
#include <stdio.h>
%}
')dnl close preamble
define(`M4_TEST_ECHO', `yyecho();')
define(`M4_TEST_FAILMESSAGE', `fprintf(stderr,"Invalid line.\n"); exit(-1);')
define(`M4_TEST_POSTAMBLE', `dnl
int main (int argc, char **argv)
{
ifdef(`M4_TEST_TABLE_SERIALIZATION', `dnl
    FILE *fp;
')dnl table_serialization
    yyscan_t  lexer;
    yylex_init( &lexer );
    yyset_out ( stdout,lexer);
    yyset_in  ( stdin, lexer);
ifdef(`M4_TEST_TABLE_SERIALIZATION', `dnl
    if((fp  = fopen(argv[1],"rb"))== NULL)
        YY_FATAL_ERROR("could not open tables file for reading");

    if(yytables_fload(fp, yyscanner) < 0)
        YY_FATAL_ERROR("yytables_fload returned < 0");
    ifdef(`M4_TEST_TABLE_VERIFICATION', `exit(0);')
')dnl table_serialization
    while( yylex(lexer) )
    {
    }
ifdef(`M4_TEST_TABLE_SERIALIZATION', `dnl
    yytables_destroy(yyscanner);
')dnl table_serialization
    yylex_destroy( lexer );
    printf("TEST RETURNING OK.\n");
    return 0;
}
')dnl close postamble
')dnl close r
dnl
dnl A hypothetical example
ifelse(M4_TEST_BACKEND, `go', `dnl
define(`M4_TEST_PREAMBLE', `dnl
%top{
package main

import (
    "fmt"
    "log"
    "os"
)
%}
%option emit="go"
')dnl close preamble
define(`M4_TEST_ECHO', `yyecho()')
define(`M4_TEST_FAILMESSAGE', `log.Fatal("Invalid line"); os.Exit(-1);')
define(`M4_TEST_POSTAMBLE', `dnl
func main(void) {
	lexer := new(FlexLexer)
	lexer.yysetOut(os.Stdout)
	lexer.yysetIn(os.Stdin)
	for lexer.yylex() != EOF {
	}
	fmt.Printf("TEST RETURNING OK.\n")
	os.Exit(0)
}
')dnl clpse postamble
')dnl close go
dnl
dnl Additional backends go here
