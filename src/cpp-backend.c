/* cpp-backend.c - C++ backend file for flex */

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


#include "flexdef.h"
#include "cpp-backend.h"

const int CPP_BACKEND_MAX_INDENT = 256;

const char *cpp_skel[] = {
#include "cpp-flex.h"
0
};

const char * cpp_get_int32_type ( struct flex_backend_t *b ) {
	return "flex_int32_t";
}

static const char * cpp_get_int16_type ( struct flex_backend_t *b ) {
	return "flex_int16_t";
}

static void cpp_open_block_comment ( struct flex_backend_t *b ) {
	fputs("/* ", stdout);
}

static void cpp_close_block_comment ( struct flex_backend_t *b ) {
	fputs(" */", stdout);
}

static void cpp_comment ( struct flex_backend_t *b, const char *c ) {
	b->indent(b);
	fprintf(stdout, "/* %s */", c);
}

static void cpp_record_separator ( struct flex_backend_t *b ) {
	fputs("},\n", stdout);
}

static void cpp_column_separator ( struct flex_backend_t *b ){
	fputs(", ", stdout);
}

static void cpp_newline ( struct flex_backend_t *b ) {
	fputs("\n", stdout);
}

static void cpp_increase_indent ( struct flex_backend_t *b ) {
	if ( b->indent_level < CPP_BACKEND_MAX_INDENT ) {
		b->indent_level += 1;
	}
}
	
static void cpp_decrease_indent ( struct flex_backend_t *b ) {
	if (b->indent_level > 0) {
		b->indent_level -= 1;
	}
}

static void cpp_indent ( struct flex_backend_t *b ) {
	int i = 0;
	while ( i < b->indent_level ) {
		fputs("\t", stdout);
		++i;
	}
}

static const char * cpp_get_trace_line_format ( struct flex_backend_t *b ) {
	return "#line %d \"%s\"\n";
}

static void cpp_line_directive_out ( struct flex_backend_t * b, FILE *output_file, char *path, int linenum ) {
	char   directive[MAXLINE*2], filename[MAXLINE];
	char   *s1, *s2, *s3;

	if (!ctrl.gen_line_dirs) {
		return;
	}
	
	/* char *infilename is in the global namespace */
	s1 = (path != NULL) ? path : infilename;

	if ((path != NULL) && !s1) {
		s1 = "<stdin>";
	}
    
	s2 = filename;
	s3 = &filename[sizeof (filename) - 2];

	while (s2 < s3 && *s1) {
		if (*s1 == '\\' || *s1 == '"') {
			/* Escape the '\' or '"' */
			*s2++ = '\\';
		}

		*s2++ = *s1++;
	}

	*s2 = '\0';

	if (path != NULL) {
		snprintf (directive, sizeof(directive), b->get_trace_line_format(b), linenum, filename);
	} else {
		snprintf (directive, sizeof(directive), b->get_trace_line_format(b), 0, filename);
	}

	/* If output_file is nil then we should put the directive in
	 * the accumulated actions.
	 */
	if (output_file) {
		fputs (directive, output_file);
	}
	else {
		add_action (directive);
	}
}

static void cpp_open_table ( struct flex_backend_t *b ) {
	fputs("{", stdout);
}

static void cpp_continue_table ( struct flex_backend_t *b ) {
	fputs("},\n", stdout);
}

static void cpp_close_table ( struct flex_backend_t *b ) {
	fputs("};\n", stdout);
}

static void cpp_relativize ( struct flex_backend_t *b, const char *s ) {
	fputs(s, stdout);
}

static void cpp_format_state_table_entry ( struct flex_backend_t * b, int t ) {
	b->indent(b);
	fprintf(stdout, "&yy_transition[%d],\n", t);
}

static void cpp_format_normal_state_case_arm ( struct flex_backend_t *b, int c ) {
	b->indent(b);
	fprintf(stdout, "case %d:", c);
}

static void cpp_format_eof_state_case_arm ( struct flex_backend_t *b, int c ) {
	b->indent(b);
	fprintf(stdout, "case YY_STATE_EOF(%d):", c);
}

static void cpp_eof_state_case_fallthrough ( struct flex_backend_t *b ) {
	b->indent(b);
	b->comment(b, "FALLTHROUGH");
}

static void cpp_eof_state_case_terminate ( struct flex_backend_t *b ) { 
	b->indent(b);
	fputs("yyterminate();\n", stdout);
}

static void cpp_take_yytext ( struct flex_backend_t *b ) {
	b->indent(b);
	fputs("YY_DO_BEFORE_ACTION; /* set up yytext */", stdout);
}

static void cpp_release_yytext ( struct flex_backend_t *b ) {
	b->indent(b);
	fputs( "*yy_cp = YY_G(yy_hold_char); /* undo effects of setting up yytext */", stdout);
}

static void cpp_format_char_rewind ( struct flex_backend_t *b, int c ) {
	b->indent(b);
	fprintf(stdout, "YY_G(yy_c_buf_p) = yy_cp -= %d;", c);
}

static void cpp_format_line_rewind ( struct flex_backend_t *b, int l ) {
	b->indent(b);
	fprintf(stdout, "YY_LINENO_REWIND_TO(yy_cp - %d);", l);
}

static void cpp_format_char_forward ( struct flex_backend_t *b, int c ) {
	b->indent(b);
	fprintf(stdout, "YY_G(yy_c_buf_p) = yy_cp = yy_bp + %d;", c);
}

