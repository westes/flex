
%option noyywrap noinput nounput
%option prefix="test"

%top{
  /*
  combined test if flex recognizes syntactically
  correct pairings of curly brackets and characters
  of curly brackets (closing curly brackets, in
  particular).
  */
 static int f(void)
 {
	int c = 0;
	if (c == '}')
	{
		return '{';
	}
	else
		return '}';
 }
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

