%{
/* A template scanner file to build "scanner-1.c" and "scanner-1.h". */
#include <stdio.h>
#include <stdlib.h>
#include "config.h"

%}

%option header="scanner-1.h"
%option 8bit outfile="scanner-1.c" prefix="S1_"
%option nounput nomain noyywrap noyy_top_state
%option warn stack reentrant

%x ON
%x OFF
%%
<INITIAL>{
on    yy_push_state(ON, yy_globals); return 10;
off   yy_push_state(OFF, yy_globals); return 11;
.|\n  return 12;
}
<ON>.|\n  yy_pop_state(yy_globals); return 13;

<OFF>.|\n yy_pop_state(yy_globals); return 14;

%%

