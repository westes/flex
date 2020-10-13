/* basic example, fully reentrant thread-safe version */
%{
 struct stats {
     int num_lines;
     int num_chars;
 };
%}
%option reentrant noyywrap
%option extra-type="struct stats"
%%
\n	{
     		struct stats ns = yyget_extra(yyscanner);
		++ns.num_lines; ++ns.num_chars;
		yyset_extra(ns, yyscanner);
	}
.       {
     		struct stats ns = yyget_extra(yyscanner);
		++ns.num_chars;
		yyset_extra(ns, yyscanner);
	}

%%

int main() {
        yyscan_t scanner;
	struct stats ns;

        yylex_init ( &scanner );
        yylex ( scanner );

	ns = yyget_extra(scanner);
        printf( "# of lines = %d, # of chars = %d\n",
                ns.num_lines, ns.num_chars);
        yylex_destroy ( scanner );
}
