/* c99-backend.c - C99 backend file for flex */

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
#include "c99-backend.h"

const int C99_BACKEND_MAX_INDENT = 256;

const char *c99_skel[] = {
#include "c99-flex.h"
0
};

/* For C99, emit the typedef names from stdint.h */
const char * c99_get_int32_type ( const struct flex_backend_t *b ) {
	return "int32_t";
}

/* For C99, emit the typedef names from stdint.h */
static const char * c99_get_int16_type ( const struct flex_backend_t *b ) {
	return "int16_t";
}

/* Emit the name of the datatype used in the NULTRANS table.
   The datatype of the table depends on various option settings
   and the skeleton in use.
 */
static const char * c99_get_state_type ( const struct flex_backend_t *b ) {
	/* cpp-flex.skl defines transition states as pointers to 
	   struct yy_trans_info when the FULLSPD option is enabled. 
	   Otherwise, it just uses the int32 type.
	*/
	return (ctrl.fullspd) ? "struct yy_trans_info*" : b->get_int32_type(b);
}

static const char * c99_get_packed_type (const struct flex_backend_t *b, struct packtype_t *p) {
	switch(p->width) {
		case 32: return b->get_int32_type(b);
		case 16: return b->get_int16_type(b);
		default: flexerror("unsupported packed data width requested\n");
		         break;
	}
}

/* TODO: Indent? */
static void c99_open_block_comment ( const struct flex_backend_t *b ) {
	fputs("/* ", stdout);
}

/* TODO: Indent? */
static void c99_close_block_comment ( const struct flex_backend_t *b ) {
	fputs(" */", stdout);
}

static const char * c99_get_comment ( const struct flex_backend_t *b, const char *c ) {
	static const char *format = "/* %s */";
	static char directive[MAXLINE*2] = {0};

	snprintf(directive, sizeof(directive), format, c);
	return directive;
}

static void c99_record_separator ( const struct flex_backend_t *b ) {
	/* Expected to ocurr at the end of a line, so I'm not indenting. */
	fputs("},\n", stdout);
}

static void c99_column_separator ( const struct flex_backend_t *b ){
	/* Expected to ocurr at the end of a line, so don't indent. */
	fputs(", ", stdout);
}

static void c99_newline ( const struct flex_backend_t *b ) {
	/* Expected to ocurr at the end of a line, so don't indent. */
	fputs("\n", stdout);
}

static void c99_increase_indent ( const struct flex_backend_t *b ) {
	if ( b->indent_level < C99_BACKEND_MAX_INDENT ) {
		/* ++ will require more parens to clarify */
		((struct flex_backend_t *)b)->indent_level += 1;
	}
}
	
static void c99_decrease_indent ( const struct flex_backend_t *b ) {
	if (b->indent_level > 0) {
		/* -- will require more parens to clarify */
		((struct flex_backend_t *)b)->indent_level -= 1;
	}
}

static void c99_indent ( const struct flex_backend_t *b ) {
	int i = 0;
	while ( i < b->indent_level ) {
		fputs("\t", stdout);
		++i;
	}
}

/* Return a format string appropriate for the skeleton language.
   The format string will perform an equivalent function to the CPP line directive.
   That is, it will tell target language compiler what .l source line the target source
   corresponds to when the compiler generates warnings and errors.

   This method does not provide the arguments to the format string. It just provides the string.
*/
static const char * c99_get_trace_line_format ( const struct flex_backend_t *b ) {
	return "#line %d \"%s\"\n";
}

/* TODO: indent? */
static void c99_open_table ( const struct flex_backend_t *b ) {
	fputs("{", stdout);
}

static void c99_continue_table ( const struct flex_backend_t *b ) {
	/* Expected to ocurr at the end of a line, so don't indent. */
	fputs("},\n", stdout);
}

/* TODO: indent? */
static void c99_close_table ( const struct flex_backend_t *b ) {
	fputs("};\n", stdout);
}

