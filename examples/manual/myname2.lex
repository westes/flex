/*
 * myname2.lex : A sample Flex program
 *               that does token replacement.                      
 */

%{
#include <stdio.h>
%}

%x STRING
%%
\"                ECHO; BEGIN(STRING); 
<STRING>[^\"\n]*  ECHO;
<STRING>\"        ECHO; BEGIN(INITIAL);

%NAME     { printf("%s",getenv("LOGNAME")); }
%HOST     { printf("%s",getenv("HOST"));    }
%HOSTTYPE { printf("%s",getenv("HOSTTYPE"));}
%HOME     { printf("%s",getenv("HOME"));    }
