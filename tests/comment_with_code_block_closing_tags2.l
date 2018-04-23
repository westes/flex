%option noyywrap noinput nounput

%%

%{
	/* This is a comment with closing tags
	   of a code block, one at the beginning
	   of the line, one within the line.

	   Flex should not consider both of them
	   as closing tag of this code block.

	   %} ... closing tag within c-comment

%} ... closing tag within c-comment at BOL
	*/
%}
.

%%

/*
	This test is about processing this input
	file, not about compilation. So we add
	a simple main function.
*/
int main(void)
{
	return 0;
}
