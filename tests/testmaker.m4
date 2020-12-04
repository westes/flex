dnl Multi-backend test template
dnl
dnl Supply per-backend boilerplate for Flex tests.
dnl See testmaker.sh for how it's used.
dnl
dnl M4_TEST_BACKEND must be set to a recognized back end name.
dnl For purposes of this file, "nr" and "r" are treated as
dnl separate back ends - non-reentrant and reentrant C through
dnl the default cpp skeleton. The c99 backend is treated here
dnl as re-entrant C, but may have a different skeleton file.
dnl
dnl The purpose of this file is to properly set these macros:
dnl
dnl M4_TEST_PREAMBLE = the contyents of s top secton, if required, and
dnl the emit option required to srt the back end.
dnl
dnl M4_TEST_COMPRESSION = compression option.
dnl
dnl M4_TEST_DO = Add a statement terminator if target language
dnl requires it.
dnl
dnl M4_TEST_FAILMESSAGE = a line of code required to issue dnl a
dnl failure notification to standard error and exit with a failure status.
dnl
dnl M4_TEST_INCREMENT = increment the argument variable.
dnl
dnl M4_TEST_DECREMENT = decrement the argument variable.
dnl
dnl M4_TEST_POSTAMBLE = the test main.
dnl
dnl M4_TEST_TABLE_SERIALIZATION = define this to exercise table
dnl serialization.
dnl
dnl M4_TEST_TABLE_VERIFICATION = define this to exercise table
dnl verification and exit.
dnl
dnl M4_TEST_INITHOOK = define this in a rules file to splice code into
dnl the test main as a compilation test.
dnl
dnl These macros are to be expanded by files with a .rules extension
dnl that contain pure flex rulesets and no backend-specific code.
dnl
changecom
define(`M4_TEST_FAILMESSAGE', `INVALID BACK END')dnl
define(`M4_TEST_INITHOOK', `')dnl
dnl
ifelse(M4_TEST_BACKEND, `nr', `dnl
define(`M4_TEST_PREAMBLE', `dnl
%{
#include "config.h"
#include <stdio.h>
%}
')dnl close preamble
define(`M4_TEST_DO', `$1;')
define(`M4_TEST_FAILMESSAGE', `fprintf(stderr,"TEST FAILED: %d:\"%s\".\n", yylineno, yytext); exit(1);')
define(`M4_TEST_ASSERT', `if (!($1)) {fprintf(stderr,"ASSERT FAILED: %d:\"%s\"\n", yylineno, yytext); exit(1);}')
m4_ifdef(`M4_TEST_ENABLEDEBUG', `define(`M4_TEST_INITHOOK', `flex_debug = 1;')') 
define(`M4_TEST_INCREMENT', `++$1')
define(`M4_TEST_DECREMENT', `--$1')
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
        yypanic("could not open tables file for reading");

    if(yytables_fload(fp) < 0)
        yypanic("yytables_fload returned < 0");
    ifdef(`M4_TEST_TABLE_VERIFICATION', `exit(0);')
    M4_TEST_INITHOOK
')dnl table_serialization
    while( yylex() != YY_NULL )
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
define(`M4_TEST_DO', `$1;')
define(`M4_TEST_FAILMESSAGE', `fprintf(stderr,"TEST FAILED: %d:\"%s\".\n", yylineno, yytext); exit(1);')
define(`M4_TEST_ASSERT', `if (!$1) {fprintf(stderr,"ASSERT FAILED: %d:\"%s\"\n", yylineno, yytext); exit(1);}')
m4_ifdef(`M4_TEST_ENABLEDEBUG', `define(`M4_TEST_INITHOOK', `yyset_debug (yyget_debug(lexer), lexer);')') 
define(`M4_TEST_INCREMENT', `++$1')
define(`M4_TEST_DECREMENT', `--$1')
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
        yypanic("could not open tables file for reading", lexer);

    if(yytables_fload(fp, yyscanner) < 0)
        yypanic("yytables_fload returned < 0", lexer);
    ifdef(`M4_TEST_TABLE_VERIFICATION', `exit(0);')
')dnl table_serialization
    M4_TEST_INITHOOK
    while( yylex(lexer) != YY_NULL )
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
dnl This is a fake Go wrapper that will only work as long as the "go"
dnl back end is actually generating C.
ifelse(M4_TEST_BACKEND, `go', `dnl
define(`M4_TEST_PREAMBLE', `dnl
%{
#include "config.h"
#include <stdio.h>
%}
%option emit="go"
')dnl close preamble
define(`M4_TEST_DO', `$1;')
define(`M4_TEST_FAILMESSAGE', `fprintf(stderr,"TEST FAILED: %d:\"%s\".\n", yylineno, yytext); exit(1);')
define(`M4_TEST_ASSERT', `if (!$1) {fprintf(stderr,"ASSERT FAILED: %d:\"%s\"\n", yylineno, yytext); exit(1);}')
m4_ifdef(`M4_TEST_ENABLEDEBUG', `define(`M4_TEST_INITHOOK', `yyset_debug (yyget_debug(lexer), lexer);')') 
define(`M4_TEST_INCREMENT', `++$1')
define(`M4_TEST_DECREMENT', `--$1')
define(`M4_TEST_POSTAMBLE', `dnl
int main (int argc, char **argv)
{
    FlexLexer *lexer;
    yylex_init( &lexer );
    yyset_out ( stdout,lexer);
    yyset_in  ( stdin, lexer);
    M4_TEST_INITHOOK
    while( yylex(lexer) != flexEOF )
    {
    }
    yylex_destroy( lexer );
    printf("TEST RETURNING OK.\n");
    return 0;
}
')dnl close postamble
')dnl close fake-go
dnl
dnl A hypothetical example
ifelse(M4_TEST_BACKEND, `hypothetical-go', `dnl
define(`M4_TEST_PREAMBLE', `dnl
%top{
package main

import (
    "fmt"
    "log"
    "os"
)
%}
%option emit="hypothetical-go"
')dnl close preamble
define(`M4_TEST_DO', `$1')
define(`M4_TEST_FAILMESSAGE', `fmt.Fprintf(os.Stderr, "TEST FAILMESSAGE: %d:\"%s\"\n", yylineno, yytext); os.Exit(1);')
define(`M4_TEST_ASSERT', `if !$1 {fmt.Fprintf(os.Stderr,"ASSERT FAILED: %d:\"%s\"\n", yylineno, yytext); os.Exit(1);}')
m4_ifdef(`M4_TEST_ENABLEDEBUG', `define(`M4_TEST_INITHOOK', `lexer.yysetDebug(lexer.yygetDebug())')') 
define(`M4_TEST_INCREMENT', `++$1')
define(`M4_TEST_DECREMENT', `--$1')
define(`M4_TEST_POSTAMBLE', `dnl
func main(void) {
	lexer := new(FlexLexer)
	lexer.yysetOut(os.Stdout)
	lexer.yysetIn(os.Stdin)
	M4_TEST_INITHOOK
	for lexer.yylex() != flexEOF {
	}
	fmt.Printf("TEST RETURNING OK.\n")
	os.Exit(0)
}
')dnl close postamble
')dnl close go
dnl
dnl Additional backends go here
