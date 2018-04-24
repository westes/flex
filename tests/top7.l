
%option noyywrap noinput nounput
%option prefix="test"

%top{
	/*
		closing tag of top section is %}
	*/
%}

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