/* Format an entry into a data table. */
static void c99_format_data_table_entry ( const struct flex_backend_t * b, int t ) {
	/* Expected to occur in a block format, so don't indent. */
	fprintf(stdout, "%5d", t);
}

/* Format an entry from the transition table into the state table. */
static void c99_format_state_table_entry ( const struct flex_backend_t * b, int t ) {
	b->indent(b);
	fprintf(stdout, "&yy_transition[%d],\n", t);
}

/* Generate a case for the main state switch (in C/CXX). 
   Other target languages may use another selection syntax. Basically, each arm matches
   the character under examination, treated as a number.
*/
static const char * c99_get_normal_state_case_arm ( const struct flex_backend_t *b, int c ) {
	static const char *format = "case %d: ";
	static char directive[MAXLINE*2] = {0};

	snprintf (directive, sizeof(directive), format, c);
	return directive;
}

/* Generate the special case arm for EOF. 
   This lives in the body of yyinput and determines whether/when to switch to the next buffer.
*/
static const char * c99_get_eof_state_case_arm ( const struct flex_backend_t *b, const char * const c ) {
	static const char *format = "case YY_STATE_EOF(%s): ";
	static char directive[MAXLINE*2] = {0};

	snprintf (directive, sizeof(directive), format, c);
	return directive;
}

/* Generate the special action FALLTHROUGH. 
   This is just a comment in C/CXX, but a few languages require a keyword for fallthrough logic.
*/
static void c99_eof_state_case_fallthrough ( const struct flex_backend_t *b ) {
	b->indent(b);
	b->comment(b, "FALLTHROUGH");
}

/* Generate the special action terminate. */
static void c99_eof_state_case_terminate ( const struct flex_backend_t *b ) { 
	b->indent(b);
	fputs("yyterminate();\n", stdout);
}

/* Generate the action preamble. */
static const char * c99_get_take_yytext( const struct flex_backend_t *b ) {
	static const char *directive = "YY_DO_BEFORE_ACTION; /* set up yytext */";
	return directive;
}

/* Generate the action postamble. */
static const char * c99_get_release_yytext( const struct flex_backend_t *b ) {
	static const char *directive = "*yy_cp = YY_G(yy_hold_char); /* undo effects of setting up yytext */";
	return directive;
}

/* Generate the buffer rewind sub-action. */
static const char * c99_get_char_rewind( const struct flex_backend_t *b, int c ) {
	static const char *format = "YY_G(yy_c_buf_p) = yy_cp -= %d;";
	static char directive[MAXLINE*2] = {0};

	snprintf (directive, sizeof(directive), format, c);
	return directive;
}

/* Generate the line rewind sub-action. */
static const char * c99_get_line_rewind( const struct flex_backend_t *b, int l ) {
	static const char *format = "YY_LINENO_REWIND_TO(yy_cp - %d);";
	static char directive[MAXLINE*2] = {0};

	snprintf (directive, sizeof(directive), format, l);
	return directive;
}

/* Generate the buffer skip sub-action. */
static const char * c99_get_char_forward( const struct flex_backend_t *b, int c ) {
	static const char *format = "YY_G(yy_c_buf_p) = yy_cp = yy_bp + %d;";
	static char directive[MAXLINE*2] = {0};

	snprintf (directive, sizeof(directive), format, c);
	return directive;
}

/* Generate the line skip sub-action. */
static const char * c99_get_line_forward( const struct flex_backend_t *b, int l ) {
	static const char *format = "YY_LINENO_REWIND_TO(yy_bp + %d);";
	static char directive[MAXLINE*2] = {0};

	snprintf (directive, sizeof(directive), format, l);
	return directive;
}

/* Define a byte-width constant. */
static void c99_format_byte_const ( const struct flex_backend_t *b, const char *n, const int c ) {
	fprintf(stdout, "#define %s %d\n", n, c);
}

/* Define a state constant. */
static void c99_format_state_const ( const struct flex_backend_t *b, const char *n, const int s ) {
	fprintf(stdout, "#define %s %d\n", n, s);
}

