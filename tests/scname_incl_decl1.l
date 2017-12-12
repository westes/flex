%option noyywrap noinput nounput

 /*
   This test checks if a name of a start condition
   which immediately follows '%s' is recognized as
   such by flex.
 */
%sA B
%%
.
%%
int main()
{
	int i = 0;

#ifndef A
	fprintf(stderr, "FAILED - '%%s' start condition 'A' not defined\n");
	i++;
#endif

	return i > 0;
}
