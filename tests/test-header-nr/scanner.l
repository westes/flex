%{
/* A template scanner file to build "scanner.c". */
#include <stdio.h>
#include <stdlib.h>
#include "config.h"

%}

%option 8bit outfile="scanner.c" prefix="test" header="scanner.h"
%option nounput nomain noyywrap 
%option warn


%%

.|\n              { }

%%