static void cpp_format_line_forward ( struct flex_backend_t *b, int l ) {
	b->indent(b);
	fprintf(stdout, "YY_LINENO_REWIND_TO(yy_bp + %d);", l);
}

static void cpp_format_byte_const ( struct flex_backend_t *b, const char *n, const int c ) {
	fprintf(stdout, "#define %s %d\n", n, c);
}

static void cpp_format_state_const ( struct flex_backend_t *b, const char *n, const int s ) {
	fprintf(stdout, "#define %s %d\n", n, s);
}

static void cpp_format_uint_const ( struct flex_backend_t *b, const char *n, const unsigned int u ) {
	fprintf(stdout, "#define %s %u\n", n, u);
}

static void cpp_format_bool_const ( struct flex_backend_t *b, const char *n, const int t ){
	fprintf(stdout, "#define %s %d\n", n, t);
}

static void cpp_format_const ( struct flex_backend_t *b, const char *n, const char *v ) {
	fprintf(stdout, "#define %s %s\n", n, v);
}

static void cpp_format_offset_type ( struct flex_backend_t *b, const char *t ) {
	b->format_const(b, "YY_OFFSET_TYPE", t);
}

static void cpp_format_yy_decl ( struct flex_backend_t *b, const char *d ) {
	b->format_const(b, "YY_DECL", d);
}

static void cpp_format_userinit ( struct flex_backend_t *b, const char *d ) {
	b->format_const(b, "YY_USER_INIT", d);
}

static void cpp_format_rule_setup ( struct flex_backend_t *b ) {
	b->relativize(b, "YY_RULE_SETUP");
	b->newline(b);
}

static void cpp_format_user_preaction ( struct flex_backend_t *b, const char *d ) {
	b->format_const(b, "YY_USER_ACTION", d);
}

static void cpp_format_state_case_break ( struct flex_backend_t *b ) {
	b->indent(b);
	if (!ctrl.postaction) {
		fputs("/*LINTED*/break;", stdout);
	} 
	else {
		fputs(ctrl.postaction, stdout);
	}
}

static void cpp_format_user_postaction ( struct flex_backend_t *b, const char *d ) {
	if (d != NULL) {
		b->format_const(b, "YY_STATE_CASE_BREAK", d);
	}
	else {
		b->format_const(b, "YY_STATE_CASE_BREAK", "/*LINTED*/break;");
	}
}

static void cpp_format_fatal_error ( struct flex_backend_t *b, const char *e ) {
	b->indent(b);
	fprintf(stdout, "yypanic(%s M4_YY_CALL_LAST_ARG);", e);
}

static void cpp_echo ( struct flex_backend_t *b ) {
	b->indent(b);
	fputs("yyecho();", stdout);
}

static void cpp_format_yyterminate ( struct flex_backend_t *b, const char *d ) {
	if (d != NULL) {
		b->format_const(b, "yyterminate", d);
	}
	else {
		b->format_const(b, "yyterminate", "return NULL");
	}
}

static void cpp_format_yyreject ( struct flex_backend_t *b ) {
	b->indent(b);
	fputs("yyreject()", stdout);
}

struct flex_backend_t cpp_backend = {
	.skel = cpp_skel,
	.indent_level = 0,
	.get_int32_type = cpp_get_int32_type,
	.get_int16_type = cpp_get_int16_type,
	.open_block_comment = cpp_open_block_comment,
	.close_block_comment = cpp_close_block_comment,
	.comment = cpp_comment,
	.record_separator = cpp_record_separator,
	.column_separator = cpp_column_separator,
	.newline = cpp_newline,
	.increase_indent = cpp_increase_indent,
	.decrease_indent = cpp_decrease_indent,
	.indent = cpp_indent,
	.get_trace_line_format = cpp_get_trace_line_format,
	.line_directive_out = cpp_line_directive_out,
	.open_table = cpp_open_table,
	.continue_table = cpp_continue_table,
	.close_table = cpp_close_table,
	.relativize = cpp_relativize,
	.format_state_table_entry = cpp_format_state_table_entry,
	.format_normal_state_case_arm = cpp_format_normal_state_case_arm,
	.format_eof_state_case_arm = cpp_format_eof_state_case_arm,
	.eof_state_case_fallthrough = cpp_eof_state_case_fallthrough,
	.eof_state_case_terminate = cpp_eof_state_case_terminate,
	.take_yytext = cpp_take_yytext,
	.release_yytext = cpp_release_yytext,
	.format_char_rewind = cpp_format_char_rewind,
	.format_line_rewind = cpp_format_line_rewind,
	.format_char_forward = cpp_format_char_forward,
	.format_line_forward = cpp_format_line_forward,
	.format_byte_const = cpp_format_byte_const,
	.format_state_const = cpp_format_state_const,
	.format_uint_const = cpp_format_uint_const,
	.format_bool_const = cpp_format_bool_const,
	.format_const = cpp_format_const,
	.format_offset_type = cpp_format_offset_type,
	.format_yy_decl = cpp_format_yy_decl,
	.format_userinit = cpp_format_userinit,
	.format_rule_setup = cpp_format_rule_setup,
	.format_user_preaction = cpp_format_user_preaction,
	.format_state_case_break = cpp_format_state_case_break,
	.format_user_postaction = cpp_format_user_postaction,
	.format_fatal_error = cpp_format_fatal_error,
	.echo = cpp_echo,
	.format_yyterminate = cpp_format_yyterminate,
	.format_yyreject = cpp_format_yyreject
};

