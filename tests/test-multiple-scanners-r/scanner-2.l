%{
/* A template scanner file to build "scanner-2.c" and "scanner-2.h". */
#include <stdio.h>
#include <stdlib.h>
#include "config.h"

%}

%option header="scanner-2.h"
%option 8bit outfile="scanner-2.c" prefix="S2_"
%option nounput nomain noyywrap
%option warn stack reentrant noyy_top_state

%x OFF
%x ON
%%
<INITIAL>{
on    yy_push_state(ON, yy_globals); return 3;
off   yy_push_state(OFF, yy_globals); return 4;
.|\n  return 5;
}
<ON>.|\n  yy_pop_state(yy_globals); return 6;

<OFF>.|\n yy_pop_state(yy_globals); return 7;
%%