/* Define a size constant. 
   TODO: It would be better if s were unsigned, but Flex currently counts in signed ints.
*/
static void c99_format_size_const ( const struct flex_backend_t *b, const char *n, const int s ) {
	fprintf(stdout, "#define %s %d\n", n, s);
}

/* Define a uint constant. */
static void c99_format_uint_const ( const struct flex_backend_t *b, const char *n, const unsigned int u ) {
	fprintf(stdout, "#define %s %u\n", n, u);
}

/* Define a boolean constant. */
static void c99_format_bool_const ( const struct flex_backend_t *b, const char *n, const int t ){
	fprintf(stdout, "#define %s %d\n", n, t);
}

/* Define a string constant. */
static const char * c99_get_const ( const struct flex_backend_t *b, const char *n, const char *v ) {
	static const char *format = "#define %s %s\n";
	static char directive[MAXLINE*2] = {0};

	snprintf(directive, sizeof(directive), format, n, v);
	return directive;
}

/* Define a constant used by the skeleton. */
static void c99_format_offset_type ( const struct flex_backend_t *b, const char *t ) {
	b->format_const(b, "YY_OFFSET_TYPE", t);
}

/* Define a constant used by the skeleton. */
static void c99_format_yy_decl ( const struct flex_backend_t *b, const char *d ) {
	b->format_const(b, "YY_DECL", d);
}

/* Define a constant used by the skeleton. */
static void c99_format_userinit ( const struct flex_backend_t *b, const char *d ) {
	b->format_const(b, "YY_USER_INIT", d);
}

/* Generate the rule_setup preamble. */
static const char * c99_get_rule_setup ( const struct flex_backend_t *b ) {
	static const char *directive = "YY_RULE_SETUP";
	return directive;
}

/* Generate the user_action constant, if needed. */
static const char * c99_get_user_preaction ( const struct flex_backend_t *b, const char *d  ) {
	return b->get_const(b, "YY_USER_ACTION", d);
}

/* End a state case arm, optionally inserting user postactions. 

   TODO: Why can't this use YY_STATE_CASE_BREAK from format_user_postaction?
*/
static const char * c99_get_state_case_break ( const struct flex_backend_t *b ) {
	static const char *directive = "/*LINTED*/break;";
	if (!ctrl.postaction) {
		return directive;
	} 
	else {
		return ctrl.postaction;
	}
}

/* Generate the definition of the STATE_CASE_BREAK end of action. */
static const char * c99_get_user_postaction ( const struct flex_backend_t *b, const char *d ) {
	if (d != NULL) {
		return b->get_const(b, "YY_STATE_CASE_BREAK", d);
	}
	else {
		return b->get_const(b, "YY_STATE_CASE_BREAK", "/*LINTED*/break;");
	}
}

/* Generate the fatal_error action. */
static const char * c99_get_fatal_error ( const struct flex_backend_t *b, const char *e ) {
	static const char *format = "yypanic(%s, yyscanner);";
	static char directive[MAXLINE*2] = {0};

	snprintf (directive, sizeof(directive), format, e);
	return directive;
}

/* Generate the echo action. */
static const char * c99_get_echo ( const struct flex_backend_t *b ) {
	static const char *directive = "yyecho();";

	return directive;
}

/* Generate the definition of the terminate special action. */
static const char * c99_get_yyterminate ( const struct flex_backend_t *b, const char *d ) {
	static const char * format = "%s /* yyterminate */";
	static char directive[MAXLINE*2] = {0};

	if (d != NULL) {
		snprintf(directive, sizeof(directive), format, d);
		return b->get_const(b, "yyterminate", directive);
	}
	else {
		return b->get_const(b, "yyterminate", "return NULL /* yyterminate */");
	}
}

/* Generate the reject special action. */
static const char * c99_get_yyreject ( const struct flex_backend_t *b ) {
	static const char *directive = "yyreject()";
	return directive;
}

