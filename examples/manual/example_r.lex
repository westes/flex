/* basic example - flawed reentrant version with global */
%{
        int num_lines = 0, num_chars = 0;
%}
%option reentrant noyywrap
%%
\n      ++num_lines; ++num_chars;
.       ++num_chars;

%%

int main() {
        yyscan_t scanner;

        yylex_init ( &scanner );
        yylex ( scanner );
        yylex_destroy ( scanner );

        printf( "# of lines = %d, # of chars = %d\n",
                num_lines, num_chars );
}
