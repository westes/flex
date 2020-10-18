dnl Multi-backend test template
dnl
dnl Supply per-backend boilerplate for Flex tests.
dnl See testmaker.sh for how it's used.
dnl
dnl M4_TEST_BACKEND must be set to a recognized back end name.
dnl For purposes of this file, "nr" and "r" are treated as
dnl separate back ends - non-reentrant and reentrant C through
dnl the default cpp skeleton.  At time of writing this file,
dnl c99 is the only other; it is expected this will change.
dnl
dnl The purpose of this file is to properly set three macros:
dnl
dnl M4_TEST_PREAMBLE = the contyents of s top secton, if required, and
dnl the emit option required to srt the back end.
dnl
dnl M4_TEST_COMPRESSION = compression option.
dnl
dnl M4_TEST_FAILMESSAGE = A line of code required to issue dnl a
dnl failure notification to standard error and exit with a failure status.
dnl
dnl M4_TEST_POSTAMBLE = the test main.
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
define(`M4_TEST_FAILMESSAGE', `dnl
fprintf(stderr,"Invalid line.\n"); exit(-1);
')dnl close failmessage
define(`M4_TEST_POSTAMBLE', `dnl
int main (void)
{
    yyin = stdin;
    yyout = stdout;
    while( yylex() )
    {
    }
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
define(`M4_TEST_FAILMESSAGE', `fprintf(stderr,"Invalid line.\n"); exit(-1);')
define(`M4_TEST_POSTAMBLE', `dnl
int main (void)
{
    yyscan_t  lexer;
    yylex_init( &lexer );
    yyset_out ( stdout,lexer);
    yyset_in  ( stdin, lexer);
    while( yylex(lexer) )
    {
    }
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
define(`M4_TEST_FAILMESSAGE', `log.Fatal("Invalid line"); os.Exit(-1);')
define(`M4_TEST_POSTAMBLE', `dnl
func main(void) {
	lexer := new(FlexLexer)
	lexer.yysetOut(os.Stdout)
	lexer.yysetIn(os.Stdin)
	for lexer.yylex() {
	}
	fmt.Printf("TEST RETURNING OK.\n")
	os.Exit(0)
}
')dnl clpse postamble
')dnl close go
dnl
dnl Additional backends go here