/* Construct the c99_backend method table.
   This follows the definition in skeletons.h. 
   cpp-backends.h provides a handle to this structure with external linkage.
   skeletons.c imports that handle to access these methods.
   That module makes this implementation available to others as an opaque singleton.

   Note that indentation level is managed indepentently for each backend.
*/
struct flex_backend_t c99_backend = {
	.skel = c99_skel,
	.indent_level = 0,
	.get_int32_type = c99_get_int32_type,
	.get_int16_type = c99_get_int16_type,
	.get_state_type = c99_get_state_type,
	.get_packed_type = c99_get_packed_type,
	.open_block_comment = c99_open_block_comment,
	.close_block_comment = c99_close_block_comment,
	.get_comment = c99_get_comment,
	.comment = _format_comment,
	.record_separator = c99_record_separator,
	.column_separator = c99_column_separator,
	.newline = c99_newline,
	.increase_indent = c99_increase_indent,
	.decrease_indent = c99_decrease_indent,
	.indent = c99_indent,
	.get_trace_line_format = c99_get_trace_line_format,
	.line_directive_out = _format_line_directive_out,
	.open_table = c99_open_table,
	.continue_table = c99_continue_table,
	.close_table = c99_close_table,
	.verbatim = _verbatim,
	.format_data_table_entry = c99_format_data_table_entry,
	.format_state_table_entry = c99_format_state_table_entry,
	.get_normal_state_case_arm = c99_get_normal_state_case_arm,
	.format_normal_state_case_arm = _format_normal_state_case_arm,
	.get_eof_state_case_arm = c99_get_eof_state_case_arm,
	.format_eof_state_case_arm = _format_eof_state_case_arm,
	.eof_state_case_fallthrough = c99_eof_state_case_fallthrough,
	.eof_state_case_terminate = c99_eof_state_case_terminate,
	.get_take_yytext = c99_get_take_yytext,
	.format_take_yytext = _format_take_yytext,
	.get_release_yytext = c99_get_release_yytext,
	.format_release_yytext = _format_release_yytext,
	.get_char_rewind = c99_get_char_rewind,
	.format_char_rewind = _format_char_rewind,
	.get_line_rewind = c99_get_line_rewind,
	.format_line_rewind = _format_line_rewind,
	.get_char_forward = c99_get_char_forward,
	.format_char_forward = _format_char_forward,
	.get_line_forward = c99_get_line_forward,
	.format_line_forward = _format_line_forward,
	.format_byte_const = c99_format_byte_const,
	.format_state_const = c99_format_state_const,
	.format_size_const = c99_format_size_const,
	.format_uint_const = c99_format_uint_const,
	.format_bool_const = c99_format_bool_const,
	.get_const = c99_get_const,
	.format_const = _format_const,
	.format_offset_type = c99_format_offset_type,
	.format_yy_decl = c99_format_yy_decl,
	.format_userinit = c99_format_userinit,
	.get_rule_setup = c99_get_rule_setup,
	.format_rule_setup = _format_rule_setup,
	.get_user_preaction = c99_get_user_preaction,
	.format_user_preaction = _format_user_preaction,
	.get_state_case_break = c99_get_state_case_break,
	.format_state_case_break = _format_state_case_break,
	.get_user_postaction = c99_get_user_postaction,
	.format_user_postaction = _format_user_postaction,
	.get_fatal_error = c99_get_fatal_error,
	.format_fatal_error = _format_fatal_error,
	.get_echo = c99_get_echo,
	.echo = _echo,
	.get_yyterminate = c99_get_yyterminate,
	.format_yyterminate = _format_yyterminate,
	.get_yyreject = c99_get_yyreject,
	.format_yyreject = _format_yyreject,
	.filter_define_name = _filter_define_name,
	.filter_define_close = _filter_define_close,
	.filter_define_vars = _filter_define_vars,
	.filter_define_vard = _filter_define_vard,
	.filter_call_macro = _filter_call_macro
};

