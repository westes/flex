%{
/* Build "scanner.c". 
   The scanner is not important. 
   This test is really about compilation. See "main.c".
*/
#include <stdio.h>
#include <stdlib.h>
#include "config.h"

%}

%option reentrant
%option 8bit outfile="scanner.c" prefix="test"
%option nounput nomain noyywrap 
%option warn


%%

.|\n              { }

%%

