/*
 *
 * myname.lex : A sample Flex program
 *              that does token replacement.
 */

%%

%NAME     { printf("%s",getenv("LOGNAME")); }
%HOST     { printf("%s",getenv("HOST"));    }
%HOSTTYPE { printf("%s",getenv("HOSTTYPE"));}
%HOME     { printf("%s",getenv("HOME"));    }

%%

