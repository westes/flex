/* 
 * reject.lex: An example of REJECT and unput()
 *             misuse.
 */

%%
UNIX       { 
                unput('U'); unput('N'); unput('G'); unput('\0');
                REJECT;
           } 
GNU        printf("GNU is Not Unix!\n"); 
%%
