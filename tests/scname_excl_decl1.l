%option noyywrap noinput nounput

 /*
   This test checks if a name of a start condition
   which immediately follows '%x' is recognized as
   such by flex.
 */
%xA B
%%
.
%%
int main()
{
	int i = 0;

#ifndef A
	fprintf(stderr, "FAILED - '%%x' start condition 'A' not defined\n");
	i++;
#endif

	return i > 0;
}
