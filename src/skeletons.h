
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

#include "flexdef.h"

bool push_backend(flex_backend_id_t);
flex_backend_id_t pop_backend(void);
flex_backend_id_t top_backend(void);

/* Method table describing a language-specific back end.
 * Even if this never gets a member other than the skel
 * array, it prevents us from getting lost in a maze of
 * twisty array reference levels, all different.
 */
struct flex_backend_t {
	const char **skel;		// Digested skeleton file
	unsigned int indent_level;
	const char * (*get_int32_type) ( const struct flex_backend_t *b );
	const char * (*get_int16_type) ( const struct flex_backend_t *b );
	const char * (*get_state_type) ( const struct flex_backend_t *b );
	void (*open_block_comment) ( const struct flex_backend_t *b );
	void (*close_block_comment) ( const struct flex_backend_t *b );
	void (*comment) ( const struct flex_backend_t *b, const char *c );
	void (*record_separator) ( const struct flex_backend_t *b );
	void (*column_separator) ( const struct flex_backend_t *b );
	void (*newline) ( const struct flex_backend_t *b );
	void (*increase_indent) ( const struct flex_backend_t *b );
	void (*decrease_indent) ( const struct flex_backend_t *b );
	void (*indent) ( const struct flex_backend_t *b );
	const char * (*get_trace_line_format) ( const struct flex_backend_t *b );
	void (*line_directive_out) ( const struct flex_backend_t *b, FILE *output_file, char *path, int linenum );
	void (*open_table) ( const struct flex_backend_t *b );
	void (*continue_table) ( const struct flex_backend_t *b );
	void (*close_table) ( const struct flex_backend_t *b );
	void (*verbatim) ( const struct flex_backend_t *b, const char *s );
	void (*format_data_table_entry) ( const struct flex_backend_t *b, int t );
	void (*format_state_table_entry) ( const struct flex_backend_t *b, int t );
	void (*format_normal_state_case_arm) ( const struct flex_backend_t *b, int c );
	void (*format_eof_state_case_arm) ( const struct flex_backend_t *b, const char *const c );
	void (*eof_state_case_fallthrough) ( const struct flex_backend_t *b );
	void (*eof_state_case_terminate) ( const struct flex_backend_t *b );
	void (*take_yytext) ( const struct flex_backend_t *b );
	void (*release_yytext) ( const struct flex_backend_t *b );
	void (*format_char_rewind) ( const struct flex_backend_t *b, int c );
	void (*format_line_rewind) ( const struct flex_backend_t *b, int l );
	void (*format_char_forward) ( const struct flex_backend_t *b, int c );
	void (*format_line_forward) ( const struct flex_backend_t *b, int l );
	void (*format_byte_const) ( const struct flex_backend_t *b, const char *n, const int c );
	void (*format_state_const) ( const struct flex_backend_t *b, const char *n, const int s );
	void (*format_size_const) ( const struct flex_backend_t *b, const char *n, const int s );
	void (*format_uint_const) ( const struct flex_backend_t *b, const char *n, const unsigned int u );
	void (*format_bool_const) ( const struct flex_backend_t *b, const char *n, const int t );
	void (*format_const) ( const struct flex_backend_t *b, const char *n, const char *v );
	void (*format_offset_type) ( const struct flex_backend_t *b, const char *t );
	void (*format_yy_decl) ( const struct flex_backend_t *b, const char *d );
	void (*format_userinit) ( const struct flex_backend_t *b, const char *d );
	void (*format_rule_setup) ( const struct flex_backend_t *b );
	void (*format_user_preaction) ( const struct flex_backend_t *b, const char *d );
	void (*format_state_case_break) ( const struct flex_backend_t *b );
	void (*format_user_postaction) ( const struct flex_backend_t *b, const char *d );
	void (*format_fatal_error) ( const struct flex_backend_t *b, const char *e );
	void (*echo) ( const struct flex_backend_t *b );
	void (*format_yyterminate) ( const struct flex_backend_t *b, const char *d );
	void (*format_yyreject) ( const struct flex_backend_t *b );
	void (*filter_define_name) ( const struct flex_backend_t *b, const char *n, const int leave_open );
	void (*filter_define_close) (const struct flex_backend_t *b, const char *v);
	void (*filter_define_vars) ( const struct flex_backend_t *b, const char *n, const char *v );
	void (*filter_define_vard) ( const struct flex_backend_t *b, const char *n, const int v );
	void (*filter_call_macro) ( const struct flex_backend_t *b, const char *n, const char *v );
};

const struct flex_backend_t *get_backend(void);


#endif /* FLEX_SKELETONS_H */
