%{
#include <stdio.h>
#include <string.h>
#include "y.tab.h"                 /* this comes from bison        */

#define TRUE  1
#define FALSE 0

#define copy_and_return(token_type) { strcpy(yylval.name,yytext); \
                                      return(token_type); }

int             yylexlinenum = 0;  /* so we can count lines        */
%}

%%
                            /* Lexical scanning rules begin from here.  */

MEN|WOMEN|STOCKS|TREES      copy_and_return(NOUN)
MISTAKES|GNUS|EMPLOYEES     copy_and_return(NOUN)
LOSERS|USERS|CARS|WINDOWS   copy_and_return(NOUN)

DATABASE|NETWORK|FSF|GNU    copy_and_return(PROPER_NOUN)
COMPANY|HOUSE|OFFICE|LPF    copy_and_return(PROPER_NOUN)

THE|THIS|THAT|THOSE         copy_and_return(DECLARATIVE)

ALL|FIRST|LAST              copy_and_return(CONDITIONAL)

FIND|SEARCH|SORT|ERASE|KILL copy_and_return(VERB)
ADD|REMOVE|DELETE|PRINT     copy_and_return(VERB)

QUICKLY|SLOWLY|CAREFULLY    copy_and_return(ADVERB)

IN|AT|ON|AROUND|INSIDE|ON   copy_and_return(POSITIONAL)

"."                         return(PERIOD);                             
"\n"                        yylexlinenum++; return(NEWLINE);            
.                                                                       
%%

