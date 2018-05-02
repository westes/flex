 /*
    This test checks if flex honours 'no' options flags,
    in the following obscure %option section, in particular:

    %option noyylineno <<< last option is negated
    %optionnoyywrap    <<< first option is negated
                            and *no* blank after
                            token '%option' which
                            makes flex 2.6.4 ignore
                            the negation.

    Return code of program is 1 if flex fails to allow for
    'no' flag, 0 otherwise.

    Compile with DEBUG=1 (e.g. 'gcc -DDEBUG') to have program
    emit verbose information.
 */
%option prefix="test"
%option noyylineno
%optionnoyywrap

%%
    #ifdef testwrap
        /* flex correctly parsed the option 'noyywrap' */
        #define ADJ 0
    #else
        /* add definition of testwrap which flex forgot */
        #define ADJ 1
        #define testwrap() (1)
    #endif
.

%%

int main(int argc, char** argv)
{
#if DEBUG
    if ( ADJ == 0 )
        printf("OK - flex defined 'testwrap' as expected.\n");
    else
        printf("FAILED - flex %d.%d.%d forgot to define 'testwrap'.\n"
            , YY_FLEX_MAJOR_VERSION
            , YY_FLEX_MINOR_VERSION
            , YY_FLEX_SUBMINOR_VERSION
            );
#endif

    return ADJ;
}
