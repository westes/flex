
%option noyywrap noinput nounput
%option prefix="test"

%top{
	/* string literal with closing curly
		bracket, broken by backslash at EOL */
	const char* string_literal = "abc }\
		def";
}

%%
.
%%

 /* add a simple main function,
	since we only want to check if
	the scanner code can be generated
	by flex
 */
int main()
{
	return 0;
}

