/*
 * This file is part of flex.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * Neither the name of the University nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE.
 */

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
%option tables-file="scanner-1.tables"

%x ON
%x OFF
%%
<INITIAL>{
on    yy_push_state(ON, yyscanner); return 10;
off   yy_push_state(OFF, yyscanner); return 11;
.|\n  return 12;
}
<ON>.|\n  yy_pop_state(yyscanner); return 13;

<OFF>.|\n yy_pop_state(yyscanner); return 14;

%%

