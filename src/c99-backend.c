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

static void c99_comment ( const struct flex_backend_t *b, const char *c ) {
	b->indent(b);
	fprintf(stdout, "/* %s */", c);
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


/* Combine the format string from *_get_trace_line_format with its arguments. */
static void c99_line_directive_out ( const struct flex_backend_t * b, FILE *output_file, char *path, int linenum ) {
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

/* Intended to emit a macro call in C/CXX.
   Can also emit a bare string.
 */
static void c99_verbatim ( const struct flex_backend_t *b, const char *s ) {
	if (s)
		fputs(s, stdout);
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
static void c99_format_normal_state_case_arm ( const struct flex_backend_t *b, int c ) {
	b->indent(b);
	fprintf(stdout, "case %d: ", c);
}

/* Generate the special case arm for EOF. 
   This lives in the body of yyinput and determines whether/when to switch to the next buffer.
*/
static void c99_format_eof_state_case_arm ( const struct flex_backend_t *b, const char * const c ) {
	b->indent(b);
	fprintf(stdout, "case YY_STATE_EOF(%s): ", c);
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
static void c99_take_yytext ( const struct flex_backend_t *b ) {
	b->indent(b);
	fputs("YY_DO_BEFORE_ACTION; /* set up yytext */", stdout);
}

/* Generate the action postamble. */
static void c99_release_yytext ( const struct flex_backend_t *b ) {
	b->indent(b);
	fputs( "*yy_cp = YY_G(yy_hold_char); /* undo effects of setting up yytext */", stdout);
}

/* Generate the buffer rewind sub-action. */
static void c99_format_char_rewind ( const struct flex_backend_t *b, int c ) {
	b->indent(b);
	fprintf(stdout, "YY_G(yy_c_buf_p) = yy_cp -= %d;", c);
}

/* Generate the line rewind sub-action. */
static void c99_format_line_rewind ( const struct flex_backend_t *b, int l ) {
	b->indent(b);
	fprintf(stdout, "YY_LINENO_REWIND_TO(yy_cp - %d);", l);
}

/* Generate the buffer skip sub-action. */
static void c99_format_char_forward ( const struct flex_backend_t *b, int c ) {
	b->indent(b);
	fprintf(stdout, "YY_G(yy_c_buf_p) = yy_cp = yy_bp + %d;", c);
}

/* Generate the line skip sub-action. */
static void c99_format_line_forward ( const struct flex_backend_t *b, int l ) {
	b->indent(b);
	fprintf(stdout, "YY_LINENO_REWIND_TO(yy_bp + %d);", l);
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
static void c99_format_const ( const struct flex_backend_t *b, const char *n, const char *v ) {
	fprintf(stdout, "#define %s %s\n", n, v);
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

/* Inject the rule_setup macro call where needed. */
static void c99_format_rule_setup ( const struct flex_backend_t *b ) {
	b->verbatim(b, "YY_RULE_SETUP");
	b->newline(b);
}

/* Define the user_action constant, if needed. */
static void c99_format_user_preaction ( const struct flex_backend_t *b, const char *d ) {
	b->format_const(b, "YY_USER_ACTION", d);
}

/* End a state case arm, optionally inserting user postactions. 

   TODO: Why can't this use YY_STATE_CASE_BREAK from format_user_postaction?
*/
static void c99_format_state_case_break ( const struct flex_backend_t *b ) {
	b->indent(b);
	if (!ctrl.postaction) {
		fputs("/*LINTED*/break;", stdout);
	} 
	else {
		fputs(ctrl.postaction, stdout);
	}
}

/* Generate the definition of the STATE_CASE_BREAK end of action. */
static void c99_format_user_postaction ( const struct flex_backend_t *b, const char *d ) {
	if (d != NULL) {
		b->format_const(b, "YY_STATE_CASE_BREAK", d);
	}
	else {
		b->format_const(b, "YY_STATE_CASE_BREAK", "/*LINTED*/break;");
	}
}

/* Generate the fatal_error action. */
static void c99_format_fatal_error ( const struct flex_backend_t *b, const char *e ) {
	b->indent(b);
	fprintf(stdout, "yypanic(%s M4_YY_CALL_LAST_ARG);", e);
}

/* Generate the echo action. */
static void c99_echo ( const struct flex_backend_t *b ) {
	b->indent(b);
	fputs("yyecho();", stdout);
}

/* Generate the definition of the terminate special action. */
static void c99_format_yyterminate ( const struct flex_backend_t *b, const char *d ) {
	if (d != NULL) {
		b->format_const(b, "yyterminate", d);
	}
	else {
		b->format_const(b, "yyterminate", "return NULL");
	}
}

/* Generate the reject special action. */
static void c99_format_yyreject ( const struct flex_backend_t *b ) {
	b->indent(b);
	fputs("yyreject()", stdout);
}

/* Define a symbol used by the output filter system. 
   Optionally, leave the definition open to encompass a block of verbatim output.
*/
static void c99_filter_define_name ( const struct flex_backend_t *b, const char *n, const int leave_open ) {
	b->verbatim(b, "m4_define([[");
	b->verbatim(b, n);
	b->verbatim(b, "]], [[");
	if (leave_open)
		b->verbatim(b, "m4_dnl");
	else
		b->verbatim(b, "]])m4_dnl");
	b->newline(b);
}

/* Close a filter symbol definition that was left open by a call to filter_define_name. 
   Optionally, provide a final string of verbatim output to emit before closing the definition block.
*/
static void c99_filter_define_close (const struct flex_backend_t *b, const char *v) {
	b->verbatim(b, v);
	b->verbatim(b, "]])m4_dnl");
	b->newline(b);
}

/* Define a variable used by the output filter system. 
   Provide a string value the filter will substitue for the variable when it is encountered
   later in the output. 
*/
static void c99_filter_define_vars ( const struct flex_backend_t *b, const char *n, const char *v ) {
	b->filter_define_name(b, n, true);
	b->filter_define_close(b, v);
}

/* Define a variable used by the output filter system. 
   Provide a numeric value the filter will substitue for the variable when it is encountered
   later in the output. 
*/
static void c99_filter_define_vard ( const struct flex_backend_t *b, const char *n, const int v ) {
	b->filter_define_name(b, n, true);
	fprintf(stdout, "%d", v);
	b->filter_define_close(b, NULL);
}

/* Format a macro replacement through the output filter system.
   Filter macros are defined like variables. The syntax for defining a filter macro depends on the 
   filter chain in use.
   
   This example assumes the M4 filter chain where: every variable is a macro; the tokens following
   the name are substituted for the macro name; if the first token following the name is an OPAREN,
   it is followed by a comma-delimited list of positional parameters that are themselves substituded
   into the text after the next CPAREN in place of the tokens '$1', '$2', etc.
   
   Flex's own filter macros only use one positional argument, currently.
*/
static void c99_filter_call_macro ( const struct flex_backend_t *b, const char *n, const char *v ) {
	b->verbatim(b, n);
	b->verbatim(b, "( ");
	b->verbatim(b, v);
	b->verbatim(b, " )");
	b->newline(b);
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
	.get_packed_type= c99_get_packed_type,
	.open_block_comment = c99_open_block_comment,
	.close_block_comment = c99_close_block_comment,
	.comment = c99_comment,
	.record_separator = c99_record_separator,
	.column_separator = c99_column_separator,
	.newline = c99_newline,
	.increase_indent = c99_increase_indent,
	.decrease_indent = c99_decrease_indent,
	.indent = c99_indent,
	.get_trace_line_format = c99_get_trace_line_format,
	.line_directive_out = c99_line_directive_out,
	.open_table = c99_open_table,
	.continue_table = c99_continue_table,
	.close_table = c99_close_table,
	.verbatim = c99_verbatim,
	.format_data_table_entry = c99_format_data_table_entry,
	.format_state_table_entry = c99_format_state_table_entry,
	.format_normal_state_case_arm = c99_format_normal_state_case_arm,
	.format_eof_state_case_arm = c99_format_eof_state_case_arm,
	.eof_state_case_fallthrough = c99_eof_state_case_fallthrough,
	.eof_state_case_terminate = c99_eof_state_case_terminate,
	.take_yytext = c99_take_yytext,
	.release_yytext = c99_release_yytext,
	.format_char_rewind = c99_format_char_rewind,
	.format_line_rewind = c99_format_line_rewind,
	.format_char_forward = c99_format_char_forward,
	.format_line_forward = c99_format_line_forward,
	.format_byte_const = c99_format_byte_const,
	.format_state_const = c99_format_state_const,
	.format_size_const = c99_format_size_const,
	.format_uint_const = c99_format_uint_const,
	.format_bool_const = c99_format_bool_const,
	.format_const = c99_format_const,
	.format_offset_type = c99_format_offset_type,
	.format_yy_decl = c99_format_yy_decl,
	.format_userinit = c99_format_userinit,
	.format_rule_setup = c99_format_rule_setup,
	.format_user_preaction = c99_format_user_preaction,
	.format_state_case_break = c99_format_state_case_break,
	.format_user_postaction = c99_format_user_postaction,
	.format_fatal_error = c99_format_fatal_error,
	.echo = c99_echo,
	.format_yyterminate = c99_format_yyterminate,
	.format_yyreject = c99_format_yyreject,
	.filter_define_name = c99_filter_define_name,
	.filter_define_close = c99_filter_define_close,
	.filter_define_vars = c99_filter_define_vars,
	.filter_define_vard = c99_filter_define_vard,
	.filter_call_macro = c99_filter_call_macro
};

