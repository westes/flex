/* yylex - scanner front-end for flex */

/*  Copyright (c) 1990 The Regents of the University of California. */
/*  All rights reserved. */

/*  This code is derived from software contributed to Berkeley by */
/*  Vern Paxson. */

/*  The United States Government has rights in this work pursuant */
/*  to contract no. DE-AC03-76SF00098 between the United States */
/*  Department of Energy and the University of California. */

/*  This file is part of flex. */

/*  Redistribution and use in source and binary forms, with or without */
/*  modification, are permitted provided that the following conditions */
/*  are met: */

/*  1. Redistributions of source code must retain the above copyright */
/*     notice, this list of conditions and the following disclaimer. */
/*  2. Redistributions in binary form must reproduce the above copyright */
/*     notice, this list of conditions and the following disclaimer in the */
/*     documentation and/or other materials provided with the distribution. */

/*  Neither the name of the University nor the names of its contributors */
/*  may be used to endorse or promote products derived from this software */
/*  without specific prior written permission. */

/*  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR */
/*  IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED */
/*  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR */
/*  PURPOSE. */

#include <ctype.h>
#include "flexdef.h"
#include "parse.h"


/* yylex - scan for a regular expression token */
extern char *yytext;
extern FILE *yyout;
bool no_section3_escape = false;
int     yylex (void)
{
	int     toktype;
	static int beglin = false;

	if (eofseen) {
		toktype = EOF;
        } else {
		toktype = flexscan ();
        }
	if (toktype == EOF || toktype == 0) {
		eofseen = 1;

		if (sectnum == 1) {
			synerr (_("premature EOF"));
			sectnum = 2;
			toktype = FLEX_TOK_SECTEND;
		}

		else
			toktype = 0;
	}

	if (trace) {
		if (beglin) {
			fprintf (stderr, "%d\t", num_rules + 1);
			beglin = 0;
		}

		switch (toktype) {
		case '<':
		case '>':
		case '^':
		case '$':
		case '"':
		case '[':
		case ']':
		case '{':
		case '}':
		case '|':
		case '(':
		case ')':
		case '-':
		case '/':
		case '\\':
		case '?':
		case '.':
		case '*':
		case '+':
		case ',':
			(void) putc (toktype, stderr);
			break;

		case '\n':
			(void) putc ('\n', stderr);

			if (sectnum == 2)
				beglin = 1;

			break;

		case FLEX_TOK_SCDECL:
			fputs ("%s", stderr);
			break;

		case FLEX_TOK_XSCDECL:
			fputs ("%x", stderr);
			break;

		case FLEX_TOK_SECTEND:
			fputs ("%%\n", stderr);

			/* We set beglin to be true so we'll start
			 * writing out numbers as we echo rules.
			 * flexscan() has already assigned sectnum.
			 */
			if (sectnum == 2)
				beglin = 1;

			break;

		case FLEX_TOK_NAME:
			fprintf (stderr, "'%s'", nmstr);
			break;

		case FLEX_TOK_CHAR:
			switch (yylval) {
			case '<':
			case '>':
			case '^':
			case '$':
			case '"':
			case '[':
			case ']':
			case '{':
			case '}':
			case '|':
			case '(':
			case ')':
			case '-':
			case '/':
			case '\\':
			case '?':
			case '.':
			case '*':
			case '+':
			case ',':
				fprintf (stderr, "\\%c", yylval);
				break;

			default:
				if (!isascii (yylval) || !isprint (yylval)) {
					if(trace_hex)
						fprintf (stderr, "\\x%02x", (unsigned int) yylval);
					else
						fprintf (stderr, "\\%.3o", (unsigned int) yylval);
				} else
					(void) putc (yylval, stderr);
				break;
			}

			break;

		case FLEX_TOK_NUMBER:
			fprintf (stderr, "%d", yylval);
			break;

		case FLEX_TOK_PREVCCL:
			fprintf (stderr, "[%d]", yylval);
			break;

		case FLEX_TOK_EOF_OP:
			fprintf (stderr, "<<EOF>>");
			break;

		case FLEX_TOK_OPTION:
			fprintf (stderr, "%s ", yytext);
			break;

		case FLEX_TOK_OUTFILE:
		case FLEX_TOK_PREFIX:
		case FLEX_TOK_CCE_ALNUM:
		case FLEX_TOK_CCE_ALPHA:
		case FLEX_TOK_CCE_BLANK:
		case FLEX_TOK_CCE_CNTRL:
		case FLEX_TOK_CCE_DIGIT:
		case FLEX_TOK_CCE_GRAPH:
		case FLEX_TOK_CCE_LOWER:
		case FLEX_TOK_CCE_PRINT:
		case FLEX_TOK_CCE_PUNCT:
		case FLEX_TOK_CCE_SPACE:
		case FLEX_TOK_CCE_UPPER:
		case FLEX_TOK_CCE_XDIGIT:
			fprintf (stderr, "%s", yytext);
			break;

		case 0:
			fprintf (stderr, _("End Marker\n"));
			break;

		default:
			fprintf (stderr,
				 _
				 ("*Something Weird* - tok: %d val: %d\n"),
				 toktype, yylval);
			break;
		}
	}

	return toktype;
}
