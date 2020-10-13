/* 
 * reject.lex: An example of yyreject() and yyunput()
 *             misuse.
 */

%%
UNIX       { 
                yyunput('U'); yyunput('N'); yyunput('G'); yyunput('\0');
                yyreject();
           } 
GNU        printf("GNU is Not Unix!\n"); 
%%
