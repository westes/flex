/*
 * This file is part of flex.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * Neither the name of the University nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED `AS IS' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE.
 */

%{
/* An <<EOF>> rule whose action is '|' shares the action of the rule after
   it. It has no action text of its own, so flex must not open a quoted
   section for it that nothing closes: m4 used to fail with "end of file
   in string" on such a scanner. */
#include <stdio.h>
%}

%option 8bit nomain noyywrap nounput noinput
%x SC

%%

a        { BEGIN(SC); }
b        { return 1; }
<SC>x    { return 2; }
<SC><<EOF>> |
<SC>y    { return 3; }
<<EOF>> |
c        { return 4; }

%%

int main(void);

int
main (void)
{
	int ret;

	/* "c" and the end of input run the same action. */
	yy_scan_string("bc");
	if ((ret = yylex()) != 1 || (ret = yylex()) != 4 ||
	    (ret = yylex()) != 4) {
		printf("TEST FAILED: outside a start condition, got %d.\n", ret);
		return 1;
	}

	/* The same inside a start condition. */
	yy_scan_string("ay");
	if ((ret = yylex()) != 3 || (ret = yylex()) != 3) {
		printf("TEST FAILED: inside a start condition, got %d.\n", ret);
		return 1;
	}

	printf("TEST RETURNING OK.\n");
	return 0;
}
