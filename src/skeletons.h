
/* skeletons.h - skeletons file for flex */

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


#ifndef FLEX_SKELETONS_H
#define FLEX_SKELETONS_H 1

/* Method table describing a language-specific back end.
 * Even if this never gets a member other than the skel
 * array, it prevents us from getting lost in a maze of
 * twisty array reference levels, all different.
 */
struct flex_backend_t {
	const char **skel;		// Digested skeleton file
	unsigned int indent_level;
	const char * (*get_int32_type) ( struct flex_backend_t *b );
	const char * (*get_int16_type) ( struct flex_backend_t *b );
	void (*open_block_comment) ( struct flex_backend_t *b );
	void (*close_block_comment) ( struct flex_backend_t *b );
	void (*comment) ( struct flex_backend_t *b, const char *c );
	void (*record_separator) ( struct flex_backend_t *b );
	void (*column_separator) ( struct flex_backend_t *b );
	void (*newline) ( struct flex_backend_t *b );
	void (*increase_indent) ( struct flex_backend_t *b );
	void (*decrease_indent) ( struct flex_backend_t *b );
	void (*indent) ( struct flex_backend_t *b );
	const char * (*get_trace_line_format) ( struct flex_backend_t *b );
	void (*line_directive_out) ( struct flex_backend_t *b, FILE *output_file, char *path, int linenum );
	void (*open_table) ( struct flex_backend_t *b );
	void (*continue_table) ( struct flex_backend_t *b );
	void (*close_table) ( struct flex_backend_t *b );
	void (*relativize) ( struct flex_backend_t *b, const char *s );
	void (*format_state_table_entry) ( struct flex_backend_t *b, int t );
	void (*format_normal_state_case_arm) ( struct flex_backend_t *b, int c );
	void (*format_eof_state_case_arm) ( struct flex_backend_t *b, int c );
	void (*eof_state_case_fallthrough) ( struct flex_backend_t *b );
	void (*eof_state_case_terminate) ( struct flex_backend_t *b );
	void (*take_yytext) ( struct flex_backend_t *b );
	void (*release_yytext) ( struct flex_backend_t *b );
	void (*format_char_rewind) ( struct flex_backend_t *b, int c );
	void (*format_line_rewind) ( struct flex_backend_t *b, int l );
	void (*format_char_forward) ( struct flex_backend_t *b, int c );
	void (*format_line_forward) ( struct flex_backend_t *b, int l );
	void (*format_byte_const) ( struct flex_backend_t *b, const char *n, const int c );
	void (*format_state_const) ( struct flex_backend_t *b, const char *n, const int s );
	void (*format_uint_const) ( struct flex_backend_t *b, const char *n, const unsigned int u );
	void (*format_bool_const) ( struct flex_backend_t *b, const char *n, const int t );
	void (*format_const) ( struct flex_backend_t *b, const char *n, const char *v );
	void (*format_offset_type) ( struct flex_backend_t *b, const char *t );
	void (*format_yy_decl) ( struct flex_backend_t *b, const char *d );
	void (*format_userinit) ( struct flex_backend_t *b, const char *d );
	void (*format_rule_setup) ( struct flex_backend_t *b );
	void (*format_user_preaction) ( struct flex_backend_t *b, const char *d );
	void (*format_state_case_break) ( struct flex_backend_t *b );
	void (*format_user_postaction) ( struct flex_backend_t *b, const char *d );
	void (*format_fatal_error) ( struct flex_backend_t *b, const char *e );
	void (*echo) ( struct flex_backend_t *b );
	void (*format_yyterminate) ( struct flex_backend_t *b, const char *d );
	void (*format_yyreject) ( struct flex_backend_t *b );
};

/* For blocking out code from the header file. */
// #define OUT_BEGIN_CODE() outn("m4_ifdef( [[M4_YY_IN_HEADER]],,[[m4_dnl")
// #define OUT_END_CODE()   outn("]])")


#endif /* FLEX_SKELETONS_H */
