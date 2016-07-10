 //  This file is part of flex.
 //  
 //  Redistribution and use in source and binary forms, with or without
 //  modification, are permitted provided that the following conditions
 //  are met:
 //  
 //  1. Redistributions of source code must retain the above copyright
 //     notice, this list of conditions and the following disclaimer.
 //  2. Redistributions in binary form must reproduce the above copyright
 //     notice, this list of conditions and the following disclaimer in the
 //     documentation and/or other materials provided with the distribution.
 //  
 //  Neither the name of the University nor the names of its contributors
 //  may be used to endorse or promote products derived from this software
 //  without specific prior written permission.
 //  
 //  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 //  IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 //  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 //  PURPOSE.

%{
#include "config.h"

%}

%option 8bit prefix="S1_"
%option nounput nomain noyywrap
%option warn stack noyy_top_state
%option c++

%x ON
%x OFF
%%
<INITIAL>{
on    yy_push_state(ON); return 10;
off   yy_push_state(OFF); return 11;
.|\n  return 12;
}
<ON>.|\n  yy_pop_state(); return 13;

<OFF>.|\n yy_pop_state(); return 14;

%%

