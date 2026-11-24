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

/* Test that the code flex generates between rule actions, and for the
   default rule, is attributed to the generated file and not to the .l
   file. YY_USER_ACTION runs in generated code at the start of every
   rule, so __FILE__ there must name the generated C file, whichever
   rule ran before it. */

%{
#include <stdio.h>
#include <string.h>

static int bad;

#define YY_USER_ACTION \
	if (strstr(__FILE__, ".l") != NULL && \
	    strstr(__FILE__, ".lo") == NULL) { \
		fprintf(stderr, "rule for \"%s\" runs in %s\n", \
		        yytext, __FILE__); \
		bad = 1; \
	}
%}

%option 8bit nomain noyywrap nounput noinput

%%

a    { }
b    { }
c    ;
d    {
         /* a multi-line action */
     }
e    |
f    { }
<<EOF>> { return 0; }

%%

int main(void);

int
main (void)
{
	/* "g" and "\n" are caught by the default rule. */
	yy_scan_string("abcdefg\n");
	while (yylex() != 0)
		;
	if (bad)
		return 1;
	printf("TEST RETURNING OK.\n");
	return 0;
}
